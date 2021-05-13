/*
 * types used by thread library
 */
#include <stdio.h>
#ifndef TLIBH
#define TLIBH

#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <string.h>
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

struct messageNode {
         char *message;     // copy of the message 
         int  len;          // length of the message 
         int  sender;       // TID of sender thread 
         int  receiver;     // TID of receiver thread 
         struct messageNode *next; // pointer to next node 
}; typedef struct messageNode messageNode;

struct mbox {
	  struct messageNode  *msg;       // message queue
	  sem_t               *mbox_sem;  // used as lock
}; typedef struct mbox mbox;


#endif
