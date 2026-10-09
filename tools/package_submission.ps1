param([string]$Destination = 'submission/Automotive_Chain_Reaction_2107111')
$ErrorActionPreference = 'Stop'
$workspace = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$target = [IO.Path]::GetFullPath((Join-Path $workspace $Destination))
if (-not $target.StartsWith($workspace + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'The submission destination must be a child of this workspace.'
}
if (Test-Path -LiteralPath $target) { throw 'Destination already exists. Choose a new destination; existing submissions are never deleted.' }
$compiler = Split-Path -Parent (Get-Command g++ -ErrorAction Stop).Source
$runtimeNames = @('libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')
foreach ($name in $runtimeNames) {
    if (-not (Test-Path -LiteralPath (Join-Path $compiler $name))) { throw "Missing compiler runtime: $name" }
}
$null = New-Item -ItemType Directory -Path $target
foreach ($folder in @('src','include','lib','.vscode')) {
    Copy-Item -LiteralPath (Join-Path $workspace $folder) -Destination $target -Recurse
}
foreach ($file in @('cog_demo.exe','glfw3.dll','Makefile','CMakeLists.txt','run.bat','README.md')) {
    Copy-Item -LiteralPath (Join-Path $workspace $file) -Destination $target
}
foreach ($name in $runtimeNames) { Copy-Item -LiteralPath (Join-Path $compiler $name) -Destination $target }
$docs = Join-Path $target 'docs'
$null = New-Item -ItemType Directory -Path $docs
foreach ($file in @('PROPOSAL.md','VALIDATION.md','VIVA.md','HANDOFF_REVIEW.md','LIGHTING_REVIEW.md')) {
    Copy-Item -LiteralPath (Join-Path $workspace "docs/$file") -Destination $docs
}
$review = Join-Path $docs 'review-2026-10-09'
$null = New-Item -ItemType Directory -Path $review
# One current PNG review set; no raw BMP duplicates or earlier generations.
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $workspace 'docs/review-2026-10-09') -File) {
    if ($file.Extension -eq '.png' -or $file.Name -eq 'frames.csv') { Copy-Item -LiteralPath $file.FullName -Destination $review }
}
$lampReview = Join-Path $docs 'lamp-review'
$null = New-Item -ItemType Directory -Path $lampReview
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $workspace 'docs/lamp-review') -File) {
    if ($file.Extension -eq '.png' -or $file.Name -eq 'frames.csv') { Copy-Item -LiteralPath $file.FullName -Destination $lampReview }
}
$lightingReview = Join-Path $docs 'lighting-correction'
$null = New-Item -ItemType Directory -Path $lightingReview
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $workspace 'docs/lighting-correction') -File) {
    if ($file.Extension -eq '.png' -or $file.Name -eq 'frames.csv') { Copy-Item -LiteralPath $file.FullName -Destination $lightingReview }
}
@'
@echo off
cd /d "%~dp0"
cog_demo.exe
if errorlevel 1 pause
'@ | Set-Content -LiteralPath (Join-Path $target 'START_PROJECT.bat') -Encoding ASCII
@'
AUTOMOTIVE CHAIN REACTION - 2107111

Extract the entire submission folder before running.
Double-click START_PROJECT.bat or cog_demo.exe. Keep all DLLs beside it.
The presentation starts automatically. The car releases after 70 active seconds.
I switches to interactive mode. Follow the displayed key; C opens the larger guide.
Camera: WASD/QE and right mouse. Car: arrows after release. R resets.

No compiler is needed to run this build. Rebuilding with run.bat requires MinGW.
The system needs Windows and an OpenGL 3.3-capable graphics driver.
README.md contains full controls; docs/VIVA.md contains demonstration notes.
docs/HANDOFF_REVIEW.md links the current review frames.
SUBMISSION_CHECK.txt records the isolated runtime check.
'@ | Set-Content -LiteralPath (Join-Path $target 'START_HERE.txt') -Encoding ASCII
@'
Bundled project dependencies

GLFW: existing project header, link libraries and glfw3.dll.
GLAD: existing generated source and headers.
MinGW GCC runtime: libgcc_s_seh-1.dll and libstdc++-6.dll.
MinGW threading runtime: libwinpthread-1.dll (required by the two GCC DLLs).

Runtime DLLs were copied from the same C:/mingw64/bin toolchain that built
cog_demo.exe. Existing copyright notices in supplied headers/source are retained.
Windows system DLLs are supplied by Windows and are not copied into this folder.
'@ | Set-Content -LiteralPath (Join-Path $target 'THIRD_PARTY.txt') -Encoding ASCII
Write-Output "Staged current submission at $target"
Write-Output 'Run the isolated checks, then add SHA256SUMS.txt and create the ZIP.'
