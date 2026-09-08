# Build script pour bank2_cpp
# Utilisation :  .\build.ps1          (compile)
#               .\build.ps1 -Run     (compile puis execute)
param([switch]$Run)

$ErrorActionPreference = "Stop"
$proj = $PSScriptRoot

# Compilateur g++ (fourni avec Webots sur cette machine)
$gccBin = "C:\Users\etudiant_adm\Webots\msys64\mingw64\bin"
$env:Path = "$gccBin;$gccBin\cpp;$env:Path"

$sources = @("$proj\main.cpp") + (Get-ChildItem -Recurse "$proj\src" -Filter *.cpp | ForEach-Object { $_.FullName })
$exe = "$proj\bank2.exe"

Write-Host "Compilation..." -ForegroundColor Cyan
& g++ -std=c++17 -Wall -Wextra -I"$proj\include" $sources -o $exe
if ($LASTEXITCODE -ne 0) { Write-Host "Echec de compilation." -ForegroundColor Red; exit 1 }
Write-Host "OK -> $exe" -ForegroundColor Green

if ($Run) {
    Write-Host "--- Execution ---" -ForegroundColor Cyan
    & $exe
}
