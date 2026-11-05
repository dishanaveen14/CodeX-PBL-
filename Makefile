# ==============================================================================
# Makefile for POSIX Inter-Process Communication (IPC) Project
#
# Team Members & Responsibilities:
#   - Student 1: Pipes and Message Queues (src/pipes.c, src/message_queue.c)
#   - Student 2: Sockets and Shared Memory (src/sockets.c, src/shared_memory.c)
#   - Student 3: Semaphores and Mutex Locks (src/semaphores.c, src/mutex.c)
#   - Student 4: Integration and Main Application (src/main.c, src/gui.c, src/ipc_logger.c)
# ==============================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -pthread -Iinclude
TARGET  := ipc_project
BUILD   := build

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET  := ipc_project.exe
    LDFLAGS := -static -lws2_32 -lgdi32 -lcomctl32
    RM      := del /Q /F
    MKDIR   := if not exist $(BUILD) mkdir $(BUILD)
else
    TARGET  := ipc_project
    LDFLAGS := -pthread -lrt
    RM      := rm -rf
    MKDIR   := mkdir -p $(BUILD)
endif

SRCS := src/ipc_logger.c \
        src/pipes.c \
        src/message_queue.c \
        src/sockets.c \
        src/shared_memory.c \
        src/semaphores.c \
        src/mutex.c \
        src/gui.c \
        src/main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

gui: $(TARGET)
	./$(TARGET) --gui

test: $(TARGET)
	./$(TARGET) -a

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(TARGET) *.o 2>nul
else
	$(RM) $(TARGET) *.o $(BUILD)
endif

.PHONY: all run gui test clean
