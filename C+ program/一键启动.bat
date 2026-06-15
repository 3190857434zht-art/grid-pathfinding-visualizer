@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

cd /d "%~dp0"
title 多策略路径规划可视化 - 一键启动

echo.
echo ============================================
echo   多策略路径规划可视化系统
echo   一键启动（无需打开代码编辑器）
echo ============================================
echo.

set "EXE=build\bin\path_planner.exe"
set "NEED_BUILD=0"
set "NEED_UI=0"

:: ---------- 检查前端资源 ----------
if not exist "web\index.html" set "NEED_UI=1"
if not exist "web\vendor\bootstrap\bootstrap.min.css" set "NEED_UI=1"
if not exist "web\js\app.js" set "NEED_UI=1"
if not exist "third_party\httplib\httplib.h" set "NEED_UI=1"

if "!NEED_UI!"=="1" (
    echo [1/3] 首次运行，正在下载界面资源（需联网）...
    call "%~dp0scripts\setup_frontend.bat"
    if errorlevel 1 goto fail
    echo.
) else (
    echo [1/3] 界面资源已就绪
)

:: ---------- 检查 / 编译程序 ----------
if not exist "!EXE!" set "NEED_BUILD=1"

if "!NEED_BUILD!"=="0" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\check_rebuild.ps1" -ProjectRoot "%CD%" >nul 2>&1
    if errorlevel 2 set "NEED_BUILD=1"
)

if not "!NEED_BUILD!"=="1" goto skip_build

echo [2/3] 正在编译程序（源码有更新或首次运行，约 20~40 秒）...
call "%~dp0build.bat"
if errorlevel 1 goto fail
if not exist "!EXE!" (
    echo 编译完成但未找到可执行文件: !EXE!
    goto fail
)
echo.
goto after_build

:skip_build
echo [2/3] 程序已编译

:after_build
set "PORT="
if not "%~1"=="" set "PORT=%~1"

:: ---------- 启动服务（浏览器由程序自动打开，仅打开一次）----------
echo [3/3] 正在启动 Web 服务...
if defined PORT (
    echo.
    echo  --------------------------------------------
    echo   浏览器地址: http://127.0.0.1:!PORT!
    echo   若未自动打开，请复制上述地址到浏览器
) else (
    echo.
    echo  --------------------------------------------
    echo   浏览器地址: http://127.0.0.1:8765
    echo   若未自动打开，请复制上述地址到浏览器
)
echo.
echo   【关闭本窗口 = 停止程序】
echo  --------------------------------------------
echo.

if defined PORT (
    "!EXE!" web !PORT!
) else (
    "!EXE!" web
)
set "ERR=!ERRORLEVEL!"

if not "!ERR!"=="0" (
    echo.
    echo 程序异常退出（错误码 !ERR!^）。
    echo 若提示端口占用，可双击「一键启动.bat 9000」或见下方说明。
    goto fail
)

exit /b 0

:fail
echo.
echo ============================================
echo  启动失败，常见原因：
echo   1. 未安装 C++ 编译器（MinGW 或 Visual Studio）
echo   2. 网络问题导致资源下载失败
echo   3. 端口 8765 已被占用
echo.
echo  端口被占用时可新建快捷方式，目标设为：
echo   "%~f0" 9000
echo  或在命令行运行：一键启动.bat 9000
echo ============================================
echo.
pause
exit /b 1
