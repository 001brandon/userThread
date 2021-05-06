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
};

typedef struct tcb tcb;

typedef struct {
    int count;
    tcb *q;
} sem_t;

#endif
