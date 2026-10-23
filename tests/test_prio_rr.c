/* LOCAL TEST ONLY - do not commit */
#include <stdio.h>
#include <string.h>
#include "common.h"
void gantt_add(Gantt *g, int pid, int s, int e){
    if (e<=s) return;
    if (g->count>0 && g->e[g->count-1].pid==pid && g->e[g->count-1].end==s){ g->e[g->count-1].end=e; return; }
    g->e[g->count].pid=pid; g->e[g->count].start=s; g->e[g->count].end=e; g->count++;
}
static void show(const char*name, void(*f)(Process*,int,Gantt*), int tq, int rr, Process *src,int n){
    Process p[MAX_PROC]; memcpy(p,src,sizeof(Process)*n); Gantt g={.count=0};
    if(rr) round_robin(p,n,tq,&g); else f(p,n,&g);
    printf("%s: ",name);
    for(int i=0;i<g.count;i++){ if(g.e[i].pid<0)printf("[idle %d-%d] ",g.e[i].start,g.e[i].end); else printf("[P%d %d-%d] ",g.e[i].pid,g.e[i].start,g.e[i].end);}
    printf("\n   ct/fs: "); for(int i=0;i<n;i++)printf("P%d=%d/%d ",p[i].pid,p[i].ct,p[i].first_start); printf("\n");
}
#define P(id,a,b,r) {id,a,b,r,0,0,0,0,0}
int main(void){
    Process t1[]={P(1,0,10,3),P(2,1,1,1),P(3,2,2,4),P(4,3,1,5),P(5,4,5,2)};
    Process t2[]={P(1,3,4,2),P(2,6,3,1)};
    Process t3[]={P(1,0,5,2),P(2,0,5,2),P(3,0,3,2)};
    Process t4[]={P(1,0,4,1),P(2,2,3,1),P(3,4,2,1)};  /* RR expiry-arrival */
    Process t5[]={P(7,0,3,1)};
    Process t6[]={P(1,0,2,1),P(2,6,2,1),P(3,7,1,0)};  /* mid idle */
    struct {const char*n;Process*p;int c;} T[]={{"T1",t1,5},{"T2",t2,2},{"T3",t3,3},{"T4",t4,3},{"T5(n=1)",t5,1},{"T6(mid idle)",t6,3}};
    for(int k=0;k<6;k++){
        printf("=== %s ===\n",T[k].n);
        show("PrioNP",priority_np,0,0,T[k].p,T[k].c);
        show("PrioP ",priority_p,0,0,T[k].p,T[k].c);
        show("RR tq2",0,2,1,T[k].p,T[k].c);
        show("RR tq4",0,4,1,T[k].p,T[k].c);
        show("RR tq1",0,1,1,T[k].p,T[k].c);
        show("RR tq99",0,99,1,T[k].p,T[k].c);
    }
    return 0;
}
