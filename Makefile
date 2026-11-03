# ==============================================================================
# Deadlock Simulation System - Makefile
# PBL Project (C, Win32, Modular Architecture)
# ==============================================================================

CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Iinclude
LDFLAGS_GUI = -mwindows -lcomctl32 -lgdi32

BIN_DIR = bin
BUILD_DIR = build
TARGET = $(BIN_DIR)/deadlock_sim.exe
TEST_TARGET = $(BIN_DIR)/test_skeleton.exe

SRCS = src/main.c \
       src/common/system_state.c \
       src/common/error.c \
       src/person1_process_resource/process_mgmt.c \
       src/person1_process_resource/resource_mgmt.c \
       src/person1_process_resource/matrix_mgmt.c \
       src/person2_bankers/bankers.c \
       src/person3_detection_recovery/detection.c \
       src/person3_detection_recovery/rag.c \
       src/person3_detection_recovery/recovery.c \
       src/person4_integration/coordinator.c \
       src/gui/gui_app.c \
       src/gui/panel_dashboard.c \
       src/gui/panel_setup.c \
       src/gui/panel_bankers.c \
       src/gui/panel_detection.c \
       src/gui/panel_rag.c \
       src/utils/logger.c

TEST_SRCS = tests/test_skeleton.c \
            src/common/system_state.c \
            src/common/error.c \
            src/person1_process_resource/process_mgmt.c \
            src/person1_process_resource/resource_mgmt.c \
            src/person1_process_resource/matrix_mgmt.c \
            src/person2_bankers/bankers.c \
            src/person3_detection_recovery/detection.c \
            src/person3_detection_recovery/rag.c \
            src/person3_detection_recovery/recovery.c \
            src/person4_integration/coordinator.c

.PHONY: all clean test run dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BIN_DIR) $(BUILD_DIR) 2>nul || (if not exist $(BIN_DIR) mkdir $(BIN_DIR)) && (if not exist $(BUILD_DIR) mkdir $(BUILD_DIR))

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS_GUI) -o $(TARGET)

test: dirs $(TEST_TARGET)
	@echo Running test suite...
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CC) $(CFLAGS) $(TEST_SRCS) -o $(TEST_TARGET)

run: all
	@cmd /c start $(TARGET)

clean:
	@cmd /c "if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)"
	@cmd /c "if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)"

