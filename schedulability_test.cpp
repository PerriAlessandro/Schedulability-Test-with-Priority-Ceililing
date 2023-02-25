//ASSIGNMENT RTOS - PERRI ALESSANDRO n. 4476726

// Starting from the exercise with 3 tasks scheduled with Rate Monotonic
// - Create an application with 4 periodic tasks, with period 80ms, 100ms, 200ms, 160ms (the task with the highest priority is called Task1, the one with the lowest priority Task4)
// - Create 3 global variables called T1T2, T1T4, T2T3.
// - Task1  shall write something into T1T2, Task 2 shall read from it.
// - Task1  shall write something into T1T4, Task 4 shall read from it.
// - Task2  shall write something into T2T3, Task 3 shall read from it.
// -All critical sections shall be protected by semaphores
// - Semaphores shall use Priority Ceiling
//compile with: g++ -lpthread <sourcename> -o <executablename>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/types.h>

//Colors
#define RESET "\033[0m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHBLU "\e[1;94m"
#define BHCYN "\e[1;96m"

//funcion to find the largest number in the given array
double largest(double arr[], int n);

//function used inside each code just for wasting time
void waste_time();
//code of periodic tasks
void task1_code();
void task2_code();
void task3_code();
void task4_code();
//code of aperiodic tasks (if any)

//characteristic function of the thread, only for timing and synchronization
//periodic tasks
void *task1(void *);
void *task2(void *);
void *task3(void *);
void *task4(void *);

//Declare mutexes
pthread_mutex_t mutex12;
pthread_mutex_t mutex14;
pthread_mutex_t mutex23;

#define INNERLOOP 100
#define OUTERLOOP 2000

#define NPERIODICTASKS 4
#define NAPERIODICTASKS 0
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS
//number of executions for each task
#define NEXEC 100

long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];

//values on which the tasks will write and read
float T1T2, T1T4, T2T3 = 0;

//Critical Sections z_ij where (i-th task, j-th critical section of that task)
double z1[2];
double z2[2];
double z3[1];
double z4[1];

