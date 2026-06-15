param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot
)

Set-Location $ProjectRoot

$exe = Join-Path $ProjectRoot "build\bin\path_planner.exe"
if (-not (Test-Path $exe)) {
    exit 1
}

$srcRoot = Join-Path $ProjectRoot "src"
if (-not (Test-Path $srcRoot)) {
    exit 0
}

$exeTime = (Get-Item $exe).LastWriteTime
$newest = Get-ChildItem $srcRoot -Recurse -Include *.cpp, *.h -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if ($null -ne $newest -and $newest.LastWriteTime -gt $exeTime) {
    exit 2
}

exit 0
