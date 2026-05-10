@REM @echo off
setlocal enabledelayedexpansion

rem This script is used as a custom build tool for vcpkg.json files in Visual Studio.
rem The working directory is the current project folder.

set VCPKG="..\vcpkg\vcpkg.exe"
set BOOTSTRAP_COMMIT_PATH="vcpkg_installed\bootstrap_commit.txt"
set VCPKG_COMMIT=

set LOCK_FILE="..\DO_NOT_SAVE.lock"
set LOCK_TIMEOUT_SECONDS=30

set MANIFEST_PATH="vcpkg.json"
set CHECKSUM_PATH="vcpkg_installed\manifest_checksum.txt"
set MANIFEST_CHECKSUM=""

SET IS_DYNAMIC_INSTALL_REQUESTED=false
SET IS_STATIC_INSTALL_REQUESTED=false
SET IS_DYNAMIC_INSTALL_REQUIRED=false
SET IS_STATIC_INSTALL_REQUIRED=false

if /i "%1"=="dynamic" (
    echo Option "dynamic" specified
    set IS_DYNAMIC_INSTALL_REQUESTED=true
) else if /i "%1"=="static" (
    echo Option "static" specified
    set IS_STATIC_INSTALL_REQUESTED=true
) else if /i "%1"=="all" (
    echo Option "all" specified
    set IS_DYNAMIC_INSTALL_REQUESTED=true
    set IS_STATIC_INSTALL_REQUESTED=true
) else (
    echo Invalid arguments.
    echo Usage: %~n0.bat dynamic^|static^|all
    exit /b 1
)

echo Acquiring lock on vcpkg...
set LOCK_TIMEOUT_COUNTER=0

:waitForLock
if exist "%LOCK_FILE%" (
    set /A LOCK_TIMEOUT_COUNTER+=1
    if !LOCK_TIMEOUT_COUNTER! geq 15 (
        echo Lock timeout reached.
        exit /b 1
    )

    echo Waiting for vcpkg lock...
    rem Hack to wait 1 sec
    ping -n 2 127.0.0.1 >nul

    goto waitForLock
)

type nul > "%LOCK_FILE%"
echo This file represents a lock on vcpkg. Locked by %0 > "%LOCK_FILE%"
echo vcpkg lock acquired

echo Running vcpkg install step...

for /f %%H in ('git rev-parse HEAD:../vcpkg 2^>nul') do set "VCPKG_COMMIT=%%H"

if not exist "..\vcpkg\bootstrap-vcpkg.bat" (
    echo bootstrap-vcpkg.bat not found! Initializing Git submodules...
    call git submodule update --init --recursive
    if errorlevel 1 (
        echo Git submodule update returned an error.
        del "%LOCK_FILE%"
        echo vcpkg lock released
        exit /b 1
    )
)

set NEEDS_BOOTSTRAP=false
if not exist "!VCPKG!" (
    set NEEDS_BOOTSTRAP=true
) else if not "!VCPKG_COMMIT!"=="" (
    if not exist !BOOTSTRAP_COMMIT_PATH! (
        set NEEDS_BOOTSTRAP=true
    ) else (
        set /p BOOTSTRAPPED_COMMIT=<!BOOTSTRAP_COMMIT_PATH!
        if not "!VCPKG_COMMIT!"=="!BOOTSTRAPPED_COMMIT!" (
            echo vcpkg submodule has changed since last bootstrap. Re-bootstrapping...
            set NEEDS_BOOTSTRAP=true
        )
    )
)

if "!NEEDS_BOOTSTRAP!"=="true" (
    echo Bootstrapping vcpkg...
    call "..\vcpkg\bootstrap-vcpkg.bat"
    if errorlevel 1 (
        echo vcpkg bootstrapping returned an error.
        del "%LOCK_FILE%"
        echo vcpkg lock released
        exit /b 1
    )
    if not "!VCPKG_COMMIT!"=="" (
        if not exist "vcpkg_installed" mkdir "vcpkg_installed"
        echo !VCPKG_COMMIT!>!BOOTSTRAP_COMMIT_PATH!
    )
    echo vcpkg bootstrapped successfully
)

if not exist !MANIFEST_PATH! (
    echo Manifest not found at !MANIFEST_PATH!
    del "%LOCK_FILE%"
    echo vcpkg lock released
    exit /b 1
)

echo Checking vcpkg.json for changes...
for /f "delims=" %%A in ('certutil -hashfile !MANIFEST_PATH! SHA1 ^| find /v ":"') do set "MANIFEST_CHECKSUM=%%A"
echo Manifest checksum: !MANIFEST_CHECKSUM!

if exist !CHECKSUM_PATH! (
    set /p PREVIOUS_CHECKSUM=<!CHECKSUM_PATH!
    if "!MANIFEST_CHECKSUM!" == "!PREVIOUS_CHECKSUM!" (
        echo No changes to vcpkg.json detected since last build. Dependencies will not be reinstalled
    ) else (
        echo Detected changes to vcpkg.json. Dependencies will be reinstalled
        set IS_DYNAMIC_INSTALL_REQUIRED=!IS_DYNAMIC_INSTALL_REQUESTED!
        set IS_STATIC_INSTALL_REQUIRED=!IS_STATIC_INSTALL_REQUESTED!
    )
) else (
    echo Checksum file !CHECKSUM_PATH! was not found. Dependencies will be installed
    set IS_DYNAMIC_INSTALL_REQUIRED=!IS_DYNAMIC_INSTALL_REQUESTED!
    set IS_STATIC_INSTALL_REQUIRED=!IS_STATIC_INSTALL_REQUESTED!
)

if "!IS_DYNAMIC_INSTALL_REQUIRED!"=="false" (if "!IS_STATIC_INSTALL_REQUIRED!"=="false" (
    echo Project dependencies are already installed, skipping

    del "%LOCK_FILE%"
    echo vcpkg lock released
    exit 0
))

set IS_INSTALL_FAILED=false

if "!IS_DYNAMIC_INSTALL_REQUIRED!"=="true" (
    echo Installing dependencies ^(dynamic linkage^)...
    call "!VCPKG!" install ^
        --no-print-usage ^
        --overlay-triplets=..\triplets ^
        --triplet=x64-windows-adhoc ^
        --x-install-root=vcpkg_installed\dynamic

    if errorlevel 1 (
        echo vcpkg returned an error on triplet x64-windows.
        set IS_INSTALL_FAILED=true
    )
)

if "!IS_STATIC_INSTALL_REQUIRED!"=="true" (
    echo Installing dependencies ^(static linkage^)...
    call "!VCPKG!" install ^
        --no-print-usage ^
        --overlay-triplets=..\triplets ^
        --triplet=x64-windows-static-md-adhoc ^
        --x-install-root=vcpkg_installed\static

    if errorlevel 1 (
        echo vcpkg returned an error on triplet x64-windows-static-md.
        set IS_INSTALL_FAILED=true
    )
)

del "%LOCK_FILE%"
echo vcpkg lock released

if "!IS_INSTALL_FAILED!"=="true" (
    echo One or more vcpkg install steps failed.
    exit /b 1
)

echo Installation steps completed without issue. Writing manifest checksum to file...
echo !MANIFEST_CHECKSUM!>!CHECKSUM_PATH!
if errorlevel 1 (
    echo Manifest checksum could not be written to disk. Next build may redundantly reinstall dependencies.
)

echo Successfully completed vcpkg install step!
exit /b 0
