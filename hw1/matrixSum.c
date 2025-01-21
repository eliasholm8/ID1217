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
#include <unistd.h>

#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */
// #define DEBUG

pthread_mutex_t barrier; /* mutex lock for the barrier */
pthread_cond_t go;		 /* condition variable for leaving */
int numWorkers;			 /* number of workers */
int numArrived = 0;		 /* number who have arrived */

int next_row_counter = 0;
pthread_mutex_t next_row_counter_mutex;

int get_next_row()
{
	pthread_mutex_lock(&next_row_counter_mutex);
	int row = next_row_counter++;
	pthread_mutex_unlock(&next_row_counter_mutex);
	return row;
}

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
int size, stripSize;		  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

typedef struct
{
	int sum;
	int max;
	int min;
	int max_pos;
	int min_pos;
} WorkerResult;

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
	pthread_mutex_init(&next_row_counter_mutex, NULL);
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

	/* Create variables for storing data. */
	int total = 0;
	int min = INT_MAX;
	int max = INT_MIN;
	int min_pos, max_pos;

	/* Join the results from all workers */
	for (l = 0; l < numWorkers; l++)
	{
		WorkerResult *cur_result;
		pthread_join(workerid[l], (void **)&cur_result);

		total += cur_result->sum;
		if (cur_result->max > max)
		{
			max = cur_result->max;
			max_pos = cur_result->max_pos;
		}
		if (cur_result->min < min)
		{
			min = cur_result->min;
			min_pos = cur_result->min_pos;
		}

#ifdef DEBUG
		printf("\nmax->%ld: %d\n", l, cur_result->max);
		printf("max_pos->%ld: %d\n", l, cur_result->max_pos);
		printf("min->%ld: %d\n", l, cur_result->min);
		printf("min_pos->%ld: %d\n\n", l, cur_result->min_pos);
#endif

		free(cur_result);
	}

	/* get end time */
	end_time = read_timer();

	printf("Global max: %d (%d,%d)\n", max, (int)(max_pos / size), max_pos % size);
	printf("Global min: %d (%d,%d)\n", min, (int)(min_pos / size), min_pos % size);
	printf("The total is %d\n", total);
	printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
	long myid = (long)arg;
	int total, min, max;
	int min_pos, max_pos;

#ifdef DEBUG
	printf("INIT: worker %ld (pthread id %lu) has started\n", myid, (unsigned long)pthread_self());
#endif

	/* sum values in my strip */
	total = 0;
	min = INT_MAX;
	max = INT_MIN;

	int cur_row;
	int i;

	while ((cur_row = get_next_row()) < size)
	{
#ifdef DEBUG
		printf("TASK: worker %ld started working on row #%d\n", myid, cur_row);
#endif
		for (i = 0; i < size; i++)
		{
			total += matrix[cur_row][i];
			if (matrix[cur_row][i] > max)
			{
				max = matrix[cur_row][i];
				max_pos = cur_row * size + i;
			}
			if (matrix[cur_row][i] < min)
			{
				min = matrix[cur_row][i];
				min_pos = cur_row * size + i;
			}
		}
	}

	WorkerResult *result = malloc(sizeof(WorkerResult));
	result->sum = total;
	result->max = max;
	result->min = min;
	result->max_pos = max_pos;
	result->min_pos = min_pos;

	pthread_exit((void *)result);
}
