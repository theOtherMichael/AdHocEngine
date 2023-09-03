SETLOCAL

IF NOT EXIST "..\vcpkg\bootstrap-vcpkg.bat" (
    ECHO Initializing Git submodules...
    CALL git submodule update --init --recursive
)

IF NOT EXIST "..\vcpkg\vcpkg.exe" (
    ECHO Bootstrapping vcpkg...
    CALL "..\vcpkg\bootstrap-vcpkg.bat"
)

ECHO Running vcpkg install for dynamic triplet...
CALL "..\vcpkg\vcpkg.exe" install --no-print-usage --triplet=x64-windows --x-install-root=vcpkg_installed\dynamic
ECHO Running vcpkg install for static triplet...
CALL "..\vcpkg\vcpkg.exe" install --no-print-usage --triplet=x64-windows-static-md --x-install-root=vcpkg_installed\static

SET "output_file=vcpkg_installed\empty.txt"
IF EXIST %output_file% (
    DEL %output_file%
)
ECHO. > %output_file%

ENDLOCAL

EXIT 0
