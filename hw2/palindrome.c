#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define BUFFERSIZE 1024

FILE *file_pointer;
int line_count;
char **lines;

// Create function to count the number of lines in the file.
int count_lines()
{
    int count = 0;
    char ch;

    while ((ch = fgetc(file_pointer)) != EOF)
    {
        if (ch == '\n')
        {
            count++;
        }
    }

    fseek(file_pointer, 0, SEEK_SET);

    return count;
}

// Create function to read the lines from the file.
void read_lines()
{
    char buffer[BUFFERSIZE];

    for (int i = 0; i < line_count; i++)
    {
        if (fgets(buffer, BUFFERSIZE, file_pointer) == NULL)
        {
            printf("Could not read line %d\n", i);
            return;
        }

        for (char *p = buffer; *p; p++)
        {
            *p = tolower(*p);

            if (*p == '\n')
            {
                *p = '\0';
            }
        }

        lines[i] = strdup(buffer);
    }
}

// Sort lines.
int compare_func(const void *a, const void *b)
{
    const char *first = *(const char **)a;
    const char *second = *(const char **)b;
    return strcmp(first, second);
}

void sort_lines()
{
    qsort(lines, line_count, sizeof(char *), compare_func);
}

// Function to find a line using binary search.
int find_line(char *to_find)
{
    int low = 0;
    int high = line_count - 1;

    while (low <= high)
    {
        int middle = (low + high) / 2;
        int compare = strcmp(to_find, lines[middle]);

        if (compare < 0)
            high = middle - 1;
        else if (compare > 0)
            low = middle + 1;
        else
            return middle;
    }

    return -1;
}

// Function to reverse a string.
void reverse_string(char *str)
{
    int len = strlen(str);

    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

/// @brief The main function of the program.
/// @param argc The number of arguments.
/// @param argv The arguments as an array of strings.
/// @return The exit code of the program.
int main(int argc, char *argv[])
{
    int num_threads = 1;
    
    // Check if the file name is provided.
    if (argc < 2)
    {
        printf("Missing arguments. %d\n", argc);
        return 1;
    }
    num_threads = atoi(argv[2]);
    if (num_threads < 1) num_threads = 1;
    omp_set_num_threads(num_threads);

    // Open the file.
    file_pointer = fopen(argv[1], "r");
    if (file_pointer == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    line_count = count_lines();
    lines = malloc(line_count * sizeof(char *));
    read_lines();
    sort_lines();


    // Get the start time
    double start_time = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 0; i < line_count; i++)
    {

        //int thread_id = omp_get_thread_num(); // Get thread ID
        //printf("Thread %d processing line %d\n", thread_id, i);

        // Set the current line.
        char *current_line = lines[i];

        // Create a copy of the line.
        char *reversed_line = strdup(current_line);

        // Reverse the line.
        reverse_string(reversed_line);

        // printf("Current line: %s\n", current_line);
        // printf("reversed line: %s\n", reversed_line);


        // Check if the reversed line is the same as the original line.
        if (strcmp(current_line, reversed_line) == 0)
        {
            //printf("Palindrome: %s\n", current_line);
        }

        // Find the reversed line in the sorted lines.
        int index = find_line(reversed_line);

        // If the reversed line is found in the sorted lines.
        if (index != -1)
        {
            //printf("Semordnilaps: %s @ %d\n", current_line, index);
        }

        // Free the memory of the line.
        free(reversed_line);
    }

    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    printf("time: %f\n",elapsed);

    return 0;
}