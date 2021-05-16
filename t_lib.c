#include "t_lib.h"

struct tcb *running;
struct tcb *ready;
struct allThreads *origin;

void t_yield()
{
    if(ready!=NULL){
        struct tcb *old = running;
        struct tcb *new=ready;
        struct tcb *temp=new;
        if(temp->next!=NULL){
            while(temp->next!=NULL){
                temp=temp->next;
            }
        }
        temp->next=old;
        ready=ready->next;
        new->next=NULL;
        running=new;
        //printf("this is the new id: %d\n",new->thread_id);
        swapcontext(old->thread_context, new->thread_context);


    }
}

void t_init()
{
  shutdownFlag = 0;
  //ucontext_t *tmp, tmpTest1, tmpTest2;
  struct tcb *tmp=malloc(sizeof(struct tcb));
  tmp->thread_context=(ucontext_t *)malloc(sizeof(ucontext_t));
  tmp->next=NULL;
  tmp->thread_id=0;
  tmp->thread_priority=0;
  sem_t *s1,*s2;
  sem_init(&s1,1);
  sem_init(&s2,0);
  tmp->mq_sem = s1;
  tmp->br_sem = s2;
  getcontext(tmp->thread_context);
  running = tmp;
  ready=NULL;
  struct allThreads *t = malloc(sizeof(struct allThreads));
  t->next = NULL;
  t->thread = tmp;
  origin = t;
  struct tcb *looking = (origin->thread);
  printf("hello: %d\n",looking->thread_id);
}

int t_create(void (*fct)(int), int id, int pri)
{
  size_t sz = 0x10000;

  ucontext_t *uc;
  uc = (ucontext_t *) malloc(sizeof(ucontext_t));

  getcontext(uc);
/***
  uc->uc_stack.ss_sp = mmap(0, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANON, -1, 0);
***/
  uc->uc_stack.ss_sp = malloc(sz);  /* new statement */
  uc->uc_stack.ss_size = sz;
  uc->uc_stack.ss_flags = 0;
  uc->uc_link = running->thread_context; 
  makecontext(uc, (void (*)(void)) fct, 1, id);
  //ready = uc; //we don't need this line because we're not using the ready point like this anymore (using queue system)

  struct tcb *newTcb = malloc(sizeof(struct tcb));
  newTcb->next=NULL;
  newTcb->thread_priority = pri;
  newTcb->thread_id = id;
  sem_t *s1,*s2;
  sem_init(&s1,1);
  sem_init(&s2,0);
  newTcb->mq_sem = s1;
  newTcb->br_sem = s2;
  newTcb->thread_context = uc;
  struct tcb *temp=ready;
  if(temp==NULL){
      ready=newTcb;
  } else {
      while(temp->next!=NULL){
          temp=temp->next;
      }
      temp->next=newTcb;
  }
  struct allThreads *aThread = malloc(sizeof(struct allThreads));
  struct tcb *looking;
  aThread->next = NULL;
  aThread->thread = newTcb;
  struct allThreads *tt = origin;
  while(tt->next != NULL) {
      tt = tt->next;
  }
  tt->next = aThread;
  looking = (origin->thread);
  printf("this is the thread id: %d\n",looking->thread_id);
}

void t_terminate()
{
    struct tcb *old = running;
    struct tcb *new=ready;
    struct tcb *temp=new;

    //remove from allThreads
    struct allThreads *aThread = origin;
    struct allThreads *aPrev;

    //delete the semaphores attached to tcb
    sem_destroy(&(old->mq_sem));
    sem_destroy(&(old->br_sem));

    //delete from allThreads linked list
    while(aThread != NULL) { 
        struct tcb *t;
        t = (aThread->thread);
        if(t->thread_id == running->thread_id) {
            if(aPrev == NULL) { //we are terminating the original thread which idk if that can actually happen
                origin = aThread->next;
            }
            else{
                aPrev->next = aThread->next;
            }
            free(aThread);
            break;
        }
        aPrev = aThread;
        aThread = aThread->next;
    }

    //delete from running queue
    if(ready!=NULL){        
        ready=ready->next;
        new->next=NULL;
        running=new;
        //printf("this is the new id: %d\n",new->thread_id);
        messageNode *msgList = old->msg;
        messageNode *msgPrev;
        while(msgList != NULL) {
            msgPrev = msgList;
            free(msgPrev->message);
            free(msgPrev);
            msgList = msgList->next;
        }
        free(old->thread_context->uc_stack.ss_sp);
        free(old->thread_context);
        free(old);
        //printf("Old is freed\n");
        setcontext(running->thread_context);
    }
    else {
        printf("no threads left, finishing program\n");
    }
}

