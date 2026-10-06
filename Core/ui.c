#include "ipc_protocol.h"
#include <windows.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int read_integer(const char *prompt,int *value)
{
    char line[128],*end; long n; printf("%s",prompt); if(!fgets(line,sizeof line,stdin)) return 0;
    errno=0; n=strtol(line,&end,10); while(isspace((unsigned char)*end)) ++end;
    if(end==line || *end!='\0' || errno==ERANGE || n<INT_MIN || n>INT_MAX) { puts("Please enter a valid integer."); return -1; }
    *value=(int)n; return 1;
}
static int transact(HANDLE p,Request *q)
{
    Response r; DWORD n;
    if(!WriteFile(p,q,sizeof *q,&n,NULL)||n!=sizeof *q) { puts("Could not send command to Core Process."); return 0; }
    if(!ReadFile(p,&r,sizeof r,&n,NULL)||n!=sizeof r) { puts("Could not receive response from Core Process."); return 0; }
    puts(r.text); return 1;
}
int main(void)
{
    HANDLE pipe=CreateFileA(CORE_PIPE_NAME,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    int choice,status,running=1; char op[32]; Request q;
    if(pipe==INVALID_HANDLE_VALUE) { fprintf(stderr,"Cannot connect to Core Process (Windows error %lu).\nStart core.exe first, then launch ui.exe.\n",GetLastError()); return 1; }
    puts("Connected to Student 2 Core Process.");
    while(running) {
        printf("\n========== STUDENT 1 UI ==========\n1. Execute CPU Operation\n2. Write to Memory\n3. Read from Memory\n4. Push to Stack\n5. Pop from Stack\n6. Enqueue\n7. Dequeue\n8. Display Core State\n9. Exit Core\n");
        status=read_integer("Enter your choice: ",&choice); if(status==0) break; if(status<0) continue;
        memset(&q,0,sizeof q); q.command=choice;
        switch(choice) {
        case 1:
            printf("Operation (ADD/SUB/MUL/DIV): "); if(!fgets(op,sizeof op,stdin)) { running=0; continue; }
            op[strcspn(op,"\r\n")]='\0'; if(strlen(op)>=sizeof q.operation) { puts("Operation name too long."); continue; }
            { int i; for(i=0;op[i];++i) q.operation[i]=(char)toupper((unsigned char)op[i]); }
            status=read_integer("First value: ",&q.first); if(status<=0) { if(status==0) running=0; continue; }
            status=read_integer("Second value: ",&q.second); if(status<=0) { if(status==0) running=0; continue; } break;
        case 2:
            status=read_integer("Memory address (0-99): ",&q.first); if(status<=0) { if(status==0) running=0; continue; }
            status=read_integer("Value: ",&q.second); if(status<=0) { if(status==0) running=0; continue; } break;
        case 3: case 4: case 6:
            status=read_integer(choice==3?"Memory address (0-99): ":"Value: ",&q.first); if(status<=0) { if(status==0) running=0; continue; } break;
        case 5: q.command=CMD_STACK_POP; break;
        case 7: q.command=CMD_QUEUE_DEQUEUE; break;
        case 8: q.command=CMD_SHOW_STATE; break;
        case 9: q.command=CMD_EXIT; break;
        default: puts("Choose a menu option from 1 to 9."); continue;
        }
        if(!transact(pipe,&q)) { running=0; continue; }
        if(q.command==CMD_EXIT) running=0;
    }
    CloseHandle(pipe); puts("UI Process closed."); return 0;
}
