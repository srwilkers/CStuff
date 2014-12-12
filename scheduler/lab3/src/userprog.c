#include "lab2-api.h"

//----------------------------------------------------------------------------
//   You are free to modify this file (and other userprogs) to test your solution
//   Understand the scheduling algorithm well before you hack or you'll get nowhere. 
//----------------------------------------------------------------------------
main (int argc, char *argv[])
{
  int number, i,j,offset;
  uint32 handle;
  uint32 start,  finish, total;
  sem_t semaphore;
  char num_str[10], semaphore_str[10];

  start = TimerGet();
  switch(argc)
  {
    case 2:
      number = dstrtol(argv[1], NULL, 10);
      Printf("Setting number = %d\n", number);
      semaphore = sem_create(1);
      ditoa(semaphore, semaphore_str);	//Convert the semaphore to a string
        
      for(i=0; i<number; i++)
      {
        ditoa(i, num_str);
        process_create(i,0,"userprog.dlx.obj",semaphore_str, num_str,NULL);
      }
      break;
    case 3:
      offset = dstrtol(argv[1], NULL, 10);       //Get semaphore
      semaphore = dstrtol(argv[2], NULL, 10);       //Get semaphore
      for(i=0;i<100;i++)
      {
        sem_wait(semaphore);
        for(j=0;j<50000;j++);
        Printf("%c%d\n",'A'+offset,i);
        sem_signal(semaphore);
      }

      break;
    default:
      Printf("Usage: ");
      Printf(argv[0]);
      Printf(" number\n");
      Printf("argc = %d\n", argc);
      
      exit();
  }
  finish = TimerGet();
  total = finish - start;
  Printf("Total time ran was %d\n", total);


}
