@echo off
set SHADER_OUTPUT_DIR=%1\assets\shaders
set SHADER_DIR=%2

set GLSLC_PATH=%VULKAN_SDK%\Bin\glslc.exe

if not exist "%SHADER_DIR%" (
    echo Shader directory not found, given path: "%SHADER_DIR%"
    exit /b 1
)

if not exist "%GLSLC_PATH%" (
    echo glslc.exe not found, given path: "%GLSLC_PATH%"
    exit /b 1
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
