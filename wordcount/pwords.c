/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright (C) 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word_count.h"
#include "word_helpers.h"

//need to:
    //spawn threads
    //open and process each file in aseparate thread
    //properly synchronize access to shared data structures when processing files
//should make:
    //struct:
        //has name of file the thread is reading and pointer to shared list -->one per thread
    //a function that each thread runs:
        //open the file (w/ proper error if cant open)
        //count the words
        //CLOSE the file

typedef struct {
    const char *filename;
    word_count_list_t *wclist;
} threadStruct;

void *thread_function(void *arg) {
    threadStruct *threadArg = (threadStruct *)arg;
    FILE *file = fopen(threadArg->filename, "r");
    if (!file) { //throw an error if the file cant be opened
        fprintf(stderr, "could not open file: %s\n", threadArg->filename);
        free(threadArg);
        return NULL;
    }
    //if you CAN open then count and close file
    count_words(threadArg->wclist, file);
    fclose(file);
    free(threadArg);
    return NULL;
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts); //the shared list to use in thread function

    if (argc <= 1) {
        /* Process stdin in a single thread. */
        count_words(&word_counts, stdin);
    } else {
        int num_files = argc - 1;
        //one thread per file in an array
        pthread_t threads[num_files];

        for (int i = 0; i < num_files; i++) {
            //for each file, allocate memory for filename/list (threadStruct)
            threadStruct *threadArg = malloc(sizeof(threadStruct));
            if (!threadArg) { //if can't allocate mem
                fprintf(stderr, "could not allocate memory for thread arg\n");
                exit(1);
            }

            threadArg->filename = argv[i + 1]; //skip the program's name
            threadArg->wclist = &word_counts; //reference to shared list

            //make new thread w/ thread ID stored in &threads[i]
            int rc = pthread_create(&threads[i], NULL, thread_function, threadArg);
            if (rc != 0) {
                fprintf(stderr, "could not create thread for file: %s\n", argv[i + 1]);
                free(threadArg);
                continue;
            }
        }

        //join all the threads (so program has to wait for all threads to finish before exiting)
        for (int i = 0; i < num_files; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    /* Output final result of all threads' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
