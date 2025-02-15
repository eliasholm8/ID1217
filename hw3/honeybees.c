#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>

#define mutex_lock_name "./mutex_lock"
#define wake_bear_signal_name "./wake_bear_signal"

sem_t *mutex_lock;
sem_t *wake_bear_signal;

pthread_t bear_thread;
pthread_t *bee_threads;

int pot_capacity = 7;
int num_of_bees = 3;
int current_honey;

void *bee_worker(void *arg)
{
    // Get id.
    int id = (int)arg;

    // Loop indefinitely.
    while (true)
    {
        // Wait for the mutex to unlock.
        sem_wait(mutex_lock);

        // Add to the honey.
        current_honey++;

        // PRint to console.
        printf("Bee #%d added to the pot. Now: %d.\n", id, current_honey);

        // Check if the pot is full and signal for the bear, else unlock and let another bee work.
        if (current_honey == pot_capacity) {
            printf("Bee #%d is signaling for the bear!\n", id);
            sem_post(wake_bear_signal);
        }
        else {
            sem_post(mutex_lock);
        }

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
        // Wait for the pot to be full.
        sem_wait(wake_bear_signal);

        // Reset the pot.
        current_honey = 0;

        // Print to console.
        printf("The bear ate the pot. Now %d.\n", current_honey);

        // Unlock the mutex again so the bees can work.
        sem_post(mutex_lock);

        // Sleep a random amount of seconds.
        usleep((1000 + rand() % 2000) * 1000);
    }

    pthread_exit(NULL);
}

/// @brief The main function of the program.
/// @param argc The number of arguments.
/// @param argv The arguments as an array of strings.
/// @return The exit code of the program.
int main(int argc, char *argv[])
{
    // First argument is the pot_capacity.
    if (argc >= 2)
    {
        pot_capacity = atoi(argv[1]);
    }

    // Second argument is num_of_bees.
    if (argc >= 3)
    {
        num_of_bees = atoi(argv[2]);
    }

    // Print config.
    printf("Pot capacity is %d and the number of bees are %d.\n", pot_capacity, num_of_bees);

    // Initalize "randomness".
    srand(time(NULL));

    // Ensure the symaphores doesn't already exist.
    sem_unlink(mutex_lock_name);
    sem_unlink(wake_bear_signal_name);

    // Open named symaphores.
    mutex_lock = sem_open(mutex_lock_name, O_CREAT, 0644, 1);
    wake_bear_signal = sem_open(wake_bear_signal_name, O_CREAT, 0644, 0);

    // Check so the semaphores were created successfully.
    if (mutex_lock == (void *)-1 || wake_bear_signal == (void *)-1)
    {
        printf("Could not create one or more named semaphores.\n");
        return 1;
    }

    // Set current_honey.
    current_honey = 0;

    // Setup thread attributes.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    // Allocate an array for the number of bees.
    bee_threads = malloc(num_of_bees * sizeof(pthread_t));

    // Create the bee threads.
    for (int i = 0; i < num_of_bees; i++)
    {
        pthread_create(&bee_threads[i], &attr, bee_worker, (void *)i);
    }

    // Create a thread for the bear.
    pthread_create(&bear_thread, &attr, parent_worker, NULL);

    // Wait for the bee threads to finish.
    for (int i = 0; i < num_of_bees; i++)
    {
        pthread_join(bee_threads[i], NULL);
    }

    // Wait for the bear thread to finish.
    pthread_join(bear_thread, NULL);

    // Close the semaphores.
    sem_close(mutex_lock);
    sem_close(wake_bear_signal);

    // Free memory.
    free(bee_threads);

    return 0;
}
