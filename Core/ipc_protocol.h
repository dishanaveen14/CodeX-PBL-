#ifndef IPC_PROTOCOL_H
#define IPC_PROTOCOL_H
#define CORE_PIPE_NAME "\\\\.\\pipe\\Student2Core"
#define IPC_TEXT_SIZE 4096
typedef enum { CMD_CPU=1, CMD_MEMORY_WRITE, CMD_MEMORY_READ, CMD_STACK_PUSH, CMD_STACK_POP, CMD_QUEUE_ENQUEUE, CMD_QUEUE_DEQUEUE, CMD_SHOW_STATE, CMD_EXIT } Command;
typedef struct { int command; int first; int second; char operation[8]; } Request;
typedef struct { int success; int value; char text[IPC_TEXT_SIZE]; } Response;
#endif
