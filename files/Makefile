CFLAGS = -Wall -Wextra -O2 -pthread
all: build/logger build/tests/test_sender
build/logger: logger/logger_main.c common/ipc.h
	mkdir -p build
	gcc $(CFLAGS) $< -o $@ -lrt
build/tests/test_sender: tests/test_sender.c common/ipc.h
	mkdir -p build/tests
	gcc $(CFLAGS) $< -o $@ -lrt
clean:
	rm -f build/logger build/tests/test_sender sim.log*
