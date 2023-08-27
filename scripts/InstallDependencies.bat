SETLOCAL

IF NOT EXIST "..\vcpkg\bootstrap-vcpkg.bat" (
    ECHO Initializing Git submodules...
    CALL git submodule update --init --recursive
)

IF NOT EXIST "..\vcpkg\vcpkg.exe" (
    ECHO Bootstrapping vcpkg...
    CALL "..\vcpkg\bootstrap-vcpkg.bat"
)

ECHO Running vcpkg install...
CALL "..\vcpkg\vcpkg.exe" install --no-print-usage

SET "exists_file=vcpkg_installed\empty.txt"
IF EXIST %exists_file% (
    DEL %exists_file%
)
ECHO. > %exists_file%

ENDLOCAL

EXIT 0
