#include "ud_thread.h"
  sem_t *s1, *s2;

void *child_1(void *arg) {
    printf("child 1: before\n");
    // what goes here?
    sem_signal(s2);
    sem_wait(s1);
    printf("child 1: after\n");
    t_terminate();
}

void *child_2(void *arg) {
    printf("child 2: before\n");
    // what goes here?
    sem_signal(s1);
    sem_wait(s2);
    printf("child 2: after\n");
    t_terminate();
}

void main(){
    sem_init(&s1,0);
    sem_init(&s2,0);
    t_init();
    int id_num=1;
    t_create(child_1,id_num,1);
    id_num++;
    t_create(child_2,id_num,1);
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_yield();
    t_shutdown();
    sem_destroy(&s1);
    sem_destroy(&s2);

}