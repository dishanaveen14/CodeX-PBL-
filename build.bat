@echo off
setlocal enabledelayedexpansion

echo =================================================================
echo  Deadlock Simulation System - Build Tool (C / GCC / Win32)
echo =================================================================

set CC=gcc
set CFLAGS=-std=c11 -Wall -Wextra -Iinclude
set LDFLAGS_GUI=-mwindows -lcomctl32 -lgdi32
set LDFLAGS_TEST=

if not exist bin mkdir bin
if not exist build mkdir build

set TARGET=%1
if "%TARGET%"=="" set TARGET=all

if /I "%TARGET%"=="clean" goto do_clean
if /I "%TARGET%"=="test" goto do_test
if /I "%TARGET%"=="run" goto do_run
if /I "%TARGET%"=="all" goto do_all

:do_all
echo [BUILD] Compiling application source files...

set SRCS=src/main.c src/common/system_state.c src/common/error.c ^
src/person1_process_resource/process_mgmt.c src/person1_process_resource/resource_mgmt.c src/person1_process_resource/matrix_mgmt.c ^
src/person2_bankers/bankers.c ^
src/person3_detection_recovery/detection.c src/person3_detection_recovery/rag.c src/person3_detection_recovery/recovery.c ^
src/person4_integration/coordinator.c ^
src/gui/gui_app.c src/gui/panel_dashboard.c src/gui/panel_setup.c src/gui/panel_bankers.c src/gui/panel_detection.c src/gui/panel_rag.c ^
src/utils/logger.c

%CC% %CFLAGS% %SRCS% %LDFLAGS_GUI% -o bin\deadlock_sim.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Application build failed!
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Application built successfully: bin\deadlock_sim.exe
goto :eof

:do_test
echo [BUILD] Compiling test suite...

set TEST_SRCS=tests/test_skeleton.c src/common/system_state.c src/common/error.c ^
src/person1_process_resource/process_mgmt.c src/person1_process_resource/resource_mgmt.c src/person1_process_resource/matrix_mgmt.c ^
src/person2_bankers/bankers.c ^
src/person3_detection_recovery/detection.c src/person3_detection_recovery/rag.c src/person3_detection_recovery/recovery.c ^
src/person4_integration/coordinator.c

%CC% %CFLAGS% %TEST_SRCS% -o bin\test_skeleton.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Test build failed!
    exit /b %ERRORLEVEL%
)

echo [RUN] Running test suite...
bin\test_skeleton.exe
goto :eof

:do_run
if not exist bin\deadlock_sim.exe (
    call :do_all
)
echo [RUN] Launching bin\deadlock_sim.exe...
start bin\deadlock_sim.exe
goto :eof

:do_clean
echo [CLEAN] Removing build artifacts...
if exist bin rmdir /s /q bin
if exist build rmdir /s /q build
mkdir bin
mkdir build
echo [SUCCESS] Clean completed.
goto :eof

