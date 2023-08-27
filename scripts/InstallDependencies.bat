SETLOCAL

IF NOT EXIST ..\vcpkg\bootstrap-vcpkg.bat (
    ECHO Initializing Git submodules...
    CALL git submodule update --init --recursive
)

IF NOT EXIST ..\vcpkg\vcpkg.exe (
    ECHO Bootstrapping vcpkg...
    CALL ..\vcpkg\bootstrap-vcpkg.bat
)

set "timestamp_file=vcpkg_installed\\manifest_last_modified.txt"

if not exist %timestamp_file% (
    mkdir "vcpkg_installed"
    echo 0 > %timestamp_file%
)

set /p prev_timestamp=<%timestamp_file%

for %%A in ("vcpkg.json") do set "current_timestamp=%%~tA"

if "%current_timestamp%" neq "%prev_timestamp%" (
    echo %current_timestamp%>%timestamp_file%
    echo Running vcpkg install...
    ..\vcpkg\vcpkg.exe install
)

ENDLOCAL

EXIT 0