void t_shutdown() { //free running queue then free entire ready queue
    //how it works: gather every single tcb still running through allThreads list, free everything
    shutdownFlag = 1;
    struct allThreads *ptr = origin->next;
    struct allThreads *prev = ptr;
    while(ptr != NULL) {
        ptr = ptr->next;
        printf("looking at thread with id: %d\n",prev->thread->thread_id);
        sem_signal(prev->thread->br_sem);
        t_yield();
        //delete the semaphores attached to tcb
        printf("done\n");
        //sem_destroy(&(prev->thread->mq_sem));
        //sem_destroy(&(prev->thread->br_sem));

        //delete message queue
        /*struct messageNode *m = prev->thread->msg;
        struct messageNode *mPrev = prev->thread->msg;
        while(m != NULL) {
            m = m->next;
            free(mPrev->message);
            free(mPrev);
            mPrev = m;
        }*/

        //delete context
        //free(prev->thread->thread_context->uc_stack.ss_sp);
        //free(prev->thread->thread_context);
        

        //free(prev->thread);
        //free(prev);
        prev = ptr;
    }
    sem_destroy(&(running->mq_sem));
    sem_destroy(&(running->br_sem));
    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    free(running);
    free(origin);
    running=NULL;
    /*
    struct tcb *temp=ready;
    while(ready!=NULL){
        //delete message queue
        struct messageNode *m = prev->thread->msg;
        struct messageNode *mPrev = prev->thread->msg;
        while(m != NULL) {
            m = m->next;
            free(mPrev->message);
            free(mPrev);
            mPrev = m;
        }
        free(temp->thread_context->uc_stack.ss_sp);
        free(temp->thread_context);
        free(temp);
        ready=ready->next;
        temp=ready;
    }
    ready=NULL;

    //delete the semaphores attached to tcb
    sem_destroy(&(running->mq_sem));
    sem_destroy(&(running->br_sem));

    

    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    free(running);
    running=NULL;*/
}

void sem_init(sem_t **sp, int sem_count){
    *sp=malloc(sizeof(sem_t));
    (*sp)->count=sem_count;
    (*sp)->q=NULL;
}

void sem_wait(sem_t *sp){
    sp->count--;
   // printf("Executing sem_wait %d\n",sp->count);
    if(sp->count < 0){
        struct tcb *old=running;
        struct tcb *new=ready;
        struct tcb *temp=sp->q;
        if(temp==NULL){
            sp->q=old;
        } else {
            while(temp->next != NULL){
                temp=temp->next;
            }
            temp->next=old;
        }
        ready=ready->next;
        new->next=NULL;
        running=new;
        swapcontext(old->thread_context, new->thread_context);
    }
}

void sem_signal(sem_t *sp){
    sp->count++;
    //printf("Executing sem signal %d \n",sp->count);
    if (sp->q != NULL){
        struct tcb *temp=sp->q;
        struct tcb *new=ready;
        sp->q=sp->q->next;
        if(new==NULL){
            ready=temp;
            ready->next=NULL;
        } else {
            while(new->next!=NULL){
                new=new->next;
            }
            new->next=temp;
            new->next->next=NULL;
        }
    }
}

