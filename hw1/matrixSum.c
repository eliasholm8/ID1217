/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */
#define DEBUG

pthread_mutex_t barrier; /* mutex lock for the barrier */
pthread_cond_t go;       /* condition variable for leaving */
int numWorkers;          /* number of workers */
int numArrived = 0;      /* number who have arrived */

/* a reusable counter barrier */
void Barrier()
{
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers)
  {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  }
  else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer()
{
  static bool initialized = false;
  static struct timeval start;
  struct timeval end;
  if (!initialized)
  {
    gettimeofday(&start, NULL);
    initialized = true;
  }
  gettimeofday(&end, NULL);
  return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time;  /* start and end times */
int size, stripSize;          /* assume size is multiple of numWorkers */
int sums[MAXWORKERS];         /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
int max_values[MAXWORKERS], min_values[MAXWORKERS];
int max_positions[MAXWORKERS], min_positions[MAXWORKERS];

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE)
    size = MAXSIZE;
  if (numWorkers > MAXWORKERS)
    numWorkers = MAXWORKERS;
  else if (numWorkers > size)
    numWorkers = size;
  stripSize = size / numWorkers;

  /* initialize the matrix */
  srand(time(NULL));

  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      matrix[i][j] = rand() % 99;
    }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++)
  {
    printf("%d: [ ", i);
    for (j = 0; j < size; j++)
    {
      printf(" %d", matrix[i][j]);
    }
    printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *)l);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
  long myid = (long)arg;
  int i, j, first, last;
  int total, min, max;
  int min_pos, max_pos;

  /* determine first and last rows of my strip */
  first = myid * stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

#ifdef DEBUG
  pthread_mutex_lock(&barrier);
  printf("worker %ld (pthread id %lu) has started\n", myid, (unsigned long)pthread_self());
  printf("first: %d\n", first);
  printf("last: %d\n\n", last);
  pthread_mutex_unlock(&barrier);
#endif

  /* sum values in my strip */
  total = 0;
  min = INT_MAX;
  max = INT_MIN;
  for (i = first; i <= last; i++)
  {
    for (j = 0; j < size; j++)
    {
      total += matrix[i][j];
      if (matrix[i][j] > max)
      {
        max = matrix[i][j];
        max_pos = i * size + j;
      }
      if (matrix[i][j] < min)
      {
        min = matrix[i][j];
        min_pos = i * size + j;
      }
    }
  }
  sums[myid] = total;
  max_values[myid] = max;
  min_values[myid] = min;
  max_positions[myid] = max_pos;
  min_positions[myid] = min_pos;

  Barrier();

  if (myid == 0)
  {
    total = 0;
    min = INT_MAX;
    max = INT_MIN;
    min_pos = 0;
    max_pos = 0;
    for (i = 0; i < numWorkers; i++)
    {
      total += sums[i];
      if (max_values[i] > max)
        max = max_values[i];
      max_pos = max_positions[i];
      if (min_values[i] < min)
        min = min_values[i];
      min_pos = min_positions[i];
    }
    /* get end time */
    end_time = read_timer();
    /* print results */

#ifdef DEBUG
    for (int i = 0; i < numWorkers; i++)
    {
      printf("\nmax[%d]: %d\n", i, max_values[i]);
      printf("max_positions[i]: %d\n", max_positions[i]);
      printf("min[%d]: %d\n", i, min_values[i]);
      printf("min_positions[i]: %d\n\n", min_positions[i]);
    }
#endif

    printf("Global max: %d (%d,%d)\n", max, (int)(max_pos / size), max_pos % size);
    printf("Global min: %d (%d,%d)\n", min, (int)(min_pos / size), min_pos % size);
    printf("The total is %d\n", total);
    printf("The execution time is %g sec\n", end_time - start_time);
  }

  return 0;
}
