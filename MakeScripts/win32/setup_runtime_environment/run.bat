@echo off
set BIN_DIR=%1
set SHADER_SOURCE_DIR=%2
set ASSETS_ROOT_DIR=%3
set CONFIG_ROOT_DIR=%4

echo - runtime output dir: %BIN_DIR%
echo - shader source dir: %SHADER_SOURCE_DIR%
echo - assets root dir: %ASSETS_ROOT_DIR%
echo - config root dir: %CONFIG_ROOT_DIR%

call %~dp0\scripts\compile_shaders.bat %BIN_DIR% %SHADER_SOURCE_DIR%
if errorlevel 1 (
    echo Error occured when compiling shaders
    exit /b 1
)

call %~dp0\scripts\copy_assets.bat %BIN_DIR% %ASSETS_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying assets
    exit /b 1
)

call %~dp0\scripts\copy_config.bat %BIN_DIR% %CONFIG_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying config
    exit /b 1
)

echo All scripts executed successfully.
exit /b 0