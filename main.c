
/************************************************************
 * Mustafa Algun
 * EE442 Homework 2- User Level Thread Scheduling
 * Due Date: 28 May 2021
-----------------------

SRTF_scheduler implementation reference: https://github.com/Diiru/CPU_Scheduler_SRTF_IIC2333/blob/master/src/osrs/main.c

ucontext implementation reference: https://github.com/xanpeng/snippets/blob/master/coroutine/ucontext-example.c#L2


 ************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
#include "main.h"


#define READY    0
#define RUNNING  1
#define FINISHED 2
#define EMPTY    3


bool CancelCurrentThread = false; /*Indicator to cancel current thread*/
int WaitDuration = 1; //1 second wait in between steps
int quantum = 2; //Time quantum for Round Robin Scheduling Algorithm

Thread  MainThread;
Thread  *CurrentThread;

queue QueueReady, QueueFinished; // ready and finished queues are kept here


/*
Makes  context  switching  (using  swapcontext())using  a  preemptive  and weighted fair 
scheduling structure of your choice (for example you can use lottery scheduling) with a switching 
interval of two seconds. A context switch should take place when the associated interrupt comes every two seconds. 
If a thread finishes, its place in the thread array will be marked as empty and the scheduler will free the stack it has used.
*/

void PWF_scheduler () { // Round Robin Scheduling Algorithm is used for this

  Thread  *prev , *next = NULL;
  alarm(0);
  if ( getcontext(&MainThread.ucont ) == -1 ) {
    printf("Error while getting context\n");
    exit(1);
  }
  prev = CurrentThread;

  if (!CancelCurrentThread) {
    threadArray[(prev->id)+1].state = READY;
    enqueue(&QueueReady, prev);
  } else {
    CancelCurrentThread = false;
    threadArray[(prev->id)+1].state = FINISHED;
    exitThread((prev->id)+1);
  }

  next = dequeue(&QueueReady);
  if (next == NULL) {
    printf("No thread left\n"); //  no thread left in the ready queue
    exit(1);
  } 
  CurrentThread = next;
  alarm(quantum);

  threadArray[(next->id)+1].state = RUNNING;

  bool goIn = false;
    
    for(int i=0;i<6;i++) {
      if(threadArray[i+1].state == RUNNING ) {
        printf("running>T%d\t", i+1);
        goIn = true;
      }
    }
    if(goIn){
    goIn = false;
    printf("ready>");
    for(int i=0;i<5;i++) {
      if(threadArray[i+1].state == READY) printf("T%d,", i+1);
    }
    printf("\t");

    printf("finished>");
      for(int i=0;i<5;i++) {
        if(threadArray[i+1].state == FINISHED) printf("T%d,", i+1);
      }
    printf("\n");
    }


      
  
  if ( swapcontext(&(prev->ucont ), &(next->ucont )) == -1 ) {
    printf("Error\n"); //next thread call failed
  }
}


void SRTF_scheduler () {

    // Extract all total remaining times ----------------------------------
    int array_total_remainings[queue_size(&QueueReady)];
    for (int idx = 0; idx < queue_size(&QueueReady); idx++)
    {
      node* checking_node = get_node(idx, &QueueReady);
      array_total_remainings[idx] = checking_node->data->total_remaining_time;
    }
    // -----------------------------------------------------------------------
    // Min value in the array
    int min_remaining_time;
    min_remaining_time = minimun_in_array(array_total_remainings, queue_size(&QueueReady));
    // -----------------------------------------------------------------------
    // Check if there is repetition
    int repeated = 0;
    for (int _idx = 0; _idx < QueueReady.count; _idx++)
    {
      if (array_total_remainings[_idx] == min_remaining_time)
      {
        repeated += 1;
      }
    }

    if (repeated == 1)
    {
      for (int idx = 0; idx < QueueReady.count; idx++)
      {
        node* checking_node = get_node(idx, &QueueReady);
        if (checking_node->data->total_remaining_time == min_remaining_time)
        {
          exit(1);
        }
      }
    }
    else if (repeated > 1)
    {
      // Tie between processes, we must choose by current CPU burst
      int tie_breaker[repeated];
      int current_index = 0;
      for (int idx = 0; idx < QueueReady.count; idx++)
      {
        node* checking_node = get_node(idx, &QueueReady);
        if (checking_node -> data -> total_remaining_time == min_remaining_time)
        {
          tie_breaker[current_index] = checking_node -> data -> burst_sequence[checking_node -> data -> id];
          current_index += 1;
        }
      }
      // We extracted all the CPU bursts from the tied processes.
      // Now, we calculate the min in the array, and extract the process.
      int min_current_cpu_burst = minimun_in_array(tie_breaker, repeated);

      // Now we have the info of the chosen one
      for (int idx = 0; idx < QueueReady.count; idx++)
      {
        node* checking_node = get_node(idx, &QueueReady);
        if (checking_node -> data -> total_remaining_time == min_remaining_time
          && checking_node -> data -> burst_sequence[checking_node -> data -> id] == min_current_cpu_burst)
        {
          exit(1);
        }
      }
    }
  exit(1);
}