int main()
{
  //inizialization of pthread mutex attributes
  pthread_mutexattr_t mymutexattr;
  pthread_mutexattr_init(&mymutexattr);
  //setting the protocol of mutexes to Priority Ceiling
  pthread_mutexattr_setprotocol(&mymutexattr, PTHREAD_PRIO_PROTECT);

  // set task periods in nanoseconds
  //the first task has period 80 millisecond
  //the second task has period 120 millisecond
  //the third task has period 160 millisecond
  //the fourth task has period 200 millisecond
  periods[0] = 80000000;  //in nanoseconds
  periods[1] = 100000000; //in nanoseconds
  periods[2] = 160000000; //in nanoseconds
  periods[3] = 200000000; //in nanoseconds

  //this is not strictly necessary, but it is convenient to
  //assign a name to the maximum and the minimum priotity in the
  //system. We call them priomin and priomax.
  struct sched_param priomax;
  priomax.sched_priority = sched_get_priority_max(SCHED_FIFO);
  struct sched_param priomin;
  priomin.sched_priority = sched_get_priority_min(SCHED_FIFO);

  // set the maximum priority to the current thread (you are required to be
  // superuser). Check that the main thread is executed with superuser privileges
  // before doing anything else.

  if (getuid() == 0)
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomax);

  // execute all tasks in standalone modality in order to measure execution times
  // (use gettimeofday). Use the computed values to update the worst case execution
  // time of each task.

  int i;
  for (i = 0; i < NTASKS; i++)
  {

    // initialize time_1 and time_2 required to read the clock
    struct timespec time_1, time_2;
    WCET[i] = 0;

    //we should execute each task more than one for computing the WCET, for example 100 times

    for (int j = 0; j < 100; j++)
    {

      clock_gettime(CLOCK_REALTIME, &time_1);
      if (i == 0)
        task1_code();
      if (i == 1)
        task2_code();
      if (i == 2)
        task3_code();
      if (i == 3)
        task4_code();

      clock_gettime(CLOCK_REALTIME, &time_2);
      // compute the Worst Case Execution Time
      float currentIteration = 1000000000 * (time_2.tv_sec - time_1.tv_sec) + (time_2.tv_nsec - time_1.tv_nsec);
      if (currentIteration > WCET[i])
        WCET[i] = currentIteration;
    }
    printf("\nWorst Case Execution Time %d=%f [ns] \n\n", i, WCET[i]);
  }

  //beta_i* is defined as the set of all critical sections that can block the i-th task
  double beta1_star[] = {z2[0], z4[0]};
  double beta2_star[] = {z3[0], z4[0]};
  double beta3_star[] = {z4[0]};
  double beta4_star[] = {};

  //Compute the maximum blocking time for each task
  double B[NTASKS];
  B[0] = largest(beta1_star, sizeof(beta1_star) / sizeof(beta1_star[0]));
  B[1] = largest(beta2_star, sizeof(beta2_star) / sizeof(beta2_star[0]));
  B[2] = largest(beta3_star, sizeof(beta3_star) / sizeof(beta3_star[0]));
  B[3] = 0;

  printf(BHRED "J1" RESET ": z11= %.0lf [ns], z12= %.0lf [ns], B1= %.0lf [ns]\n", z1[0], z1[1], B[0]);
  printf(BHGRN "J2" RESET ": z21= %.0lf [ns], z22= %.0lf [ns], B2= %.0lf [ns]\n", z2[0], z2[1], B[1]);
  printf(BHBLU "J3" RESET ": z31= %.0lf [ns], B3= %.0lf [ns]\n", z3[0], B[2]);
  printf(BHCYN "J4" RESET ": z41= %.0lf [ns], B4= %.0lf [ns]\n\n", z4[0], B[3]);

  double Ulub;
  //Utilization factor considering shared resources with Priority Ceiling
  double U[NTASKS];
  for (int i = 0; i < NTASKS; i++)
  {
    U[i] = (B[i] / periods[i]);
    Ulub = (i + 1.0) * (pow(2.0, (1.0 / (i + 1.0))) - 1.0);

    for (int j = 0; j <= i; j++)
    {
      U[i] += (WCET[j] / periods[j]);
    }

    //check the sufficient conditions: if they are not satisfied, exit
    if (U[i] > Ulub)
    {
      printf("\n U=%lf > Ulub=%lf Sufficient condition for schedulability not satisfied!\n", U[i], Ulub);
      return (-1);
    }
    printf("U(%d)=%lf<%lf=Ulub(%d)\n", i, U[i], Ulub, i);
  }

  printf("\n --> U=%lf<Ulub=%lf Schedulable Task Set\n", U[3], Ulub);
  fflush(stdout);
  sleep(2);

  // set the minimum priority to the current thread: this is now required because
  //we will assign higher priorities to periodic threads to be soon created
  //pthread_setschedparam

  if (getuid() == 0)
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomin);

  // set the attributes of each task, including scheduling policy and priority
  for (i = 0; i < NPERIODICTASKS; i++)
  {
    //initializa the attribute structure of task i
    pthread_attr_init(&(attributes[i]));

    //set the attributes to tell the kernel that the priorities and policies are explicitly chosen,
    //not inherited from the main thread (pthread_attr_setinheritsched)
    pthread_attr_setinheritsched(&(attributes[i]), PTHREAD_EXPLICIT_SCHED);

    // set the attributes to set the SCHED_FIFO policy (pthread_attr_setschedpolicy)
    pthread_attr_setschedpolicy(&(attributes[i]), SCHED_FIFO);

    //properly set the parameters to assign the priority inversely proportional
    //to the period
    parameters[i].sched_priority = priomin.sched_priority + NTASKS - i;

    //set the attributes and the parameters of the current thread (pthread_attr_setschedparam)
    pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
  }

  //setting the priority of mutexes according to Priority Ceiling protocol:
  //First semaphore protects z_11 and z_21 --> takes the priority of task 1
  pthread_mutexattr_setprioceiling(&mymutexattr, parameters[0].sched_priority);
  pthread_mutex_init(&mutex12, &mymutexattr);

  //Second semaphore protects z_12 and z_41 --> takes the priority of task 1
  pthread_mutexattr_setprioceiling(&mymutexattr, parameters[0].sched_priority);
  pthread_mutex_init(&mutex14, &mymutexattr);

  //Third semaphore protects z_22 and z_31 --> takes the priority of task 2
  pthread_mutexattr_setprioceiling(&mymutexattr, parameters[1].sched_priority);
  pthread_mutex_init(&mutex23, &mymutexattr);

  //declare the variable to contain the return values of pthread_create
  int iret[NTASKS];

  //declare variables to read the current time
  struct timespec time_1;
  clock_gettime(CLOCK_REALTIME, &time_1);

  // set the next arrival time for each task. This is not the beginning of the first
  // period, but the end of the first period and beginning of the next one.
  for (i = 0; i < NPERIODICTASKS; i++)
  {
    long int next_arrival_nanoseconds = time_1.tv_nsec + periods[i];
    //then we compute the end of the first period and beginning of the next one
    next_arrival_time[i].tv_nsec = next_arrival_nanoseconds % 1000000000;
    next_arrival_time[i].tv_sec = time_1.tv_sec + next_arrival_nanoseconds / 1000000000;
    missed_deadlines[i] = 0;
  }

  // create all threads(pthread_create)
  iret[0] = pthread_create(&(thread_id[0]), &(attributes[0]), task1, NULL);
  iret[1] = pthread_create(&(thread_id[1]), &(attributes[1]), task2, NULL);
  iret[2] = pthread_create(&(thread_id[2]), &(attributes[2]), task3, NULL);
  iret[3] = pthread_create(&(thread_id[3]), &(attributes[3]), task4, NULL);

  // join all threads (pthread_join)
  pthread_join(thread_id[0], NULL);
  pthread_join(thread_id[1], NULL);
  pthread_join(thread_id[2], NULL);
  pthread_join(thread_id[3], NULL);
  // set the next arrival time for each task. This is not the beginning of the first
  // period, but the end of the first period and beginning of the next one.
  for (i = 0; i < NTASKS; i++)
  {
    printf("\nMissed Deadlines Task %d=%d\n", i, missed_deadlines[i]);
    fflush(stdout);
  }

  //destroy the mutex attributes object
  pthread_mutexattr_destroy(&mymutexattr);
  exit(0);
}

