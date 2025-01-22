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
#define BUFFERSIZE 1024
#define BLOCKSIZE 100

// Function prototypes.
void *read_worker();
void *stdout_worker();
void *write_worker();

/// @brief A task containing a buffer, a mutex to lock the buffer, and a flag to indicate if the task has been partially processed.
typedef struct Task
{
    char Buffer[BUFFERSIZE];
    pthread_mutex_t mutex;
    bool is_partially_processed;
} Task;

/// @brief A block of tasks, also containing a pointer to the next block, and a pointer to the last task in the block.
typedef struct TaskBlock
{
    struct Task tasks[BLOCKSIZE];
    struct Task *tail;
    struct TaskBlock *next_block;
} TaskBlock;

// Global variables.
TaskBlock *initial_block;
bool finished_reading = false;
FILE *file_pointer;

/// @brief The main function of the program.
/// @param argc The number of arguments.
/// @param argv The arguments as an array of strings.
/// @return The exit code of the program.
int main(int argc, char *argv[])
{
    // Check if the file name is provided.
    if (argc < 2)
    {
        printf("Missing arguments. %d\n", argc);
        exit(1);
    }

    // Open the file.
    file_pointer = fopen(argv[1], "w+");
    if (file_pointer == NULL)
    {
        printf("Could not open file.\n");
        exit(1);
    }

    // Create thread variables.
    pthread_attr_t attr;
    pthread_t read_thread, stdout_thread, write_thread;

    // Set the thread attributes.
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    // Create the initial block.
    initial_block = (TaskBlock *)malloc(sizeof(TaskBlock));
    initial_block->next_block = NULL;
    initial_block->tail = &initial_block->tasks[0];

    // Init the first task's mutex and lock it so the read thread can start reading.
    pthread_mutex_init(&initial_block->tail->mutex, NULL);
    pthread_mutex_lock(&initial_block->tail->mutex);

    // Create the threads.
    pthread_create(&read_thread, &attr, read_worker, NULL);
    pthread_create(&stdout_thread, &attr, stdout_worker, NULL);
    pthread_create(&write_thread, &attr, write_worker, NULL);

    // Wait for the threads to finish.
    pthread_join(read_thread, NULL);
    pthread_join(stdout_thread, NULL);
    pthread_join(write_thread, NULL);

    // Close the file.
    fclose(file_pointer);

    return 0;
}

/// @brief This function reads input from stdin and creates tasks for the write and stdout threads.
void *read_worker()
{
    // The block and task we are currently writing to.
    TaskBlock *cur_block = initial_block;
    Task *cur_task = &cur_block->tasks[0];

    // Read input from stdin and write it to the current task (which is locked from reading).
    while (fgets(cur_task->Buffer, BUFFERSIZE, stdin))
    {
        // Init its variables.
        cur_task->is_partially_processed = false;

        // Set the tail to the current task.
        cur_block->tail = cur_task;

        // Create variable for the next task.
        Task *next_task;

        // If we are not at the end of the block, just move to the next task. Else, create a new block and move to the first task.
        if (cur_task < &cur_block->tasks[BLOCKSIZE - 1])
        {
            next_task = cur_task + 1;
        }
        else
        {
            // Create a new block.
            TaskBlock *next_block = (TaskBlock *)malloc(sizeof(TaskBlock));
            next_block->next_block = NULL;
            next_block->tail = &next_block->tasks[0];

            // Update the next task and block.
            next_task = &next_block->tasks[0];
            cur_block->next_block = next_block;
            cur_block = next_block;
        }

        // Init the next task's mutex and lock it.
        pthread_mutex_init(&next_task->mutex, NULL);
        pthread_mutex_lock(&next_task->mutex);

        // Finally, unlock the current task and move to the next task.
        pthread_mutex_unlock(&cur_task->mutex);
        cur_task = next_task;
    }

    // Release the last task's mutex and set the finished_reading flag to true.
    pthread_mutex_unlock(&cur_task->mutex);
    finished_reading = true;
    pthread_exit(0);
}

/// @brief This function processes tasks containing strings and calls process_task with task string as argument.
void *task_worker(void (*process_task)(char *))
{
    TaskBlock *cur_block = initial_block;
    Task *cur_task = &cur_block->tasks[0];

    // Continue reading data if the reader thread is not finished, there is more blocks to read, or there are more tasks to read in the current block.
    while (!finished_reading || cur_block->next_block != NULL || cur_task <= cur_block->tail)
    {
        // Wait for the task to unlock and lock it.
        pthread_mutex_lock(&cur_task->mutex);

        // Check if the buffer is a null terminator, and if so, break the loop.
        if (cur_task->Buffer[0] == '\0')
        {
            pthread_mutex_unlock(&cur_task->mutex);
            break;
        }

        // Manage output.
        process_task(cur_task->Buffer);

        // If the task has been processed by the write thread, and
        if (cur_task == &cur_block->tasks[BLOCKSIZE - 1])
        {
            // Store the next block in a temporary pointer.
            TaskBlock *next_block = cur_block->next_block;

            // Free the current block if it has been (now) fully processed.
            if (cur_task->is_partially_processed == true)
            {
                free(cur_block);
            }

            // Set the current task as partially processed.
            cur_task->is_partially_processed = true;

            // Unlock the task.
            pthread_mutex_unlock(&cur_task->mutex);

            // Move to the next block and task.
            cur_block = next_block;
            cur_task = &cur_block->tasks[0];
        }
        else
        {
            // Set the task as partially processed and unlock it.
            cur_task->is_partially_processed = true;
            pthread_mutex_unlock(&cur_task->mutex);

            // Move to the next task.
            cur_task++;
        }
    }

    pthread_exit(0);
}

/// @brief This function processes the output to stdout. It is effectively called by the task_worker function.
/// @param content The string to be printed to stdout.
void process_stdout(char *content)
{
    printf("%s", content);
}

/// @brief This function processes the output to the file. It is effectively called by the task_worker function.
/// @param content The string to be written to the file.
void process_write(char *content)
{
    fprintf(file_pointer, "%s", content);
}

/// @brief This function is used by the stdout thread to process tasks and print them to stdout.
void *stdout_worker()
{
    task_worker(process_stdout);
    pthread_exit(0);
}

/// @brief This function is used by the write thread to process tasks and write them to the file.
void *write_worker()
{
    task_worker(process_write);
    pthread_exit(0);
}