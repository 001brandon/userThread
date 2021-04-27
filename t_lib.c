#include "t_lib.h"

ucontext_t *running;
ucontext_t *ready;

void t_yield()
{
  ucontext_t *tmp;

  tmp = running;
  running = ready;
  ready = tmp;

  swapcontext(ready, running);
}

void t_init()
{
  ucontext_t *tmp, tmpTest1, tmpTest2;
  int mainPriority = 1;
  int mainId = 1;
  struct tcb *main = (struct tcb *) malloc(sizeof(struct tcb));
  struct tcb *test1 = (struct tcb *) malloc(sizeof(struct tcb));
  struct tcb *test2 = (struct tcb *) malloc(sizeof(struct tcb));
  
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
  getcontext(tmp);    /* let tmp be the context of main() */
  running = tmp;
  main->thread_id = mainId;
  main->thread_priority = mainPriority;
  main->thread_context = tmp;
  test1->thread_id = 6;
  test1->thread_priority = 1;
  test1->thread_context = tmp;
  test2->thread_id = 4;
  test2->thread_priority = mainPriority;
  test2->thread_context = tmp;
  qInsert(*main);
  qInsert(*test1);
  qInsert(*test2);
  qdisplay(qhead);
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
  uc->uc_link = running; 
  makecontext(uc, (void (*)(void)) fct, 1, id);
  ready = uc;
}

void t_terminate()
{
}

void t_shutdown()
{
}
















 
 
void append(struct tcb curr_tcb)
{
    struct tcb *temp,*right;
    temp= (struct tcb *)malloc(sizeof(struct tcb));
    temp=&curr_tcb;
    right=(struct tcb *)head;
    while(right->next != NULL)
    right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}
 
 
 
void add(struct tcb curr_tcb)
{
    struct tcb *temp;
    temp=(struct tcb *)malloc(sizeof(struct tcb));
    temp=&curr_tcb;
    if (head== NULL)
    {
    head=temp;
    head->next=NULL;
    }
    else
    {
    temp->next=head;
    head=temp;
    }
}

void addafter(struct tcb curr_tcb, int loc)
{
    int i;
    struct tcb *temp,*left,*right;
    right=head;
    for(i=1;i<loc;i++)
    {
    left=right;
    right=right->next;
    }
    temp=(struct tcb *)malloc(sizeof(struct tcb));
    temp=&curr_tcb;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}
 
 
 
void insert(struct tcb curr_tcb)
{
    int c=0;
    struct tcb *temp;
    temp=head;
    if(temp==NULL)
    {
    add(curr_tcb);
    }
    else
    {
    while(temp!=NULL)
    {
        if(temp->thread_id < curr_tcb.thread_id)
        c++;
        temp=temp->next;
    }
    if(c==0)
        add(curr_tcb);
    else if(c<count())
        addafter(curr_tcb,++c);
    else
        append(curr_tcb);
    }
}
 
 
 
int delete(int num)
{
    struct tcb **indirect=&head;
    struct tcb *temp=NULL;
    while(*indirect!=NULL){
        if((*indirect)->thread_id==num){
            temp=*indirect;
            *indirect=(*indirect)->next;
            free(temp);
        } else{
            indirect=&((*indirect)->next);
        }
    }

    return 0;
}
 
 
void  display(struct tcb *r)
{
    r=head;
    if(r==NULL)
    {
    return;
    }
    while(r!=NULL)
    {
    printf("%d %d",r->thread_id, r->thread_priority);
    r=r->next;
    }
    printf("\n");
}
 
 
int count()
{
    struct tcb *n;
    int c=0;
    n=head;
    while(n!=NULL)
    {
    n=n->next;
    c++;
    }
    return c;
}

void freeall(){
    struct tcb *temp=head;
    while(head!=NULL){
        temp=head;
        head=head->next;
        free(temp);
    }
    qfreeall();
}

void qfreeall(){
    struct tcb *temp=qhead;
    while(qhead!=NULL){
        temp=qhead;
        qhead=qhead->next;
        free(temp);
    }
}

struct tcb* qRemove(void) { //returns pointer to tcb, removes the tcb from ready queue
  struct tcb* returnTcb;
  returnTcb = (struct tcb *)malloc(sizeof(struct tcb));
  returnTcb->thread_id = qhead->thread_id;
  returnTcb->thread_priority = qhead->thread_priority;
  returnTcb->thread_context = qhead->thread_context;
  returnTcb->next = NULL;
  struct tcb *temp = qhead;
  qhead = qhead->next;
  free(temp);
  return returnTcb;

}
 
void qInsert(struct tcb curr_tcb){
  struct tcb *right;
  right=(struct tcb *)qhead;
  if(right==NULL){
    qhead=&curr_tcb;
  } else {
    struct tcb *temp;
    temp= (struct tcb *)malloc(sizeof(struct tcb));
    temp=&curr_tcb;
    right=(struct tcb *)qhead;
    while(right->next != NULL)
    right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
  }
}


void  qdisplay(struct tcb *r)
{
    r=qhead;
    if(r==NULL)
    {
    return;
    }
    while(r!=NULL)
    {
    printf("%d %d",r->thread_id, r->thread_priority);
    r=r->next;
    }
    printf("\n");
}