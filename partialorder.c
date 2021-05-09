#include "ud_thread.h"
  sem_t *s1, *s2;

void *worker1(void *arg) {
  sem_wait(s1);
  printf("I am worker 1\n");
  sem_signal(s2);
  sem_signal(s1);
  t_terminate();
}

void *worker2(void *arg) {
  printf("I am worker 2\n");
  sem_signal(s1);
  t_terminate();
}

void *worker3(void *arg) {
  sem_wait(s2);
  sem_wait(s2);
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
    t_init();
    int id_num=1;
    t_init();
    id_num++;
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

}