// application specific task_1 code
void task1_code()
{
  //print the id of the current task
  printf(BHRED "1[" RESET);
  fflush(stdout);

  struct timespec start, finish;
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex12
  pthread_mutex_lock(&mutex12);
  //wasting time
  waste_time();
  //write something in T1T2
  T1T2 = rand() % 100;
  //printf("\nT1 writes something on T1T2:" BHRED " %f" RESET "\n", T1T2);
  //Unlock mutex12
  pthread_mutex_unlock(&mutex12);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_11, e.g. the first critical section of the first task
  z1[0] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex14
  pthread_mutex_lock(&mutex14);
  waste_time();
  //write something in T1T4
  T1T4 = rand() % 100;
  //printf("\nT1 writes something on T1T4:" BHBLU " %f" RESET "", T1T4);
  //Unlock mutex14
  pthread_mutex_unlock(&mutex14);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_12, e.g. the second critical section of the first task
  z1[1] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  printf(BHRED "]1" RESET);
  fflush(stdout);
}

//thread code for task_1 (used only for temporization)
void *task1(void *ptr)
{
  // set thread affinity, that is the processor on which threads shall run
  cpu_set_t cset;
  CPU_ZERO(&cset);
  CPU_SET(0, &cset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  //execute the task NEXEC times (e.g.one hundred times by default), it should be an infinite loop (too dangerous)
  int i = 0;
  for (i = 0; i < NEXEC; i++)
  {
    // execute application specific code
    task1_code();

    //Checking if a deadline has been missed
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double currentTime = 1000000000 * (now.tv_sec) + now.tv_nsec;
    double prevNextArrTime = 1000000000 * (next_arrival_time[0].tv_sec) + next_arrival_time[0].tv_nsec;
    //if the current time is greater than the Next Arrival Time of the previous iteration, then there's been a missed deadline
    if (currentTime > prevNextArrTime)
    {
      missed_deadlines[0] += 1;
    }

    // sleep until the end of the current period (which is also the start of the
    // new one
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);

    // the thread is ready and can compute the end of the current period for
    // the next iteration
    long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
    next_arrival_time[0].tv_nsec = next_arrival_nanoseconds % 1000000000;
    next_arrival_time[0].tv_sec = next_arrival_time[0].tv_sec + next_arrival_nanoseconds / 1000000000;
  }
}

