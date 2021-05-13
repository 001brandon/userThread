#include "t_lib.h"

struct tcb *running;
struct tcb *ready;

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
  //ucontext_t *tmp, tmpTest1, tmpTest2;
  struct tcb *tmp=malloc(sizeof(struct tcb));
  tmp->thread_context=(ucontext_t *)malloc(sizeof(ucontext_t));
  tmp->next=NULL;
  tmp->thread_id=0;
  tmp->thread_priority=0;
  getcontext(tmp->thread_context);
  running = tmp;
  ready=NULL;
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
}

void t_terminate()
{
    struct tcb *old = running;
    struct tcb *new=ready;
    struct tcb *temp=new;
    if(ready!=NULL){        
        ready=ready->next;
        new->next=NULL;
        running=new;
        //printf("this is the new id: %d\n",new->thread_id);
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

    struct tcb *temp=ready;
    while(ready!=NULL){
        free(temp->thread_context->uc_stack.ss_sp);
        free(temp->thread_context);
        free(temp);
        ready=ready->next;
        temp=ready;
    }
    ready=NULL;
    free(running->thread_context->uc_stack.ss_sp);
    free(running->thread_context);
    free(running);
    running=NULL;
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
    if((*sp)->q==NULL){
        free(*sp);
    } else {
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
    }

    /*
    mbox deposit puts in a new message in the queue
    mbox withdraw takes out that message
    mbox destroy frees space
    */
    void mbox_create(mbox **mb){
        *mb=malloc(sizeof(mbox));
        (*mb)->msg=NULL;
        sem_init(&((*mb)->mbox_sem), 1);
    }

    void mbox_deposit(mbox *mb,char* msg, int len){
        sem_wait(mb->mbox_sem);
        messageNode *new_msg=malloc(sizeof(messageNode));
        new_msg->message=malloc(sizeof(char)*len);
        new_msg->len=len;
        new_msg->sender=running->thread_id;
        strncpy(new_msg->message, msg, len);
        if(mb->msg==NULL){
            mb->msg=new_msg;
        } else {
            messageNode *top = mb->msg;
            while(top->next != NULL){
                top=top->next;
            }
            top->next=new_msg;
            top->next->next=NULL;
        }
        sem_signal(mb->mbox_sem);
        
    }

    void mbox_withdraw(mbox *mb, char* msg, int *len){
        sem_wait(mb->mbox_sem);
        messageNode *tmp=mb->msg;
        if(tmp==NULL){
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