/* 
Each thread is required to execute a simple counter function 
that takes two arguments, “n” and “i”, n being the stopping criteria 
for counting and i being the thread number. The function counts up starting 
from zero up to “n”. With each increment, the function prints the count
value having “i” tabs on its left.After each print, the function sleeps for 1 seconds. 
*/

void counter(int n, int i) {

  for (int j=0; j<n; j++) {
    for (int j=0; j<i; j++)
      printf("\t");
    printf("%d\n",j+1);
    sleep(WaitDuration);
  }
  CancelCurrentThread = true;
}


/*
Creates a new thread. If the system is unable to create the new thread, it will return -1 and print out an error message.
This function will be used for setting up a user context and
associated stack. A newly created thread will be marked as READY when it is inserted into the system.
*/

int createThread(Thread  *thread, int n, int i) {

  thread->id = queue_size(&QueueReady);
  if ( getcontext(&(thread->ucont )) == -1 ) {
    printf("Context retrieval failed...\n");
    exit(1);
  }
  /*Modifying context to point to new stack */
  thread->ucont .uc_link = &MainThread.ucont ;
  thread->ucont .uc_stack.ss_sp = malloc(SIGSTKSZ);
  thread->ucont .uc_stack.ss_size = SIGSTKSZ;
  /*Creating the context */
  makecontext(&(thread->ucont ), (void (*) ()) counter, 2, n, i);
  /*Queue the thread for execution*/
  enqueue(&QueueReady, thread);
  threadArray[i+1].state = READY;
  threadArray[i+1].context = &(thread->ucont );
  return 0 ; /* for now */

}


/*
Switches control from the main thread to one of the threads in the
thread array, which also activates the timer that triggers context switches.
*/

void runThread() {



   Thread  *prev , *next = NULL;
   alarm(0);
  if ( getcontext(&MainThread.ucont ) == -1 ) {
    printf("Context retrieval failed...\n");
    exit(1);
  }
   prev = CurrentThread;

   if (!CancelCurrentThread) {
    threadArray[(prev->id)+1].state = READY;
     enqueue(&QueueReady, prev);
   } else {
     CancelCurrentThread = false;
    threadArray[(prev->id)+1].state = FINISHED;
    exitThread((prev->id)+1);
   }

   next = dequeue(&QueueReady);
   if (next == NULL) {
    printf("No thread present in ready queue\n");
    exit(1);
  } 
   CurrentThread = next;
   alarm(quantum);

  threadArray[(next->id)+1].state = RUNNING;

   if ( swapcontext(&(prev->ucont ), &(next->ucont )) == -1 ) {
    printf("Error\n"); //next thread call failed
  }

}


void initializeThread() { // Initializes all global data structures for the thread.

  queue_init(&QueueReady);
  queue_init(&QueueFinished);

  // Singal interrupt configuration
  sigemptyset(&sa.sa_mask);

  // choose one scheduler
  // 1.SRTF_scheduler
  // 2.PWF_scheduler

  sa.sa_handler = PWF_scheduler ; 

  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);

  alarm(quantum); // timer in every 1 second

  MainThread.id = -1;

  if ( getcontext(&(MainThread.ucont )) == -1) {
    printf("Context retrieval failed...\n");
    exit(1);
  }
  CurrentThread = &MainThread; // Declear MainThread as CurrentThread

  threadArray[0].context = &(MainThread.ucont);
  threadArray[0].state = RUNNING;
}

void exitThread(int i) {  // Removes the thread from the thread array (i.e. the thread does notneed to run anymore)

    threadArray[i].state = EMPTY;
    free( threadArray[i].context->uc_stack.ss_sp);
}




/**********************************************************
 * Main Routine
 **********************************************************/

int main(int argc, char** argv) {

Thread * threadPointer;

  if (argc <=1) {
   printf("Invalid input\n");
   exit(1);
  }
  int isThreadNumber = argc-1;
  if (isThreadNumber > 5) {
    printf("Decrease the number of threads to less than five(5), please!\n");
    exit (1);
  }

  initializeThread();

  for(int i=1; i<=5; i++) {
      threadArray[i].state = EMPTY;
  }

  printf("Threads:\n");

  for(int i=1; i<=isThreadNumber; i++) {
    printf("T");
    printf("%d",i);
    printf("\t");
  }

  printf("\n");

  for (int i=0; i<isThreadNumber; i++) {
    threadPointer = malloc(isThreadNumber*sizeof(Thread));
    createThread (threadPointer, atoi(argv[i+1]), i);
  }

  while(1); // without this it will not be able to wait for new thread creation

}