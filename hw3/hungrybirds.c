#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define mutex_lock_name "/mutex_lock"
#define refill_signal_name "/refill_signal"
#define worms_available_name "/worms_available"

sem_t *mutex_lock;
sem_t *refill_signal;
sem_t *worms_available;

pthread_t parent_bird;
pthread_t *baby_birds;

int worms_to_add = 7;
int num_of_birds = 3;
int worm_count;

void *baby_worker(void *arg)
{
    // Get id.
    int id = (int)arg;

    // Loop indefinitely.
    while (true)
    {
        // Wait for food and decrement the food count.
        sem_wait(worms_available);

        // Lock the mutex by setting it back to 0.
        sem_wait(mutex_lock);

        // Decrement the worm count.
        printf("Bird #%d ate a worm. There are %d worms left.\n", id, --worm_count);

        // Check if we need to ask for more worms.
        if (worm_count == 0)
        {
            printf("WE NEED MORE FOOD!\n");
            sem_post(refill_signal);
        }

        // Unlock the mutex.
        sem_post(mutex_lock);

        // Sleep a random amount of seconds.
        usleep((100 + rand() % 2000) * 1000);
    }

    pthread_exit(NULL);
}

void *parent_worker()
{
    // Loop indefinitely.
    while (true)
    {
        // Wait for a signal that we need to refill.
        sem_wait(refill_signal);

        // Sleep a random amount of seconds.
        usleep((1000 + rand() % 2000) * 1000);

        // Lock the mutex by setting it back to 0.
        sem_wait(mutex_lock);

        // Refill the worms.
        for (int i = 0; i < worms_to_add; i++)
        {
            sem_post(worms_available);
        }

        // Update the integer counter.
        worm_count += worms_to_add;

        // Unlock the mutex by setting it back to 1.
        sem_post(mutex_lock);

        // Print to console.
        printf("[Parent] Added %d worms to the count. Now: %d\n", worms_to_add, worm_count);
    }

    pthread_exit(NULL);
}

/// @brief The main function of the program.
/// @param argc The number of arguments.
/// @param argv The arguments as an array of strings.
/// @return The exit code of the program.
int main(int argc, char *argv[])
{
    // First argument is the worms_to_add.
    if (argc >= 2)
    {
        worms_to_add = atoi(argv[1]);
    }

    // Second argument is num_of_birds.
    if (argc >= 3)
    {
        num_of_birds = atoi(argv[2]);
    }

    // Initalize "randomness".
    srand(time(NULL));

    // Ensure the symaphores doesn't already exist.
    sem_unlink(mutex_lock_name);
    sem_unlink(refill_signal_name);
    sem_unlink(worms_available_name);

    // Open named symaphores.
    mutex_lock = sem_open(mutex_lock_name, O_CREAT, 0644, 1);
    refill_signal = sem_open(refill_signal_name, O_CREAT, 0644, 0);
    worms_available = sem_open(worms_available_name, O_CREAT, 0644, worms_to_add);

    // Check so the semaphores were created successfully.
    if (mutex_lock == (void *)-1 || refill_signal == (void *)-1 || worms_available == (void *)-1)
    {
        printf("Could not create one or more named semaphores.\n");
        return 1;
    }

    // Set worm_count.
    worm_count = worms_to_add;

    // Setup thread attributes.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    // Allocate an array for the number of baby birds.
    baby_birds = malloc(num_of_birds * sizeof(pthread_t));

    // Create the baby bird threads.
    for (int i = 0; i < num_of_birds; i++)
    {
        pthread_create(&baby_birds[i], &attr, baby_worker, (void *)i);
    }

    // Create a thread for the parent bird.
    pthread_create(&parent_bird, &attr, parent_worker, NULL);

    // Wait for the baby bird threads to finish.
    for (int i = 0; i < num_of_birds; i++)
    {
        pthread_join(baby_birds[i], NULL);
    }

    // Wait for the parent bird thread to finish.
    pthread_join(parent_bird, NULL);

    // Close the semaphores.
    sem_close(mutex_lock);
    sem_close(refill_signal);
    sem_close(worms_available);

    // Free memory.
    free(baby_birds);

    return 0;
}
