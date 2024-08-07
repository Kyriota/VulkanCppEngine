@echo off
set IS_DEBUG=%1
set BIN_DIR=%2
set SHADER_SOURCE_DIR=%3
set ASSETS_ROOT_DIR=%4
set CONFIG_ROOT_DIR=%5
set DYNAMIC_LIB_DIR=%6

echo - is debug: %IS_DEBUG%
echo - runtime output dir: %BIN_DIR%
echo - shader source dir: %SHADER_SOURCE_DIR%
echo - assets root dir: %ASSETS_ROOT_DIR%
echo - config root dir: %CONFIG_ROOT_DIR%
echo - dynamic lib dir: %DYNAMIC_LIB_DIR%

call %~dp0\compile_shaders.bat %BIN_DIR% %SHADER_SOURCE_DIR%
if errorlevel 1 (
    echo Error occured when compiling shaders
    exit /b 1
)

call %~dp0\copy_assets.bat %BIN_DIR% %ASSETS_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying assets
    exit /b 1
)

call %~dp0\copy_external_dlls.bat %IS_DEBUG% %BIN_DIR% %DYNAMIC_LIB_DIR%
if errorlevel 1 (
    echo Error occured when copying external DLLs
    exit /b 1
)

call %~dp0\copy_config.bat %BIN_DIR% %CONFIG_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying config
    exit /b 1
)

echo All scripts executed successfully.
exit /b 0