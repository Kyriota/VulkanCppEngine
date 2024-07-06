@echo off
set IS_DEBUG=%1
set RUNTIME_OUTPUT_DIR=%2
set DYNAMIC_LIB_DIR=%3

if not exist "%RUNTIME_OUTPUT_DIR%" (
    echo Runtime output directory not found, given path: "%RUNTIME_OUTPUT_DIR%"
    exit /b 1
)

@REM Copy GCC DLLs
for /f "tokens=*" %%i in ('where gcc.exe') do (
    set GCC_PATH=%%~dpi
)
set GCC_DLL_LIST=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll
for %%f in (%GCC_DLL_LIST%) do (
    copy "%GCC_PATH%\%%f" "%RUNTIME_OUTPUT_DIR%" >nul
    if errorlevel 1 (
        echo Error occured when copying "%%f"
        exit /b 1
    )
    echo Copied "%%f"
)

@REM Copy yaml-cpp DLL
if %IS_DEBUG% == 1 (
    set YAML_CPP_DLL=libyaml-cppd.dll
) else (
    set YAML_CPP_DLL=libyaml-cpp.dll
)
copy "%DYNAMIC_LIB_DIR%\%YAML_CPP_DLL%" "%RUNTIME_OUTPUT_DIR%" >nul
if errorlevel 1 (
    echo Error occured when copying "%YAML_CPP_DLL%"
    exit /b 1
)
echo Copied "%YAML_CPP_DLL%"

echo All DLLs copied successfully.
exit /b 0