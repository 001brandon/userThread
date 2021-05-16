#include "ud_thread.h"
  sem_t *s1, *s2, *s3;

void *worker1(void *arg) {
  sem_wait(s1);
  printf("I am worker 1\n");
  sem_signal(s2);
  sem_signal(s1);
  char msg[32] = "ewr";
  send(2,msg,32);
  sem_signal(s3);
  char reading[35];
  int len;
  int who=0;
  receive(&who,reading,&len);
  printf("asdfasdf by thread %d: %s\n",who,reading);
  t_terminate();
}

void *worker2(void *arg) {
  printf("I am worker 2\n");
  char *msg, *msg2;
  msg = malloc(sizeof(char)*100);
  msg2 = malloc(sizeof(char)*400);
  int len,len2;
  int who=0;
  int who2=0;
  sem_signal(s1);
  //sem_wait(s3);
  //sem_wait(s3);
  receive(&who,msg,&len);
  printf("message by thread %d: %s\n",who,msg);
  printf("meow\n");
  who = 0;
  receive(&who,msg,&len);
  printf("message by thread %d: %s\n",who,msg);
  printf("woof\n");
  who = 0;
  printf("there will be nothing to read here ever\n");
  receive(&who,msg,&len);
  printf("message by thread %d: %s\n",who,msg);
  printf("woof\n");
  free(msg);
  free(msg2);
  t_terminate();
}

void *worker3(void *arg) {
  sem_wait(s2);
  sem_wait(s2);
  char msg[32] = "heaskllj";
  send(2,msg,32);
  sem_signal(s3);
  printf("I am worker 3\n");
  sem_wait(s2);
  t_terminate();
}

void *worker4(void *arg) {
  sem_wait(s1);
  printf("I am worker 4\n");
  sem_signal(s2);
  sem_signal(s1);
  t_terminate();
}


int main(){
    sem_init(&s1,0);
    sem_init(&s2,0);
    sem_init(&s3,0);
    t_init();
    int id_num=1;
    t_create(worker1,id_num, 1);
    id_num++;
    t_create(worker2,id_num,1);
    id_num++;
    t_create(worker3,id_num,1);
    id_num++;
    t_create(worker4,id_num,1);
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield(); //need this many yields to make sure that we can return to the semaphore ready queue before the t_shutdown
    printf("calling shutdown\n");
    sem_destroy(&s1);
    sem_destroy(&s2);
    sem_destroy(&s2);
    sem_destroy(&s2);
    sem_destroy(&s2);
    sem_destroy(&s2);
    sem_destroy(&s3);
    t_shutdown();
    return 0;
}