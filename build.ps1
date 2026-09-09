# Build script pour bank2_cpp
# Utilisation :  .\build.ps1           (compile le programme principal)
#               .\build.ps1 -Run      (compile puis execute)
#               .\build.ps1 -Test     (compile et lance les tests unitaires)
param([switch]$Run, [switch]$Test)

$ErrorActionPreference = "Stop"
$proj = $PSScriptRoot

# Compilateur g++ (fourni avec Webots sur cette machine)
$gccBin = "C:\Users\etudiant_adm\Webots\msys64\mingw64\bin"
$env:Path = "$gccBin;$gccBin\cpp;$env:Path"

$buildDir = "$proj\build"
New-Item -ItemType Directory -Force $buildDir | Out-Null

# --- SQLite (bibliotheque C, compilee une seule fois, sans les warnings du projet) ---
$sqliteObj = "$buildDir\sqlite3.o"
$sqliteSrc = "$proj\third_party\sqlite\sqlite3.c"
if (-not (Test-Path $sqliteObj) -or ((Get-Item $sqliteSrc).LastWriteTime -gt (Get-Item $sqliteObj).LastWriteTime)) {
    Write-Host "Compilation de SQLite..." -ForegroundColor Cyan
    & gcc -O2 -c $sqliteSrc -o $sqliteObj
    if ($LASTEXITCODE -ne 0) { Write-Host "Echec compilation SQLite." -ForegroundColor Red; exit 1 }
}

$srcFiles = Get-ChildItem -Recurse "$proj\src" -Filter *.cpp | ForEach-Object { $_.FullName }
$flags = @("-std=c++17", "-Wall", "-Wextra", "-I$proj\include", "-I$proj\third_party\sqlite")

if ($Test) {
    Write-Host "Compilation et execution des tests..." -ForegroundColor Cyan
    $testDir = "$buildDir\tests"
    New-Item -ItemType Directory -Force $testDir | Out-Null
    $pass = 0; $fail = 0
    foreach ($t in Get-ChildItem "$proj\tests" -Filter *.cpp) {
        $exe = "$testDir\$($t.BaseName).exe"
        & g++ @flags $t.FullName $srcFiles $sqliteObj -o $exe
        if ($LASTEXITCODE -ne 0) { Write-Host "  COMPILE FAIL : $($t.Name)" -ForegroundColor Red; $fail++; continue }
        & $exe | Out-Null
        if ($LASTEXITCODE -eq 0) { Write-Host "  PASS : $($t.Name)" -ForegroundColor Green; $pass++ }
        else { Write-Host "  FAIL : $($t.Name)" -ForegroundColor Red; $fail++ }
    }
    Write-Host "$pass tests OK, $fail echecs" -ForegroundColor $(if ($fail) { "Red" } else { "Green" })
    if ($fail) { exit 1 }
    exit 0
}

Write-Host "Compilation..." -ForegroundColor Cyan
$exe = "$proj\bank2.exe"
& g++ @flags "$proj\main.cpp" $srcFiles $sqliteObj -o $exe
if ($LASTEXITCODE -ne 0) { Write-Host "Echec de compilation." -ForegroundColor Red; exit 1 }
Write-Host "OK -> $exe" -ForegroundColor Green

if ($Run) {
    Write-Host "--- Execution ---" -ForegroundColor Cyan
    & $exe
}
