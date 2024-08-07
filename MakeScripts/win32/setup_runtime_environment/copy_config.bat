@echo off
set BIN_DIR=%1
set CONFIG_DIR=%2

if not exist "%CONFIG_DIR%" (
    echo Config directory not found, given path: "%CONFIG_DIR%"
    exit /b 1
)

REM Copy %CONFIG_DIR% to %BIN_DIR%\Configs
xcopy /s /i /y "%CONFIG_DIR%" "%BIN_DIR%\config" >nul

echo All configs copied successfully.
exit /b 0