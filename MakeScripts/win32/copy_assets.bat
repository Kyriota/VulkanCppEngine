@echo off
set ASSET_OUTPUT_DIR=%1\assets
set ASSET_DIR=%2

if not exist "%ASSET_DIR%" (
    echo Asset directory not found, given path: "%ASSET_DIR%"
    exit /b 1
)

if not exist "%ASSET_OUTPUT_DIR%" (
    mkdir "%ASSET_OUTPUT_DIR%"
)

REM Copy %ASSET_DIR%\models to %ASSET_OUTPUT_DIR%\models
xcopy /s /i /y "%ASSET_DIR%\models" "%ASSET_OUTPUT_DIR%\models" >nul

echo All assets copied successfully.
exit /b 0