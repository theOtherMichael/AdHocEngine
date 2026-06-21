#!/usr/bin/env python3
"""Download and extract the pinned LLVM toolchain into tools/llvm/<version>/.

Idempotent: exits 0 immediately if <output-dir>/bin/clang[.exe] already exists.
SHA256 verification is skipped when the expected hash is empty (prints a warning).
"""
from __future__ import annotations

import argparse
import hashlib
import os
import platform
import stat
import subprocess
import sys
import tarfile
import time
import urllib.request
from pathlib import Path

class _Progress:
    """Emits a fresh progress line on a steady time cadence so the job never looks stuck.
    """

    def __init__(self, interval: float = 1.0) -> None:
        self._interval = interval
        self.start = time.monotonic()
        self._last = 0.0

    def update(self, detail: str, *, force: bool = False) -> None:
        now = time.monotonic()
        if not force and (now - self._last) < self._interval:
            return
        self._last = now
        print(f"  {detail}", flush=True)

    def done(self, detail: str) -> None:
        print(f"  {detail}", flush=True)


def _rate(downloaded: int, start: float) -> str:
    elapsed = max(time.monotonic() - start, 1e-6)
    return f"{(downloaded / 1024 / 1024) / elapsed:5.1f} MB/s"


def _sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while chunk := f.read(1 << 20):
            h.update(chunk)
    return h.hexdigest()


def _download(url: str, dest: Path) -> None:
    print(f"Downloading {url}", flush=True)
    progress = _Progress(3.0)
    with urllib.request.urlopen(url) as response:
        total = int(response.headers.get("Content-Length", 0))
        mb_total = total // 1024 // 1024
        downloaded = 0
        with open(dest, "wb") as f:
            while True:
                chunk = response.read(1 << 20)
                if not chunk:
                    break
                f.write(chunk)
                downloaded += len(chunk)
                mb_done = downloaded // 1024 // 1024
                if total:
                    pct = downloaded * 100 // total
                    progress.update(f"[{pct:3d}%]  {mb_done} / {mb_total} MB   {_rate(downloaded, progress.start)}")
                else:
                    progress.update(f"{mb_done} MB   {_rate(downloaded, progress.start)}")
    progress.done(f"Download complete ({downloaded // 1024 // 1024} MB in {time.monotonic() - progress.start:.0f}s).")


