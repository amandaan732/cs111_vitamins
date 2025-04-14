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
        free(targ);
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
    init_words(&word_counts);

    if (argc <= 1) {
        /* Process stdin in a single thread. */
        count_words(&word_counts, stdin);
    } else {
        /* TODO */
    }

    /* Output final result of all threads' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
