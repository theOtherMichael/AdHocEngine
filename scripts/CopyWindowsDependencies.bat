IF "%1"=="" (
    ECHO No configuration specified!
    ECHO Usage: %~n0.bat ^(Debug^|Dev^|Release^)
    EXIT /B 1
)

IF "%1"=="Debug" (
    IF NOT EXIST "..\build\Debug\empty.txt" (
        ECHO Copying dependencies to 'Debug'...
        XCOPY /E /I /Y /D "vcpkg_installed\dynamic\x64-windows\debug\bin" "..\build\Debug"
    ) ELSE (
        ECHO vcpkg products already copied to 'Debug'.
    )
) ELSE IF "%1"=="Dev" (
    IF NOT EXIST "..\build\Dev\empty.txt" (
        ECHO Copying dependencies to 'Dev'...
        XCOPY /E /I /Y /D "vcpkg_installed\dynamic\x64-windows\bin" "..\build\Dev"
    ) ELSE (
        ECHO vcpkg products already copied to 'Dev'.
    )
) ELSE IF "%1"=="Release" (
    IF NOT EXIST "..\build\Release\empty.txt" (
        ECHO Copying dependencies to 'Release'...
        XCOPY /E /I /Y /D "vcpkg_installed\dynamic\x64-windows\bin" "..\build\Release"
    ) ELSE (
        ECHO vcpkg products already copied to 'Release'.
    )
) ELSE (
    ECHO Invalid configuration.
    ECHO Usage: %~n0.bat ^(Debug^|Dev^|Release^)
    EXIT /B 1
)

EXIT /B 0
