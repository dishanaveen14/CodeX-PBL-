CFLAGS = -Wall -Wextra -O2 -pthread

all: build/logger build/tests/test_sender

tests: build/tests/test_logger_flood build/tests/test_logger_bad

build/logger: logger/logger_main.c common/ipc.h
	mkdir -p build
	gcc $(CFLAGS) $< -o $@ -lrt

build/tests/test_sender: tests/test_sender.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

build/tests/test_logger_flood: tests/test_logger_flood.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

build/tests/test_logger_bad: tests/test_logger_bad.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt

test: build/logger build/tests/test_logger_flood build/tests/test_logger_bad
	@echo "--- Running test_logger_flood ---"
	build/tests/test_logger_flood
	@echo "--- Running test_logger_bad ---"
	build/tests/test_logger_bad
	@echo "--- All tests passed ---"

clean:
	rm -f build/logger \
	      build/tests/test_sender \
	      build/tests/test_logger_flood \
	      build/tests/test_logger_bad \
	      sim.log*