void task2_code()
{

  //print the id of the current task
  printf(BHGRN "2[" RESET);
  fflush(stdout);

  float reading_T1T2;
  struct timespec start, finish;
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex12
  pthread_mutex_lock(&mutex12);
  waste_time();
  //reading the content of T1T2
  reading_T1T2 = T1T2;
  // printf("\nT2 reads something on T1T2:" BHRED " %f" RESET "", T1T2);
  // Unlock mutex12
  pthread_mutex_unlock(&mutex12);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_21, e.g. the first critical section of the second task
  z2[0] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex23
  pthread_mutex_lock(&mutex23);
  waste_time();
  //write something in T2T3
  T2T3 = rand() % 100;
  // printf("\nT2 writes something on T2T3: " BHGRN "%f" RESET "", T2T3);
  //Unlock mutex23
  pthread_mutex_unlock(&mutex23);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_22, e.g. the second critical section of the second task
  z2[1] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  //print the id of the current task
  printf(BHGRN "]2" RESET);
  fflush(stdout);
}

void *task2(void *ptr)
{
  // set thread affinity, that is the processor on which threads shall run
  cpu_set_t cset;
  CPU_ZERO(&cset);
  CPU_SET(0, &cset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  int i = 0;
  for (i = 0; i < NEXEC; i++)
  {
    task2_code();

    //Checking if a deadline has been missed
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double currentTime = 1000000000 * (now.tv_sec) + now.tv_nsec;
    double prevNextArrTime = 1000000000 * (next_arrival_time[1].tv_sec) + next_arrival_time[1].tv_nsec;
    //if the current time is greater than the Next Arrival Time of the previous iteration, then there's been a missed deadline
    if (currentTime > prevNextArrTime)
    {
      missed_deadlines[1] += 1;
    }

    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);
    long int next_arrival_nanoseconds = next_arrival_time[1].tv_nsec + periods[1];
    next_arrival_time[1].tv_nsec = next_arrival_nanoseconds % 1000000000;
    next_arrival_time[1].tv_sec = next_arrival_time[1].tv_sec + next_arrival_nanoseconds / 1000000000;
  }
}

void task3_code()
{
  //print the id of the current task
  printf(BHBLU "3[" RESET);
  fflush(stdout);
  float reading_T2T3;
  struct timespec start, finish;
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex23
  pthread_mutex_lock(&mutex23);
  waste_time();
  //reading the content of T2T3
  reading_T2T3 = T2T3; //printf("\nT3 reads something on T2T3: " BHGRN "%f" RESET "", T2T3);
  //Unlock mutex23
  pthread_mutex_unlock(&mutex23);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_31, e.g. the first critical section of the third task
  z3[0] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  //print the id of the current task
  printf(BHBLU "]3" RESET);
  fflush(stdout);
}

