@echo off
set IS_DEBUG=%1
set RUNTIME_OUTPUT_DIRECTORY=%2
set SHADER_SOURCE_DIR=%3
set ASSETS_ROOT_DIR=%4
set CONFIG_ROOT_DIR=%5
set DYNAMIC_LIB_DIR=%6

echo - is debug: %IS_DEBUG%
echo - runtime output dir: %RUNTIME_OUTPUT_DIRECTORY%
echo - shader source dir: %SHADER_SOURCE_DIR%
echo - assets root dir: %ASSETS_ROOT_DIR%
echo - config root dir: %CONFIG_ROOT_DIR%
echo - dynamic lib dir: %DYNAMIC_LIB_DIR%

call %~dp0\compile_shaders.bat %RUNTIME_OUTPUT_DIRECTORY% %SHADER_SOURCE_DIR%
if errorlevel 1 (
    echo Error occured when compiling shaders
    exit /b 1
)

call %~dp0\copy_assets.bat %RUNTIME_OUTPUT_DIRECTORY% %ASSETS_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying assets
    exit /b 1
)

call %~dp0\copy_dlls.bat %IS_DEBUG% %RUNTIME_OUTPUT_DIRECTORY% %DYNAMIC_LIB_DIR%
if errorlevel 1 (
    echo Error occured when copying dlls
    exit /b 1
)

call %~dp0\copy_config.bat %RUNTIME_OUTPUT_DIRECTORY% %CONFIG_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying config
    exit /b 1
)

echo All scripts executed successfully.
exit /b 0