void sem_destroy(sem_t **sp){
    if(*sp == NULL) {
        return;
    }
    if((*sp)->q==NULL){
        free(*sp);
    } 
    else {
        struct tcb *temp=(*sp)->q;
        struct tcb *tempR=ready;
        if(ready == NULL) {
            ready = temp;
        }
         else {
            while(tempR->next != NULL){
                tempR = tempR->next;
            }
        tempR->next=temp;
        }
        free(*sp);
    }
    (*sp) = NULL;
}

    /*
    mbox deposit puts in a new message in the queue
    mbox withdraw takes out that message
    mbox destroy frees space
    */
    void mbox_create(mbox **mb){
        *mb=malloc(sizeof(mbox));
        (*mb)->msg=NULL;
        sem_t *s1;
        sem_init(&s1,1);
        (*mb)->mbox_sem = s1;
    }

    void mbox_deposit(mbox *mb,char* msg, int len){
        sem_wait(mb->mbox_sem);
        messageNode *new_msg=malloc(sizeof(messageNode));
        new_msg->message=malloc(sizeof(char)*len);
        new_msg->len=len;
        new_msg->sender=running->thread_id;
        new_msg->next = NULL;
        strncpy(new_msg->message, msg, len);
        if(mb->msg==NULL){
            mb->msg=new_msg;
        } else {
            messageNode *top = mb->msg;
            while(top->next != NULL){
                top=top->next;
            }
            top->next=new_msg;
        }
        sem_signal(mb->mbox_sem);
        
    }

    void mbox_withdraw(mbox *mb, char* msg, int *len){
        sem_wait(mb->mbox_sem);
        messageNode *tmp=mb->msg;
        if(tmp==NULL){
            *len = 0;
        }
        else{
            *len=tmp->len;
            strncpy(msg,tmp->message,*len);
            mb->msg=mb->msg->next;
            free(tmp->message);
            free(tmp);
        }
        sem_signal(mb->mbox_sem);
    }

    void mbox_destroy(mbox **mb){
        mbox *box = *mb;
        messageNode *temp=((*mb)->msg);
        messageNode *temp2=temp;
        while(temp!=NULL){
            temp=temp->next;
            free(temp2->message);
            free(temp2);
        }
        sem_destroy(&(*mb)->mbox_sem);
        free(*mb);
    }

    struct tcb* getThread(int tid) {
        struct allThreads *ptr = origin;
        while(ptr != NULL) {
            if(ptr->thread->thread_id == tid) {
                return ptr->thread;
            }
            ptr = ptr->next;
        }
        printf("thread with tid %d not found!\n",tid);
        return NULL;
    }

    void send(int tid, char *msg, int len) {
        struct tcb *thread = getThread(tid);
        messageNode *new_msg=malloc(sizeof(messageNode));
        if(thread == NULL) {
            printf("send failed!\n");
            free(new_msg);
            return;
        }
        sem_wait(thread->mq_sem); //adding to message queue is a critical section
        new_msg->message=malloc(sizeof(char)*len);
        new_msg->len=len;
        new_msg->sender=running->thread_id;
        new_msg->receiver = tid;
        new_msg->next = NULL;
        strncpy(new_msg->message, msg, len);
        messageNode *ptr = thread->msg;
        if(ptr == NULL) {
            thread->msg = new_msg;
        }
        else {
            while(ptr->next != NULL) {
                ptr = ptr->next;
            }
            ptr->next = new_msg;
        }
        sem_signal(thread->br_sem);
        sem_signal(thread->mq_sem);
        
    }

    void setToZero(sem_t *sem) {
        sem->count = 0;
    }

    /*
    When a thread waits on a blocking receive, its 
    TCB is queued on the queue of the semaphore (i.e., br_sem) 
    that is part of the TCB
    */
    void receive(int *tid, char *msg, int *len) {
        //printf("this is the message: %s\n",ptr->message);
        while(1) { //will go until forcibly returned out
            messageNode *ptr = running->msg;
            messageNode *prev = NULL;
            while(ptr != NULL) {
                if(*tid == 0 || ptr->sender == *tid) {//message found
                    printf("message found!\n");
                    *tid = ptr->sender;
                    strcpy(msg, ptr->message);
                    *len = ptr->len;
                    if(prev == NULL) {
                        running->msg = running->msg->next;
                    }
                    else{
                        prev->next = prev->next->next;
                        //running->msg = prev;
                    }
                    free(ptr->message);
                    free(ptr);
                    return;
                }
                prev = ptr;
                ptr = ptr->next;
            }
            printf("nothing found!\n");
            setToZero(running->br_sem);
            sem_wait(running->br_sem);
            if(shutdownFlag == 1) {
                printf("this is the shutdown signal being read\n");
                return;
            }
        }
    }