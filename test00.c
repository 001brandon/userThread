#include "ud_thread.h"
#include <stdio.h>

int id_num = 0;
  sem_t *s;

void assignCubed(int pri)
{
  int i;
  for (i = 0; i < 3; i++)
    printf("in assignCubed(1): %d with id\n", i);
  t_yield();

  for (i = 20; i < 23; i++)
    printf("in assignCubed(2): %d with id\n", i);
  t_yield();

  t_terminate();
}

void assignSquared(int pri)
{
  int i;
  for (i = 0; i < 3; i++)
    printf("in assignSquared(1): %d with id\n", i);
  t_yield();

  id_num++;
  t_create(assignCubed,id_num,1);

  for (i = 20; i < 23; i++)
    printf("in assignSquared(2): %d with id\n", i);
  t_yield();

  t_terminate();
}

void assign(int pri)
{
  int i;

  for (i = 0; i < 3; i++)
    printf("in assign(1): %d with id\n", i);

  id_num++;
  t_create(assignSquared,id_num, 1);
  t_yield();

  for (i = 10; i < 13; i++)
    printf("in assign(2): %d\n", i);

  id_num++;
  t_create(assignSquared,id_num, 1);
  t_yield();

  for (i = 20; i < 23; i++)
    printf("in assign(3): %d\n", i);

  id_num++;
  t_create(assignSquared,id_num, 1);
  t_terminate();
}

void test_sem(){
  printf("Semwait on thread\n");
  sem_wait(s);
  printf("semwait done\n");
  t_terminate();
  
  }

int main(int argc, char **argv) 
{

  sem_init(&s,1);
  t_init();
  id_num++;
  t_create(test_sem,id_num, 1);
  id_num++;
  t_create(test_sem,id_num,1);
  id_num++;
  t_create(test_sem,id_num,1);
  
  

  printf("in main(): 0\n");

  t_yield();

  printf("in main(): 1\n");
  sem_signal(s);
  t_yield();

  printf("in main(): 2\n");
  sem_signal(s);
  t_yield();


  sem_destroy(&s);

  printf("calling premature shutdown, not all threads are done\n");
  t_shutdown();



  return (0);
}

/* --- output -----
in main(): 0
in assign(1): 0
in assign(1): 1
in assign(1): 2
in main(): 1
in assign(2): 10
in assign(2): 11
in assign(2): 12
in main(): 2
in assign(3): 20
in assign(3): 21
in assign(3): 22
done...
*/
