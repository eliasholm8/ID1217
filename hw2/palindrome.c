#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Define the buffer size for reading the file.
#define BUFFERSIZE 1024

// Create global variables for the program.
FILE *file_pointer;
int line_count, palindromes_count, semordnilaps_count;
char **lines;
char **palindromes;
char **semordnilaps;

/// @brief Function used to count the number of lines in the input file.
/// @return The number of lines in the file.
int count_lines()
{
    // Initialize the count and a variable for the current character.
    int count = 0;
    char ch;

    // Loop through all characters in the file.
    while ((ch = fgetc(file_pointer)) != EOF)
    {
        // Check if we found a newline character, and increment the count if we did.
        if (ch == '\n')
        {
            count++;
        }
    }

    // Get the last char.
    fseek(file_pointer, -1, SEEK_END);
    ch = fgetc(file_pointer);

    // Add a line if the last character is not a newline.
    if (ch != '\n')
    {
        count++;
    }

    // Return to the start of the file.
    fseek(file_pointer, 0, SEEK_SET);

    printf("Number of words: %d\n", count);

    // Return the number of lines we found.
    return count;
}

/// @brief Function used to read the lines from the input file.
void read_lines()
{
    // Count the number of lines/words in the files.
    line_count = count_lines();

    // Allocate arrays for the maximum number of words.
    lines = malloc(line_count * sizeof(char *));

    // Create a buffer for reading the lines.
    char buffer[BUFFERSIZE];

    // Read the lines from the file.
    for (int i = 0; i < line_count; i++)
    {
        // Read the line from the file.
        if (fgets(buffer, BUFFERSIZE, file_pointer) == NULL)
        {
            printf("Could not read line %d\n", i);
            return;
        }

        // Read each character in the line and convert it to lowercase.
        for (char *p = buffer; *p; p++)
        {
            // Make the character lowercase.
            *p = tolower(*p);

            // Replace the newline character with a null character.
            if (*p == '\n')
            {
                *p = '\0';
            }
        }

        // Store the line in the array of words/lines.
        lines[i] = strdup(buffer);
    }
}

/// @brief Helper function used to compare two strings.
/// @param a The first string.
/// @param b The other string to compare with.
/// @return Integer of which order the first string compares to the other.
int compare_func(const void *a, const void *b)
{
    const char *first = *(const char **)a;
    const char *second = *(const char **)b;
    return strcmp(first, second);
}

/// @brief Function used to sort all the lines in the array.
void sort_lines()
{
    qsort(lines, line_count, sizeof(char *), compare_func);
}

/// @brief Function used to find a specific string in the array.
/// @param to_find The string to find in the array.
/// @return The index of the found string or -1 if it wasn't found.
int find_line(char *to_find)
{
    // Create variables for low and high-
    int low = 0;
    int high = line_count - 1;

    // Loop through the segments of the array.
    while (low <= high)
    {
        // Find the middle of the array, and compare the strings.
        int middle = (low + high) / 2;
        int compare = strcmp(to_find, lines[middle]);

        // Decide if we should move up or down depending on our comparison.
        if (compare < 0)
            high = middle - 1;
        else if (compare > 0)
            low = middle + 1;
        else
            return middle;
    }

    // Return -1 if we didn't find the line.
    return -1;
}

/// @brief Function used to reverse a string.
/// @param str A string pointer for the string to be reversed.
void reverse_string(char *str)
{
    // Get the length of the string.
    int len = strlen(str);

    // Loop through half the string and swap the characters.
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

/// @brief Function used to find the palindromes and semordnilaps in the array.
void find_words()
{
    // Allocate array for palindromes and semordnilaps, and initialize counts.
    palindromes = malloc(line_count * sizeof(char *));
    semordnilaps = malloc(line_count * sizeof(char *));
    palindromes_count = 0;
    semordnilaps_count = 0;

    // Get the start time of the concurrent part of the program.
    double start_time = omp_get_wtime();

    // Parallelize the for loop, between the threads.
    #pragma omp parallel for
    for (int i = 0; i < line_count; i++)
    {
        // Set the current line.
        char *current_line = lines[i];

        // Create a copy of the line.
        char *reversed_line = strdup(current_line);

        // Reverse the line.
        reverse_string(reversed_line);

        // Check if the reversed line is the same as the original line, or if it is a semordnilap.
        if (strcmp(current_line, reversed_line) == 0)
        {
            // Increment the count of the palindromes atomically.
            int insert_index;
            #pragma omp atomic capture
            insert_index = palindromes_count++;

            // Store the palindrome in the array.
            palindromes[insert_index] = current_line;
        }
        else if (find_line(reversed_line) != -1)
        {
            // Increment the count of the palindromes atomically.
            int insert_index;
            #pragma omp atomic capture
            insert_index = semordnilaps_count++;

            // Store the palindrome in the array.
            semordnilaps[insert_index] = current_line;
        }

        // Free the memory of the line.
        free(reversed_line);
    }

    // Get the end time and calculate the elapsed time.
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;

    // Print the elapsed time.
    printf("Concurrent processing time: %f sec\n", elapsed);
}

/// @brief Function used to print the results to a results.txt file.
void print_results()
{
    // Open the output file for writing.
    FILE *output_pointer = fopen("results.txt", "w+");
    if (output_pointer == NULL)
    {
        printf("Could not open output file.\n");
        return;
    }

    // Print title for Palindromes result, and then print all the palindromes.
    fprintf(output_pointer, "Palindromes result:\n");
    for (int i = 0; i < palindromes_count; i++)
    {
        fprintf(output_pointer, "%s\n", palindromes[i]);
    }

    // Print title for Semordnilaps result, and then print all the semordnilaps.
    fprintf(output_pointer, "\nSemordnilaps:\n");
    for (int i = 0; i < semordnilaps_count; i++)
    {
        fprintf(output_pointer, "%s\n", semordnilaps[i]);
    }

    // Close the output file.
    fclose(output_pointer);
}

/// @brief The main function of the program.
/// @param argc The number of arguments.
/// @param argv The arguments as an array of strings.
/// @return The exit code of the program.
int main(int argc, char *argv[])
{
    // Default value for number of threads.
    int num_threads = 1;

    // Check if all the correct arguments are provided.
    if (argc < 3)
    {
        printf("Missing arguments. %d\n", argc);
        return 1;
    }

    // Parse the number of threads from the arguments.
    num_threads = atoi(argv[2]);

    // Ensure that we have at least 1 thread.
    if (num_threads < 1)
        num_threads = 1;

    // Set the number of threads.
    omp_set_num_threads(num_threads);

    // Open the file of words that we received as an argument.
    file_pointer = fopen(argv[1], "r");
    if (file_pointer == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    // Check so that the file is not empty.
    fseek(file_pointer, 0, SEEK_END);
    if (ftell(file_pointer) == 0)
    {
        printf("Input file is empty.\n");
        return 1;
    }

    // Return to the start of the file.
    fseek(file_pointer, 0, SEEK_SET);

    // Read the words from the same file and ensure that they are sorted.
    read_lines();
    sort_lines();

    // Close the file.
    fclose(file_pointer);

    // Call find_words to find the palindromes and semordnilaps.
    find_words();

    // Print counts
    printf("Palindromes count: %d\n", palindromes_count);
    printf("Semordnilaps count: %d\n", semordnilaps_count);

    // Print the results to the output file.
    print_results();

    // Free the memory of the arrays.
    for (int i = 0; i < line_count; i++)
    {
        free(lines[i]);
    }
    free(lines);
    free(palindromes);
    free(semordnilaps);

    // Return success.
    return 0;
}