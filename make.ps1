
# ./make.ps1 
# ./make.ps1 clean
# ./make.ps1 build
# ./make.ps1 run
# ./make.ps1 rebuild
# ./make.ps1 play -Mode DEBUG


<#
.SYNOPSIS
    Helper script to build, clean, and run the Raylib C++ project.
#>

param (
    [ValidateSet("build", "clean", "run", "rebuild", "play")]
    [string]$Action = "play",

    [ValidateSet("RELEASE", "DEBUG")]
    [string]$Mode = "RELEASE"
)

$ExeName = "game.exe"

function Run-Make {
    param([string]$Target, [string]$BuildMode)
    
    if ($Target -eq "clean") {
        mingw32-make clean
    } else {
        mingw32-make BUILD_MODE=$BuildMode
    }
}

switch ($Action) {
    "clean" {
        Write-Host "🧹 Cleaning project..." -ForegroundColor Yellow
        Run-Make -Target "clean"
    }
    "build" {
        Write-Host "🔨 Building project in $Mode mode..." -ForegroundColor Cyan
        Run-Make -Target "all" -BuildMode $Mode
    }
    "rebuild" {
        Write-Host "♻️ Rebuilding project in $Mode mode..." -ForegroundColor Cyan
        Run-Make -Target "clean"
        Run-Make -Target "all" -BuildMode $Mode
    }
    "run" {
        if (Test-Path ".\$ExeName") {
            Write-Host "🚀 Running $ExeName..." -ForegroundColor Green
            & ".\$ExeName"
        } else {
            Write-Host "❌ Error: $ExeName not found. Please build the project first." -ForegroundColor Red
        }
    }
    "play" {
        Write-Host "⚙️ Building and starting project in $Mode mode..." -ForegroundColor Cyan
        Run-Make -Target "all" -BuildMode $Mode
        
        # Check the exact exit code of mingw32-make
        if ($LASTEXITCODE -eq 0) {
            Write-Host "🚀 Compilation successful! Running $ExeName..." -ForegroundColor Green
            & ".\$ExeName"
        } else {
            Write-Host "❌ Compilation failed. Check the C++ errors above." -ForegroundColor Red
        }
    }
}
