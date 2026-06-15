@echo off
chcp 65001 >nul
setlocal

cd /d "%~dp0\.."

echo 正在下载 Web UI 依赖（Bootstrap / Chart.js / httplib / nlohmann/json）...

if not exist "third_party\httplib" mkdir "third_party\httplib"
if not exist "third_party\json" mkdir "third_party\json"
if not exist "web\vendor\bootstrap" mkdir "web\vendor\bootstrap"
if not exist "web\vendor\icons\fonts" mkdir "web\vendor\icons\fonts"

powershell -NoProfile -Command ^
  "$ProgressPreference='SilentlyContinue';" ^
  "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.15.3/httplib.h' -OutFile 'third_party/httplib/httplib.h';" ^
  "Invoke-WebRequest -Uri 'https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp' -OutFile 'third_party/json/json.hpp';" ^
  "Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css' -OutFile 'web/vendor/bootstrap/bootstrap.min.css';" ^
  "Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js' -OutFile 'web/vendor/bootstrap/bootstrap.bundle.min.js';" ^
  "Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/bootstrap-icons.min.css' -OutFile 'web/vendor/icons/bootstrap-icons.min.css';" ^
  "Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/fonts/bootstrap-icons.woff2' -OutFile 'web/vendor/icons/fonts/bootstrap-icons.woff2';" ^
  "Invoke-WebRequest -Uri 'https://cdn.jsdelivr.net/npm/chart.js@4.4.1/dist/chart.umd.min.js' -OutFile 'web/vendor/chart.umd.min.js'"

if errorlevel 1 (
    echo 下载失败，请检查网络连接。
    exit /b 1
)

echo 前端依赖下载完成。
exit /b 0
