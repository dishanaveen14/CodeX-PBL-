# Makefile for Simulation and Comparative Analysis of Page Replacement Algorithms
# Project: Page Replacement Algorithm Simulator
# Integration Owner: Person 4

CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -O2
INCLUDES = -Iinclude -Isrc

# Check for pkg-config and GTK+ 3.0
PKG_CONFIG := $(shell which pkg-config 2>/dev/null)
GTK_AVAILABLE := $(shell pkg-config --exists gtk+-3.0 2>/dev/null && echo yes || echo no)

ifeq ($(GTK_AVAILABLE),yes)
    GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
    GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0)
else
    GTK_CFLAGS := 
    GTK_LIBS   := 
endif

# Directories
SRC_DIR   = src
INC_DIR   = include
TEST_DIR  = tests
BUILD_DIR = build
BIN_DIR   = bin

# Target executable
TARGET = $(BIN_DIR)/page_simulator

# Source files for core and utilities (non-GUI)
CORE_SRCS = $(SRC_DIR)/algorithms/fifo.c \
            $(SRC_DIR)/algorithms/lru.c \
            $(SRC_DIR)/algorithms/optimal.c \
            $(SRC_DIR)/core/simulation.c \
            $(SRC_DIR)/core/statistics.c \
            $(SRC_DIR)/utils/memory.c

# Source files for GUI
GUI_SRCS  = $(SRC_DIR)/gui/input_validation.c \
            $(SRC_DIR)/gui/results_view.c \
            $(SRC_DIR)/gui/main_window.c \
            $(SRC_DIR)/gui/main.c

# All application sources
APP_SRCS  = $(CORE_SRCS) $(GUI_SRCS)
APP_OBJS  = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(APP_SRCS))

# Test executables
TEST_BINS = $(BIN_DIR)/test_fifo \
            $(BIN_DIR)/test_lru \
            $(BIN_DIR)/test_optimal \
            $(BIN_DIR)/test_simulation \
            $(BIN_DIR)/test_integration

.PHONY: all clean run test check-gtk

all: check-gtk $(TARGET)

check-gtk:
ifeq ($(GTK_AVAILABLE),no)
	@echo "=========================================================================="
	@echo "[WARNING] GTK+ 3.0 was not detected via pkg-config."
	@echo "To compile the full graphical simulator, install GTK+ 3 development libraries:"
	@echo "  - On Debian/Ubuntu: sudo apt-get install libgtk-3-dev pkg-config"
	@echo "  - On Fedora:        sudo dnf install gtk3-devel pkgconfig"
	@echo "  - On MSYS2/MinGW:   pacman -S mingw-w64-ucrt-x86_64-gtk3 pkg-config"
	@echo "You can still run independent core/unit tests using: make test"
	@echo "=========================================================================="
	@exit 1
endif

$(TARGET): $(APP_OBJS) | $(BIN_DIR)
	@echo "[LD] Linking $@"
	$(CC) $(CFLAGS) $^ -o $@ $(GTK_LIBS) -lm
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(CC) $(CFLAGS) $(INCLUDES) $(GTK_CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

run: all
	@echo "[RUN] Executing $(TARGET)"
	./$(TARGET)

clean:
	@echo "[CLEAN] Removing build artifacts"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean completed."

# Tests compilation and execution (Core tests do not require GTK)
test: $(TEST_BINS)
	@echo "=========================================="
	@echo "Running all test suites..."
	@echo "=========================================="
	@./$(BIN_DIR)/test_fifo
	@./$(BIN_DIR)/test_lru
	@./$(BIN_DIR)/test_optimal
	@./$(BIN_DIR)/test_simulation
	@./$(BIN_DIR)/test_integration
	@echo "=========================================="
	@echo "All tests passed successfully!"
	@echo "=========================================="

$(BIN_DIR)/test_fifo: $(TEST_DIR)/test_fifo.c $(SRC_DIR)/algorithms/fifo.c $(SRC_DIR)/utils/memory.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BIN_DIR)/test_lru: $(TEST_DIR)/test_lru.c $(SRC_DIR)/algorithms/lru.c $(SRC_DIR)/utils/memory.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BIN_DIR)/test_optimal: $(TEST_DIR)/test_optimal.c $(SRC_DIR)/algorithms/optimal.c $(SRC_DIR)/utils/memory.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BIN_DIR)/test_simulation: $(TEST_DIR)/test_simulation.c $(CORE_SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -lm

$(BIN_DIR)/test_integration: $(TEST_DIR)/test_integration.c $(CORE_SRCS) $(SRC_DIR)/gui/input_validation.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -lm

