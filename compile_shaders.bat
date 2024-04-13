@echo off
set SDK_PATH=C:\VulkanSDK
set GLSLC_PATH=%SDK_PATH%\Bin\glslc.exe
set SHADER_DIR=%~dp0\shaders

if not exist "%SHADER_DIR%" (
    echo Shader directory not found.
    exit /b 1
)

for %%f in ("%SHADER_DIR%\*.vert" "%SHADER_DIR%\*.frag" "%SHADER_DIR%\*.comp") do (
    "%GLSLC_PATH%" -o "%%f.spv" "%%f"
    echo Compiled "%%f" to "%%f.spv"
)

echo Shaders compiling finished.
exit /b 0