def _extract_tar(archive_path: Path, dest: Path) -> None:
    """Extract a .tar.xz, stripping any single top-level directory prefix."""
    with tarfile.open(archive_path, "r:xz") as tf:
        members = tf.getmembers()
        # Detect a shared prefix (e.g. "LLVM-20.1.4-macOS-ARM64/").
        # If the first entry IS the top-level dir itself, strip it.
        prefix = ""
        if members:
            first = members[0].name
            top = first.split("/")[0]
            # All entries under this prefix → strip it.
            if all(m.name.startswith(top + "/") or m.name == top for m in members):
                prefix = top + "/"
        total = len(members)
        progress = _Progress()
        for i, m in enumerate(members):
            if prefix and m.name.startswith(prefix):
                m.name = m.name[len(prefix):]
            if not m.name or m.name == ".":
                continue
            tf.extract(m, dest)
            if total:
                pct = (i + 1) * 100 // total
                progress.update(f"[{pct:3d}%]  Extracting... ({i + 1}/{total} files)")
        progress.done(f"Extracted {total} files.")
    # Ensure binaries are executable.
    bin_dir = dest / "bin"
    if bin_dir.is_dir():
        for f in bin_dir.iterdir():
            if f.is_file():
                f.chmod(f.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def _ensure_tool_aliases(versioned_dir: Path) -> None:
    """Windows only: hard-link '<tool>' -> '<tool>.exe' for each VS Code-facing tool.

    Hard links need no elevation on NTFS. CreateProcess executes PE binaries by full
    path regardless of extension, so VS Code can use the same extension-free path on
    all platforms (e.g. clangd.path / clang_format_path in settings.json).
    """
    if sys.platform != "win32":
        return
    bin_dir = versioned_dir / "bin"
    for name in ("clang-format", "clangd"):
        src = bin_dir / f"{name}.exe"
        dst = bin_dir / name
        if dst.exists():
            continue
        if not src.exists():
            print(f"WARNING: {src} not found — skipping {name} alias", file=sys.stderr)
            continue
        try:
            os.link(src, dst)
            print(f"Hard link: {dst.name} -> {src.name}")
        except OSError as e:
            print(f"WARNING: Could not create hard link {dst}: {e}", file=sys.stderr)


def _ensure_current_symlink(versioned_dir: Path) -> None:
    """Create/update tools/llvm/current -> <version>.

    `current` is a stable, version-agnostic handle for consumers that resolve
    LLVM at *runtime* (so a toolchain bump needs no re-baking): clang-format
    (scripts/format.py, .vscode/settings.json) and the source-mode editor, which
    references this copy rather than staging the multi-GB toolchain per slot (see
    docs/SourceMode.md). The build itself doesn't use it -- CMake compiles through
    the versioned ADHOC_LLVM_ROOT.

    macOS/Linux: relative symlink. Windows: directory junction (no elevation).
    """
    link = versioned_dir.parent / "current"
    target_rel = Path(versioned_dir.name)  # relative: e.g. "20.1.4"

    # Remove stale link/junction if it points elsewhere.
    if link.exists() or link.is_symlink():
        try:
            if link.resolve() == versioned_dir.resolve():
                return
        except OSError:
            pass
        if link.is_symlink():
            link.unlink()
        elif sys.platform == "win32":
            # os.rmdir removes a junction without touching its contents;
            # fails safely with OSError if it is a real non-empty directory.
            try:
                os.rmdir(link)
            except OSError:
                print(f"WARNING: {link} exists and cannot be replaced — skipping 'current' alias", file=sys.stderr)
                return
        else:
            print(f"WARNING: {link} exists and is not a symlink — skipping 'current' alias", file=sys.stderr)
            return

    if sys.platform == "win32":
        # Use a directory junction — works without elevation or Developer Mode.
        # Junctions store absolute paths, so pass versioned_dir directly.
        try:
            subprocess.run(
                ["cmd", "/c", "mklink", "/J", str(link), str(versioned_dir)],
                check=True,
                capture_output=True,
            )
            print(f"Junction: {link} -> {versioned_dir}")
        except Exception as e:
            print(f"WARNING: Could not create junction {link}: {e}", file=sys.stderr)
    else:
        try:
            link.symlink_to(target_rel, target_is_directory=True)
            print(f"Symlink: {link} -> {target_rel}")
        except OSError as e:
            print(f"WARNING: Could not create symlink {link}: {e}", file=sys.stderr)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--version",    required=True)
    p.add_argument("--url",        required=True)
    p.add_argument("--sha256",     default="")
    p.add_argument("--output-dir", required=True)
    args = p.parse_args()

    out = Path(args.output_dir)
    system = platform.system()

    if system == "Darwin":
        clang_bin = out / "bin" / "clang"
    elif system == "Windows":
        clang_bin = out / "bin" / "clang-cl.exe"
    else:
        print(f"ERROR: Unsupported platform: {system}", file=sys.stderr)
        return 1

    if clang_bin.exists():
        print(f"LLVM {args.version} already present at {out}")
        _ensure_tool_aliases(out)
        _ensure_current_symlink(out)
        return 0

    out.mkdir(parents=True, exist_ok=True)
    archive = out.parent / Path(args.url).name

    _download(args.url, archive)

    if args.sha256:
        actual = _sha256(archive)
        if actual != args.sha256.lower():
            print(f"ERROR: SHA256 mismatch!\n  expected: {args.sha256}\n  actual:   {actual}", file=sys.stderr)
            archive.unlink(missing_ok=True)
            return 1
        print("OK: SHA256 verified.")
    else:
        print("WARNING: No expected SHA256 provided — skipping verification.", file=sys.stderr)

    print(f"Extracting to {out}...", flush=True)
    if system == "Darwin" or system == "Windows":
        _extract_tar(archive, out)

    archive.unlink(missing_ok=True)
    print(f"LLVM {args.version} installed at {out}")
    _ensure_tool_aliases(out)
    _ensure_current_symlink(out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
