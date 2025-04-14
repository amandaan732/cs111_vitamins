/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
    list_init(wclist);
}

size_t len_words(word_count_list_t *wclist) {
    return list_size(wclist);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
    //go through wc list and check if the list element == the word
    //need to check the types of each thing 
    //word_count_t is a struct --> we need to make wc's list_entry into a word_count_t typedef
    //then that word_count_t's char *word element should be compared to *word 
        //if they are equal rerturn the struct, otherwise return NULL

    for (struct list_elem *current = list_begin(wclist); current != list_end(wclist); current = list_next(current)) {
        word_count_t *curStruct = list_entry(current, word_count_t, elem);
        if (strcmp(curStruct->word, word) == 0) {
            return curStruct;
        }
    }
    // char checkWord;
    // if(strcmp(checkWord, word) == 0) {
    //     return checkWord;
    // }
    return NULL;
}

word_count_t *add_word_with_count(word_count_list_t *wclist, char *word,
                                  int count) {
    //check if the word exists, if it does increment the count and return current node
    //if not found
        //allocate memory for new node --> like in word_count.c
        //wc->word = word and wc->count = count
        //insert into wclist
        //return wc

    word_count_t *wc = find_word(wclist, word);
    if (wc != NULL) {
        wc->count += count;
        return wc;
    }

    wc = malloc(sizeof(word_count_t));
    if (wc == NULL) {
        perror("malloc");
        return NULL;
    }

    wc->word = word;
    wc->count = count;

    list_push_back(wclist, &wc->elem);
    return wc;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
    return add_word_with_count(wclist, word, 1);
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
    //iterate through wclist
    //extract struct pointer from list element --> like in find_word
    //fprintf to outfile like in word_count.c

    for (struct list_elem *current = list_begin(wclist); current != list_end(wclist); current = list_next(current)) {
        word_count_t *curStruct = list_entry(current, word_count_t, elem);
        fprintf(outfile, "%8d\t%s\n", wc->count, wc->word);
    }
}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
    //

    bool (*less)(const word_count_t *, const word_count_t *) = aux;
    word_count_t *wc1 = list_entry(ewc1, word_count_t, elem);
    word_count_t *wc2 = list_entry(ewc2, word_count_t, elem);
    return less(wc1, wc2);
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
    list_sort(wclist, less_list, less);
}
