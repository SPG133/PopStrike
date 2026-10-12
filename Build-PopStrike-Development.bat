@echo off
setlocal

set "UE_ROOT=D:\UnrealEngine-release"
set "PROJECT_FILE=%~dp0PopStrike.uproject"

rem Keep the engine's default Live Coding support; -NoLiveCoding changes shared engine definitions.
rem Refuse builds that would overwrite existing engine outputs.
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" PopStrikeEditor Win64 Development "-Project=%PROJECT_FILE%" -Module=PopStrike -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (
    echo.
    echo PopStrike Development module build succeeded.
) else (
    echo.
    echo PopStrike Development module build failed with exit code %RESULT%.
    echo If UBT reports existing engine changes, this script stops before compiling them.
)

pause
exit /b %RESULT%
