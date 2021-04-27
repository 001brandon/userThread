/*
 * types used by thread library
 */
#include <stdio.h>
#ifndef TLIBH
#define TLIBH

#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include<stdio.h>

struct tcb {
	int         thread_id;
    int         thread_priority;
    ucontext_t *thread_context;
	struct tcb *next;
} *head, *qhead;

int count();
void append(struct tcb curr_tcb);
void add(struct tcb curr_tc);
void addafter(struct tcb curr_tc, int loc);
void insert(struct tcb curr_tcb);
int delete(int num);
void  display(struct tcb *r);
void freeall();
void qfreeall();
void qInsert(struct tcb curr_tcb);
struct tcb* qRemove();
void qdisplay(struct tcb *curr_tcb);






#endif
