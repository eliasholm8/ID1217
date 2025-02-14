#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

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
    
    // Setup thread attributes.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);




    return 0;
}
