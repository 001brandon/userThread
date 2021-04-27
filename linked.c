#include<stdio.h>
#include<stdlib.h>
 

 int count();
 


struct tcb {
	int         thread_id;
    int         thread_priority;
    ucontext_t *thread_context;
	struct tcb *next;
} *head;
 
 
void append(int id, int prio, ucontext_t *thread_context)
{
    struct tcb *temp,*right;
    temp= (struct tcb *)malloc(sizeof(struct tcb));
    temp->thread_id=id;
    temp->thread_priority=prio;
    temp->thread_context=thread_context;
    right=(struct tcb *)head;
    while(right->next != NULL)
    right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}
 
 
 
void add(int id, int prio, ucontext_t *thread_context)
{
    struct tcb *temp;
    temp=(struct tcb *)malloc(sizeof(struct tcb));
    temp->thread_id=id;
    temp->thread_priority=prio;
    temp->thread_context=thread_context;
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
void addafter(int id, int prio, ucontext_t *thread_context, int loc)
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
    temp->thread_id=id;
    temp->thread_priority=prio;
    temp->thread_context=thread_context;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}
 
 
 
void insert(struct tcb)
{
    int c=0;
    struct tcb *temp;
    temp=head;
    if(temp==NULL)
    {
    add(id, prio, thread_context);
    }
    else
    {
    while(temp!=NULL)
    {
        if(temp->thread_id < id)
        c++;
        temp=temp->next;
    }
    if(c==0)
        add(id, prio, threa);
    else if(c<count())
        addafter(id, prio, thread_context,++c);
    else
        append(id, prio, thread_context);
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
    printf("%d ",r->thread_id);
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
}
 