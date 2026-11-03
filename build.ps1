# PowerShell Build Script for Deadlock Simulation System
param (
    [string]$Target = "all"
)

$ErrorActionPreference = "Stop"

Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host " Deadlock Simulation System - PowerShell Build Runner           " -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan

$CC = "gcc"
$CFLAGS = @("-std=c11", "-Wall", "-Wextra", "-Iinclude")
$LDFLAGS_GUI = @("-mwindows", "-lcomctl32", "-lgdi32")

if (!(Test-Path "bin")) { New-Item -ItemType Directory -Path "bin" | Out-Null }
if (!(Test-Path "build")) { New-Item -ItemType Directory -Path "build" | Out-Null }

$SRCS = @(
    "src/main.c",
    "src/common/system_state.c",
    "src/common/error.c",
    "src/person1_process_resource/process_mgmt.c",
    "src/person1_process_resource/resource_mgmt.c",
    "src/person1_process_resource/matrix_mgmt.c",
    "src/person2_bankers/bankers.c",
    "src/person3_detection_recovery/detection.c",
    "src/person3_detection_recovery/rag.c",
    "src/person3_detection_recovery/recovery.c",
    "src/person4_integration/coordinator.c",
    "src/gui/gui_app.c",
    "src/gui/panel_dashboard.c",
    "src/gui/panel_setup.c",
    "src/gui/panel_bankers.c",
    "src/gui/panel_detection.c",
    "src/gui/panel_rag.c",
    "src/utils/logger.c"
)

$TEST_SRCS = @(
    "tests/test_skeleton.c",
    "src/common/system_state.c",
    "src/common/error.c",
    "src/person1_process_resource/process_mgmt.c",
    "src/person1_process_resource/resource_mgmt.c",
    "src/person1_process_resource/matrix_mgmt.c",
    "src/person2_bankers/bankers.c",
    "src/person3_detection_recovery/detection.c",
    "src/person3_detection_recovery/rag.c",
    "src/person3_detection_recovery/recovery.c",
    "src/person4_integration/coordinator.c"
)

switch ($Target.ToLower()) {
    "clean" {
        Write-Host "[CLEAN] Cleaning build and bin directories..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force -ErrorAction SilentlyContinue bin, build
        New-Item -ItemType Directory -Path "bin" | Out-Null
        New-Item -ItemType Directory -Path "build" | Out-Null
        Write-Host "[SUCCESS] Clean completed." -ForegroundColor Green
    }
    "test" {
        Write-Host "[BUILD] Compiling test suite..." -ForegroundColor Yellow
        & $CC @CFLAGS @TEST_SRCS -o "bin/test_skeleton.exe"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Test compilation failed!" -ForegroundColor Red
            exit $LASTEXITCODE
        }
        Write-Host "[RUN] Executing test runner..." -ForegroundColor Yellow
        & "bin/test_skeleton.exe"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Test suite failed!" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    }
    "run" {
        if (!(Test-Path "bin/deadlock_sim.exe")) {
            Write-Host "[BUILD] Executable not found. Compiling first..." -ForegroundColor Yellow
            & $CC @CFLAGS @SRCS @LDFLAGS_GUI -o "bin/deadlock_sim.exe"
            if ($LASTEXITCODE -ne 0) {
                Write-Host "[ERROR] Compilation failed!" -ForegroundColor Red
                exit $LASTEXITCODE
            }
        }
        Write-Host "[RUN] Launching bin/deadlock_sim.exe..." -ForegroundColor Green
        Start-Process "bin/deadlock_sim.exe"
    }
    default {
        Write-Host "[BUILD] Compiling application..." -ForegroundColor Yellow
        & $CC @CFLAGS @SRCS @LDFLAGS_GUI -o "bin/deadlock_sim.exe"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Compilation failed!" -ForegroundColor Red
            exit $LASTEXITCODE
        }
        Write-Host "[SUCCESS] Application compiled: bin/deadlock_sim.exe" -ForegroundColor Green
    }
}

