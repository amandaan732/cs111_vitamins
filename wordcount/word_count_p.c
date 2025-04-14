/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
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
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t *wclist) {
    //there's a separate struct in word_count.h for pthreaad
    list_init(&wclist->lst);
    //this is from pthread.h import
    pthread_mutex_init(&wclist->lock, NULL);
}

size_t len_words(word_count_list_t *wclist) {
    return list_size(&wclist->lst);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
    //no mods = we can jsut read = no locks
    struct list_elem *current;
    for (current = list_begin(&wclist->lst); current != list_end(&wclist->lst); current = list_next(current)) {
        word_count_t *curStruct = list_entry(current, word_count_t, elem);
        if (strcmp(curStruct->word, word) == 0) {
            return curStruct;
        }
    }
    return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
    //lock the list during search/insert
    //lock the word when adding to its count.
    //otherwise its like _l.c
    word_count_t *wc;

    pthread_mutex_lock(&wclist->lock);
    wc = find_word(wclist, word);
    if (wc != NULL) {
        // pthread_mutex_lock(&wc->lock);
        wc->count++;
        // pthread_mutex_unlock(&wc->lock);
        pthread_mutex_unlock(&wclist->lock);
        return wc;
    }

    wc = malloc(sizeof(word_count_t));
    // if (!wc) { //maybe dont need this ; ask office hours
    //     pthread_mutex_unlock(&wclist->list_lock);
    //     return NULL;
    // }
    wc->word = strdup(word); //a copy of word
    // if (!wc->word) { 
    //     free(wc);
    //     pthread_mutex_unlock(&wclist->list_lock);
    //     return NULL;
    // }
    wc->count = 1;
    // pthread_mutex_init(&wc->lock, NULL);
    list_push_back(&wclist->lst, &wc->elem);
    pthread_mutex_unlock(&wclist->lock);
    return wc;
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
    //need to lock each word’s mutex when printing
    //bc can't read while another thread writes
    struct list_elem *current;
    for (current = list_begin(&wclist->lst); current != list_end(&wclist->lst); current = list_next(current)) {
        word_count_t *curStruct = list_entry(current, word_count_t, elem);
        // pthread_mutex_lock(&curStruct->lock);
        fprintf(outfile, "%8d\t%s\n", curStruct->count, curStruct->word);
        // pthread_mutex_unlock(&curStruct->lock);
    }
}

//added this (it's like the one from word_count_l.c)
static bool less_list(const struct list_elem *a,
                      const struct list_elem *b,
                      void *aux) {
    bool (*less)(const word_count_t *, const word_count_t *) = aux;
    word_count_t *wc_a = list_entry(a, word_count_t, elem);
    word_count_t *wc_b = list_entry(b, word_count_t, elem);
    return less(wc_a, wc_b);
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
    list_sort(&wclist->lst, (list_less_func *) less_list, less);
}


