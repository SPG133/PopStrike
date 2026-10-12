@echo off
setlocal

set "UE_ROOT=D:\UnrealEngine-release"
set "PROJECT_FILE=%~dp0PopStrike.uproject"
set "OUTPUT_DIR=%~dp0Packaged\Client"

call "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=%PROJECT_FILE%" -noP4 -client -targetplatform=Win64 -clientconfig=Development -build -cook -stage -pak -package -archive "-archivedirectory=%OUTPUT_DIR%" -utf8output
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (
    echo.
    echo PopStrike Development Client package succeeded.
    echo Output: %OUTPUT_DIR%
) else (
    echo.
    echo PopStrike Development Client package failed with exit code %RESULT%.
)

pause
exit /b %RESULT%
