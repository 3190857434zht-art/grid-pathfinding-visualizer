@echo off
chcp 65001 >nul
setlocal

cd /d "%~dp0"

if not exist "web\index.html" (
    echo 首次运行需下载 UI 依赖...
    call "%~dp0scripts\setup_frontend.bat"
)

if exist "build\bin\path_planner.exe" (
    if "%~1"=="" echo 启动 Web 图形界面，浏览器将自动打开 http://127.0.0.1:8765
    "build\bin\path_planner.exe" %*
    exit /b %ERRORLEVEL%
)

if exist "build\bin\Release\path_planner.exe" (
    "build\bin\Release\path_planner.exe" %*
    exit /b %ERRORLEVEL%
)

if exist "build\path_planner.exe" (
    "build\path_planner.exe" %*
    exit /b %ERRORLEVEL%
)

echo 未找到可执行文件，请先运行 build.bat 编译项目。
exit /b 1