void *task3(void *ptr)
{
  // set thread affinity, that is the processor on which threads shall run
  cpu_set_t cset;
  CPU_ZERO(&cset);
  CPU_SET(0, &cset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  int i = 0;
  for (i = 0; i < NEXEC; i++)
  {
    task3_code();

    //Checking if a deadline has been missed
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double currentTime = 1000000000 * (now.tv_sec) + now.tv_nsec;
    double prevNextArrTime = 1000000000 * (next_arrival_time[2].tv_sec) + next_arrival_time[2].tv_nsec;
    //if the current time is greater than the Next Arrival Time of the previous iteration, then there's been a missed deadline
    if (currentTime > prevNextArrTime)
    {
      missed_deadlines[2] += 1;
    }

    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
    long int next_arrival_nanoseconds = next_arrival_time[2].tv_nsec + periods[2];
    next_arrival_time[2].tv_nsec = next_arrival_nanoseconds % 1000000000;
    next_arrival_time[2].tv_sec = next_arrival_time[2].tv_sec + next_arrival_nanoseconds / 1000000000;
  }
}

void task4_code()
{
  //print the id of the current task
  printf(BHCYN "4[" RESET);
  fflush(stdout);
  float reading_T1T4;
  struct timespec start, finish;
  //get the current time (before semaphore)
  clock_gettime(CLOCK_REALTIME, &start);
  // Lock mutex14
  pthread_mutex_lock(&mutex14);
  waste_time();
  //reading the content of T1T4
  reading_T1T4 = T1T4; //printf("\nT4 reads something on T1T4:" BHBLU " %f" RESET "", T1T4);

  //Unlock mutex14
  pthread_mutex_unlock(&mutex14);
  //get the current time (after semaphore)
  clock_gettime(CLOCK_REALTIME, &finish);
  //Compute z_41, e.g. the first critical section of the fourth task
  z4[0] = 1000000000 * (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
  //print the id of the current task
  printf(BHCYN "]4" RESET);
  fflush(stdout);
}
void *task4(void *ptr)
{
  // set thread affinity, that is the processor on which threads shall run
  cpu_set_t cset;
  CPU_ZERO(&cset);
  CPU_SET(0, &cset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  int i = 0;
  for (i = 0; i < NEXEC; i++)
  {
    task4_code();

    //Checking if a deadline has been missed
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    double currentTime = 1000000000 * (now.tv_sec) + now.tv_nsec;
    double prevNextArrTime = 1000000000 * (next_arrival_time[3].tv_sec) + next_arrival_time[3].tv_nsec;
    //if the current time is greater than the Next Arrival Time of the previous iteration, then there's been a missed deadline
    if (currentTime > prevNextArrTime)
    {
      missed_deadlines[3] += 1;
    }

    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[3], NULL);
    long int next_arrival_nanoseconds = next_arrival_time[3].tv_nsec + periods[3];
    next_arrival_time[3].tv_nsec = next_arrival_nanoseconds % 1000000000;
    next_arrival_time[3].tv_sec = next_arrival_time[3].tv_sec + next_arrival_nanoseconds / 1000000000;
  }
}

//function to waste time
void waste_time()
{
  double uno;
  for (int i = 0; i < OUTERLOOP; i++)
  {
    for (int j = 0; j < INNERLOOP; j++)
    {

      uno = rand() * rand();
    }
  }
}

//function to find the largest number in the given array
double largest(double arr[], int n)
{
  int i;

  // Initialize maximum element
  double max = arr[0];
  for (i = 1; i < n; i++)
    if (arr[i] > max)
      max = arr[i];

  return max;
}

///////////////////////////////////// CONCLUSION ////////////////////////////////////////////////////
// In each task code, I used mutexes to protect the critical sections, and inside each of them,
// I added the waste_time() function to increase the probability of being pre-empted by another task
// that uses that semaphore. The utilization factor is strictly dependent on the way you compute the
// Worst-Case Execution Time (WCET) for each task, so I computed it by executing each task 100 times,
// to have a more reliable value (NOTE: this doesn't mean that it is reliable in absolute terms, it
// just means that is MORE reliable than computing it just one time). As a consequence, U becomes
// generally way higher than the one computed using a less reliable WCET, so when the task set results
// schedulable (U(i)<Ulub(i), i=1,2,.., NTASKS) the number of missed deadlines are lower (it should tend to 0).
// Notice also that when we calculate utilization factor using semaphore with Priority Ceiling protocol,
// we must consider the maximum blocking time for each task (i.e. B[NTASKS] array), where B[i] is defined
// as the longest critical section belonging to the set 'beta_i_star', i=1,2...NTASKS.
// In conclusion, it would be nice to try this code in a real Ubuntu operating system, instead of using
// an emulator (such as Virtual Machine) because the last one doesn't permit the implementation a program in
// which task are scheduled with the highest priority as possible. Using a virtual machine severely compromises
//  the execution of code like this and drawing accurate conclusions about it may be tricky.
