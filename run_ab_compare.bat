@echo off
setlocal
cd /d "%~dp0"

echo ============================================================
echo Nicolai Native Port - A/B renderer comparison
echo ============================================================
echo.

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\run_ab_compare.ps1" %*
set "RC=%ERRORLEVEL%"

echo.
if "%RC%"=="0" (
  echo A/B comparison finished successfully.
) else (
  echo A/B comparison failed with exit code %RC%.
)
echo.
pause
exit /b %RC%
