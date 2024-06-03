@echo off
set RUNTIME_OUTPUT_DIR=%1

set GCC_DLL_LIST=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll

if not exist "%RUNTIME_OUTPUT_DIR%" (
    echo Runtime output directory not found, given path: "%RUNTIME_OUTPUT_DIR%"
    exit /b 1
)

for %%f in (%GCC_DLL_LIST%) do (
    copy "C:\MinGW\bin\%%f" "%RUNTIME_OUTPUT_DIR%"
    if errorlevel 1 (
        echo Error occured when copying "%%f"
        exit /b 1
    )
    echo Copied "%%f"
)

echo All DLLs copied successfully.
exit /b 0