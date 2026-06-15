@echo off
chcp 65001 >nul
setlocal

cd /d "%~dp0"

where cmake >nul 2>&1
if errorlevel 1 (
    echo 未检测到 CMake，改用 MinGW 直接编译...
    call "%~dp0build_mingw.bat"
    exit /b %ERRORLEVEL%
)

if not exist build mkdir build
cd build

echo [1/2] 正在配置 CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake 配置失败，尝试 MinGW 直接编译...
    cd /d "%~dp0"
    call "%~dp0build_mingw.bat"
    exit /b %ERRORLEVEL%
)

echo [2/2] 正在编译...
cmake --build . --config Release
if errorlevel 1 (
    echo 编译失败。
    exit /b 1
)

echo.
echo 编译成功！运行 run.bat 启动程序。
exit /b 0
