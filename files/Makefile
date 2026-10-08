CFLAGS = -Wall -Wextra -O2 -pthread

# Core object files (shared between core binary and test_driver).
CORE_OBJS = build/core/cpu.o \
            build/core/memory.o \
            build/core/stack.o \
            build/core/queue.o

all: build/logger build/core/core build/core/test_driver build/tests/test_sender

tests: build/tests/test_logger_flood build/tests/test_logger_bad

# ---------- Logger ----------
build/logger: logger/logger_main.c common/ipc.h
	mkdir -p build
	gcc $(CFLAGS) $< -o $@ -lrt

# ---------- Core ----------
build/core/%.o: core/%.c
	mkdir -p build/core
	gcc $(CFLAGS) -c $< -o $@

build/core/core: core/core_main.c $(CORE_OBJS) common/ipc.h core/core_log.h
	mkdir -p build/core
	gcc $(CFLAGS) core/core_main.c $(CORE_OBJS) -o $@ -lrt

build/core/test_driver: core/test_driver.c $(CORE_OBJS) common/ipc.h core/core_log.h
	mkdir -p build/core
	gcc $(CFLAGS) core/test_driver.c $(CORE_OBJS) -o $@ -lrt

# ---------- Existing test utilities ----------
build/tests/test_sender: tests/test_sender.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

build/tests/test_logger_flood: tests/test_logger_flood.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

build/tests/test_logger_bad: tests/test_logger_bad.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

# ---------- Test targets ----------
test: build/logger build/tests/test_logger_flood build/tests/test_logger_bad
	@echo "--- Running test_logger_flood ---"
	build/tests/test_logger_flood
	@echo "--- Running test_logger_bad ---"
	build/tests/test_logger_bad
	@echo "--- All tests passed ---"

test-core: build/logger build/core/test_driver
	@echo "--- Running Core unit tests (Logger must be running) ---"
	build/core/test_driver

clean:
	rm -f build/logger \
	      build/core/core \
	      build/core/test_driver \
	      build/core/*.o \
	      build/tests/test_sender \
	      build/tests/test_logger_flood \
	      build/tests/test_logger_bad \
	      sim.log*
