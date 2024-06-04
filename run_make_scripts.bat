@echo off
set RUNTIME_OUTPUT_DIRECTORY=%1
set SHADER_SOURCE_DIR=%2
set ASSETS_ROOT_DIR=%3
set CONFIG_ROOT_DIR=%4

call %~dp0\MakeScripts\win32\compile_shaders.bat %RUNTIME_OUTPUT_DIRECTORY% %SHADER_SOURCE_DIR%
if errorlevel 1 (
    echo Error occured when compiling shaders
    exit /b 1
)

call %~dp0\MakeScripts\win32\copy_assets.bat %RUNTIME_OUTPUT_DIRECTORY% %ASSETS_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying assets
    exit /b 1
)

call %~dp0\MakeScripts\win32\copy_dlls.bat %RUNTIME_OUTPUT_DIRECTORY%
if errorlevel 1 (
    echo Error occured when copying dlls
    exit /b 1
)

call %~dp0\MakeScripts\win32\copy_config.bat %RUNTIME_OUTPUT_DIRECTORY% %CONFIG_ROOT_DIR%
if errorlevel 1 (
    echo Error occured when copying config
    exit /b 1
)

echo All scripts executed successfully.
exit /b 0