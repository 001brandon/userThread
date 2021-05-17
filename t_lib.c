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
  origin = t; //set the top of the allThreads data structure to the thread calling t_init
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
  aThread->next = NULL;
  aThread->thread = newTcb;
  struct allThreads *tt = origin;
  if(tt == NULL) {
      origin = aThread;
  }
  else {
      while(tt->next != NULL) {
        tt = tt->next;
      }
      tt->next = aThread;
  }
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
            aThread = NULL;
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
        setcontext(running->thread_context);
    }
}

void t_shutdown() { //free running queue then free entire ready queue
    //how it works: gather every single tcb still running through allThreads list, free everything
    //gather everying single tcb by putting it all in ready queue, destroy all tcb's semaphores to do this
    shutdownFlag = 1;
    while(ready != NULL) { //allow the threads to all complete as is, some may get into blocking queues
        t_yield();
    }
    struct allThreads *ptr = origin;
    struct allThreads *prev = ptr;
    while(ptr != NULL) { //delete all semaphores, which will force every thread into ready or running queue
        ptr = ptr->next;
        //delete the semaphores attached to tcb
        sem_destroy(&(prev->thread->br_sem));
        sem_destroy(&(prev->thread->mq_sem));
        prev = ptr;
    }
    while(ready != NULL) { //The big assumption is that if you yield enough, all threads will reach t_terminate
        t_yield();
    }
    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    free(running);
    free(origin);
    running=NULL;
}

void sem_init(sem_t **sp, int sem_count){
    *sp=malloc(sizeof(sem_t));
    (*sp)->count=sem_count;
    (*sp)->q=NULL;
}

void sem_wait(sem_t *sp){
    if(sp == NULL) { //check if semaphore is null
        printf("semaphore doesn't exist\n");
        return;
    }
    sp->count--;
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
    if(sp == NULL) { //check if semaphore is null
        printf("semaphore doesn't exist\n");
        return;
    }
    sp->count++;
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
    if(*sp == NULL) { //if the semaphore is already destroyed, simply return
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
    (*sp) = NULL; //set semaphore to null once freed
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
        //printf("message is: %s\n",tmp->message);
        if(tmp==NULL){
            *len = 0;
        }
        else{
            *len=tmp->len;
            strcpy(msg,tmp->message);
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
        if(sem != NULL) { //check if semaphore is null
            sem->count = 0;
        }
        else {
            printf("semaphore doesn't exist!\n");
        }
    }

    /*
    When a thread waits on a blocking receive, its 
    TCB is queued on the queue of the semaphore (i.e., br_sem) 
    that is part of the TCB
    */
   /*Any thread that calls blocking send will be placed in br_sem of
   the receiver's tcb data structure, which means receive must 
   check if the sender of the newest message is at the top of the br_sem queue,
   and if it is then it needs to call sem_signal on br_sem
   */ 
    void receive(int *tid, char *msg, int *len) {
        while(1) { //will go until forcibly returned out
            messageNode *ptr = running->msg;
            messageNode *prev = NULL;
            while(ptr != NULL) {
                if(*tid == 0 || ptr->sender == *tid) {//message found
                    sem_t *blockingSem = running->br_sem;
                    if(blockingSem->q != NULL) { //check if something is at the top of current thread's blocking queue
                        if(blockingSem->q->thread_id == ptr->sender) { //if this message's sender is top of br_sem q, the sender used a blocking send
                            sem_signal(running->br_sem);
                        }
                    }
                    *tid = ptr->sender;
                    strcpy(msg, ptr->message);
                    *len = ptr->len;
                    if(prev == NULL) {
                        running->msg = running->msg->next;
                    }
                    else{
                        prev->next = prev->next->next;
                    }
                    free(ptr->message);
                    free(ptr);
                    return;
                }
                prev = ptr;
                ptr = ptr->next;
            }
            setToZero(running->br_sem);
            sem_wait(running->br_sem);
            if(shutdownFlag == 1) {
                return;
            }
        }
    }

    /* Send a message and wait for reception. The same as send(), 
    except that the caller does not return until the destination 
    thread has received the message. */
    void block_send(int tid, char *msg, int length) {
        send(tid,msg,length);
        struct tcb *thread = getThread(tid);
        setToZero(thread->br_sem);
        sem_wait(thread->br_sem);
    }

    /* Wait for and receive a message; the same as receive(), 
    except that the caller (a receiver) also needs to specify the 
    sender. */
    void block_receive(int *tid, char *msg, int *length) {
        if(*tid == 0) {
            printf("blocking receive: must specify a sender!\n");
            return;
        }
        receive(tid,msg,length);
    }