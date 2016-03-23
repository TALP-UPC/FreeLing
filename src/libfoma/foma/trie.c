/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2010 Mans Hulden                                     */

/*     This file is part of foma.                                            */

/*     Foma is free software: you can redistribute it and/or modify          */
/*     it under the terms of the GNU General Public License version 2 as     */
/*     published by the Free Software Foundation.                            */

/*     Foma is distributed in the hope that it will be useful,               */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*     GNU General Public License for more details.                          */

/*     You should have received a copy of the GNU General Public License     */
/*     along with foma.  If not, see <http://www.gnu.org/licenses/>.         */

#include "fomalib.h"

#define THASH_TABLESIZE 1048573
#define TRIE_STATESIZE 32768

unsigned int trie_hashf(unsigned int source, char *insym, char *outsym);

struct fsm_trie_handle *fsm_trie_init() {
    struct fsm_trie_handle *th;

    th = xxcalloc(1,sizeof(struct fsm_trie_handle));
    th->trie_hash = xxcalloc(THASH_TABLESIZE, sizeof(struct trie_hash));
    th->trie_states = xxcalloc(TRIE_STATESIZE, sizeof(struct trie_states));
    th->statesize = TRIE_STATESIZE;
    th->trie_cursor = 0;
    th->sh_hash = sh_init();
    return(th);
}

struct fsm *fsm_trie_done(struct fsm_trie_handle *th) {
    struct trie_hash *thash, *thashp;
    struct fsm *newnet;
    struct fsm_construct_handle *newh;
    unsigned int i;

    newh = fsm_construct_init("name");
    for (i = 0; i < THASH_TABLESIZE; i++) {
	thash = (th->trie_hash)+i;
	for ( ; thash != NULL; thash = thash->next) {
	    if (thash->insym != NULL) {
		fsm_construct_add_arc(newh, thash->sourcestate, thash->targetstate, thash->insym, thash->outsym);
	    } else {
		break;
	    }		
	}
    }
    for (i = 0; i <= th->used_states; i++) {
	if ((th->trie_states+i)->is_final == 1) {
	    fsm_construct_set_final(newh, i);
	}
    }
    fsm_construct_set_initial(newh, 0);
    newnet = fsm_construct_done(newh);
    /* Free all mem */
    for (i=0; i < THASH_TABLESIZE; i++) {
	for (thash=((th->trie_hash)+i)->next; thash != NULL; thash = thashp) {
	    thashp = thash->next;
	    xxfree(thash);
	}
    }
    sh_done(th->sh_hash);
    xxfree(th->trie_states);
    xxfree(th->trie_hash);
    xxfree(th);
    return(newnet);
}

void fsm_trie_add_word(struct fsm_trie_handle *th, char *word) {
    int i, len;
    char *wcopy;
    wcopy = xxstrdup(word);
    len = strlen(wcopy);
    for (i=0 ; *word != '\0' && i < len; word = word + utf8skip(word)+1, i++) {
	strncpy(wcopy, word, utf8skip(word)+1);
	*(wcopy+utf8skip(word)+1) = '\0';
	fsm_trie_symbol(th, wcopy, wcopy);
    }
    xxfree(wcopy);
    fsm_trie_end_word(th);
}

void fsm_trie_end_word(struct fsm_trie_handle *th) {
    (th->trie_states+th->trie_cursor)->is_final = 1;
    th->trie_cursor = 0;
}

void fsm_trie_symbol(struct fsm_trie_handle *th, char *insym, char *outsym) {
    unsigned int h;
    struct trie_hash *thash, *newthash;

    h = trie_hashf(th->trie_cursor, insym, outsym);
    if ((th->trie_hash+h)->insym != NULL) {
	for (thash = th->trie_hash+h; thash != NULL; thash = thash->next) {
	    if (strcmp(thash->insym, insym) == 0 && strcmp(thash->outsym, outsym) == 0 && thash->sourcestate == th->trie_cursor) {
		/* Exists, move cursor */
		th->trie_cursor = thash->targetstate;
		return;
	    }
	}
    }
    /* Doesn't exist */
    
    /* Insert trans, move counter and cursor */
    th->used_states++;
    thash = th->trie_hash+h;
    if (thash->insym == NULL) {
	thash->insym = sh_find_add_string(th->sh_hash, insym,1);
	thash->outsym = sh_find_add_string(th->sh_hash, outsym,1);
	thash->sourcestate = th->trie_cursor;
	thash->targetstate = th->used_states;
    } else {
	newthash = xxcalloc(1, sizeof(struct trie_hash));
	newthash->next = thash->next;
	newthash->insym = sh_find_add_string(th->sh_hash, insym,1);
	newthash->outsym = sh_find_add_string(th->sh_hash, outsym,1);
	newthash->sourcestate = th->trie_cursor;
	newthash->targetstate = th->used_states;
	thash->next = newthash;
    }
    th->trie_cursor = th->used_states;

    /* Realloc */
    if (th->used_states >= th->statesize) {
	th->statesize = next_power_of_two(th->statesize);
	th->trie_states = xxrealloc(th->trie_states, th->statesize * sizeof(struct trie_states));
    }
    (th->trie_states+th->used_states)->is_final = 0;
}

unsigned int trie_hashf(unsigned int source, char *insym, char *outsym) {

    /* Hash based on insym, outsym, and sourcestate */
    register unsigned int hash;
    hash = 0;
    
    while (*insym != '\0') {
        hash = hash * 101  +  *insym++;
    }
    while (*outsym != '\0') {
        hash = hash * 101  +  *outsym++;
    }
    hash = hash * 101 + source;
    return (hash % THASH_TABLESIZE);
}
