/*
 * Word count application with one process per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2019 University of California, Berkeley
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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "word_count.h"
#include "word_helpers.h"

//multiple processes = send results via pipes (bc processes dont share memory)
//need to:
    //make pipe for every child and fork
    //write child and parent processes
    //combine

//idea:
/*
parent --> fork child1 -> open/read file1 -> outputs wordcount to pipe & close file
       --> fork child1 -> open/read file2 -> outputs wordcount to pipe & close file
       --> fork child1 -> open/read file3 -> outputs wordcount to pipe & close file
       --> etc.. (for however many files)

    --> then parent takes all the children's outputs & merges pipes into one final result
*/

/*
 * Read stream of counts and accumulate globally.
 */
void merge_counts(word_count_list_t *wclist, FILE *count_stream) {
    char *word;
    int count;
    int rv;
    while ((rv = fscanf(count_stream, "%8d\t%ms\n", &count, &word)) == 2) {
        add_word_with_count(wclist, word, count);
    }
    if ((rv == EOF) && (feof(count_stream) == 0)) {
        perror("could not read counts");
    } else if (rv != EOF) {
        fprintf(stderr, "read ill-formed count (matched %d)\n", rv);
    }
}

/*
 * main - handle command line, spawning one process per file.
 */
int main(int argc, char *argv[]) {
    /* Create the empty data structure. */
    word_count_list_t word_counts;
    init_words(&word_counts);

    if (argc <= 1) {
        /* Process stdin in a single process. */
        count_words(&word_counts, stdin);
    } else {
        for (int i = 1; i < argc; i++) { //for every file argument
            //creates a pipe where pipefd[0] is the read end & pipefd[1] is the write end.
            //use to send wordcounts from the child to parent.
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            //if pid == 0, then child process; if pid > 0, then parent process
            if (pid == 0) { //child
                close(pipefd[0]); //child doesn't read, so close read end of pipe

                //open write end to output into a FILE*
                FILE *out = fdopen(pipefd[1], "w");
                if (!out) {
                    perror("fdopen (child)");
                    exit(1);
                }

                word_count_list_t local_counts;
                init_words(&local_counts);

                FILE *file = fopen(argv[i], "r");
                if (!file) {
                    perror("fopen");
                    exit(1);
                }

                count_words(&local_counts, file);
                fclose(file);
                wordcount_sort(&local_counts, less_count);
                fprint_words(&local_counts, out); //print wordcounts to output (pipe write end)
                fclose(out); //!!close the write end of the pipe
                exit(0); //exit child process

            } else { //parent process
                close(pipefd[1]); //parent doesnt write, so close pipe's write end
                //kernel signals EOF only once all write ends are closed

                //convert pipefd[0] into a FILE* stream (so in merge_counts you can use fscanf)
                FILE *in = fdopen(pipefd[0], "r");
                if (!in) {
                    perror("fdopen (parent)");
                    exit(1);
                }

                merge_counts(&word_counts, in);
                fclose(in);
            }
        }
    }

    /* Output final result of all process' work. */
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);
    return 0;
}
