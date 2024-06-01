@echo off
set SDK_PATH=C:\VulkanSDK
set GLSLC_PATH=%SDK_PATH%\Bin\glslc.exe
set SHADER_DIR=%~dp0\assets\shaders

set BUILD_DIR=%~dp0\build
set SHADER_OUTPUT_DIR=%BUILD_DIR%\shaders

if not exist "%SHADER_DIR%" (
    echo Shader directory not found.
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

if not exist "%SHADER_OUTPUT_DIR%" (
    mkdir "%SHADER_OUTPUT_DIR%"
)

for %%f in ("%SHADER_DIR%\*.vert" "%SHADER_DIR%\*.frag" "%SHADER_DIR%\*.comp") do (
    "%GLSLC_PATH%" -o "%SHADER_OUTPUT_DIR%\%%~nxf.spv" "%%f"
    if errorlevel 1 (
        echo Error occured when compiling "%%f"
        exit /b 1
    )
    echo Compiled "%%f"
)

echo All shaders compiled successfully.
exit /b 0
