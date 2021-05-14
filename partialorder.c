#include "ud_thread.h"
  sem_t *s1, *s2, *s3;

void *worker1(void *arg) {
  sem_wait(s1);
  printf("I am worker 1\n");
  sem_signal(s2);
  sem_signal(s1);
  char msg[32] = "ewr";
  send(2,msg,4);
  sem_signal(s3);
  t_terminate();
}

void *worker2(void *arg) {
  printf("I am worker 2\n");
  sem_signal(s1);
  sem_wait(s3);
  sem_wait(s3);
  t_terminate();
}

void *worker3(void *arg) {
  sem_wait(s2);
  sem_wait(s2);
  char msg[32] = "heasdf";
  send(2,msg,4);
  sem_signal(s3);
  printf("I am worker 3\n");
  t_terminate();
}

void *worker4(void *arg) {
  sem_wait(s1);
  printf("I am worker 4\n");
  sem_signal(s2);
  sem_signal(s1);
  t_terminate();
}


void main(){
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
    t_shutdown();
    sem_destroy(&s1);
    sem_destroy(&s2);
    sem_destroy(&s3);

}