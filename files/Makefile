# Makefile for CodeX-PBL OS Simulation (IPC Addition Demo)

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -pthread
LDFLAGS := -lrt
BUILD   := build

all: $(BUILD)/logger $(BUILD)/core/core $(BUILD)/tests/test_sender

# -------------------------------------------------------------------------
# Logger Process
# -------------------------------------------------------------------------
$(BUILD)/logger: logger/logger_main.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# -------------------------------------------------------------------------
# Core Process
# -------------------------------------------------------------------------
$(BUILD)/core/core: core/core_main.c
	mkdir -p $(BUILD)/core
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# -------------------------------------------------------------------------
# Tests / Tools
# -------------------------------------------------------------------------
$(BUILD)/tests/test_sender: tests/test_sender.c
	mkdir -p $(BUILD)/tests
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD)/tests/test_logger_flood: tests/test_logger_flood.c
	mkdir -p $(BUILD)/tests
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD)/tests/test_logger_bad: tests/test_logger_bad.c
	mkdir -p $(BUILD)/tests
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests: $(BUILD)/tests/test_logger_flood $(BUILD)/tests/test_logger_bad

test: tests
	@echo "--- Running Logger robustness test ---"
	./$(BUILD)/tests/test_logger_bad
	@echo "--- Running Logger flood test ---"
	./$(BUILD)/tests/test_logger_flood

clean:
	rm -rf $(BUILD)
	rm -f sim.log*

.PHONY: all clean tests test
