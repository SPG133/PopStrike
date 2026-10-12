@echo off
setlocal

set "UE_ROOT=D:\UnrealEngine-release"
set "PROJECT_FILE=%~dp0PopStrike.uproject"
set "OUTPUT_DIR=%~dp0Packaged\Server"

call "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=%PROJECT_FILE%" -noP4 -server -noclient -servertargetplatform=Win64 -serverconfig=Development -build -cook -stage -pak -package -archive "-archivedirectory=%OUTPUT_DIR%" -utf8output
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (
    echo.
    echo PopStrike Development Dedicated Server package succeeded.
    echo Output: %OUTPUT_DIR%
) else (
    echo.
    echo PopStrike Development Dedicated Server package failed with exit code %RESULT%.
)

pause
exit /b %RESULT%
