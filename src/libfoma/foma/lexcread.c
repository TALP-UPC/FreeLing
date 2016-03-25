/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2014 Mans Hulden                                     */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foma.h"
#include "lexc.h"

#define SIGMA_HASH_TABLESIZE 3079

#define WORD_ENTRY 1
#define REGEX_ENTRY 2

extern int g_lexc_align;

struct multichar_symbols {
    char *symbol;
    short int sigma_number;
    struct multichar_symbols *next;
};

struct lexstates {             /* Separate list of LEXICON states */
    char *name;    
    struct states *state;
    struct lexstates *next;
    unsigned char targeted;
    unsigned char has_outgoing;
};

struct states {
    struct trans {
        short int in;
        short int out;
        struct states *target;
        struct trans *next;
    } *trans;
    struct lexstates *lexstate; /* ptr to lexicon state */
    int number;                 /* State number (generated later) */
    unsigned int hashval;       /* Hash for remaining symbols until next lexstate */
    unsigned char mergeable;    /* Can this state be merged with other suffix */
                                /* 0 = NO, 1 = YES, 2 = DELETED/MERGED */
    unsigned short int distance;      /* Number of remaining symbols until lexstate */
    struct states *merge_with;
};

struct statelist {
    struct states *state;
    struct statelist *next;
    char start;
    char final;
};

struct lexc_hashtable {      /* Hash for looking up symbols in sigma quickly */
    char *symbol;
    struct lexc_hashtable *next;
    int sigma_number;
};

static unsigned int primes[26] = {61,127,251,509,1021,2039,4093,8191,16381,32749,65521,131071,262139,524287,1048573,2097143,4194301,8388593,16777213,33554393,67108859,134217689,268435399,536870909,1073741789,2147483647};

static struct statelist *statelist = NULL;
static struct multichar_symbols *mc = NULL;
static struct lexstates *lexstates = NULL;
static struct sigma *lexsigma = NULL;
static struct lexc_hashtable *hashtable;
static struct fsm *current_regex_network;

static int cwordin[1000], cwordout[1000], medcwordin[2000], medcwordout[2000], carity, lexc_statecount, maxlen, hasfinal, current_entry, net_has_unknown;
static _Bool *mchash;
static struct lexstates *clexicon, *ctarget;

static char *mystrncpy(char *dest, char *src, int len);
static void lexc_string_to_tokens(char *string, int *intarr);
static void lexc_pad();
static void lexc_medpad();
static void lexc_number_states();
static void lexc_cleanup();
static unsigned int lexc_suffix_hash(int offset);
static unsigned int lexc_symbol_hash(char *s);
static void lexc_update_unknowns(int sigma_number);

static unsigned int lexc_suffix_hash(int offset) {
    register unsigned int h = 0, g, p;
    /* Read suffixes in cwordin[] and cwordout[] and return a hash value */
    for(p = offset; cwordin[p] != -1; p++) {
        h = (h << 4) + (unsigned int) (cwordin[p] | (cwordout[p] << 8));
        if (0 != (g = h & 0xf0000000)) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    /* No tablemod here, we decide on the table size later */
    return h;
}

static unsigned int lexc_symbol_hash(char *s) {
    register unsigned int hash;
    int c;
    hash = 5381;
    while ((c = *s++))
	hash = ((hash << 5) + hash) + c;
    return (hash % SIGMA_HASH_TABLESIZE);
}

int lexc_find_sigma_hash(char *symbol) {
    int ptr;
    struct lexc_hashtable *h;
    ptr = lexc_symbol_hash(symbol);

    if ((hashtable+ptr)->symbol == NULL)
        return -1;
    for (h = (hashtable+ptr); h != NULL; h = h->next) {
        if (strcmp(symbol,h->symbol) == 0) {
            return (h->sigma_number);
        }
    }
    return -1;
}

void lexc_add_sigma_hash(char *symbol, int number) {
    int ptr;
    struct lexc_hashtable *h, *hnew;
    ptr = lexc_symbol_hash(symbol);

    if (net_has_unknown == 1)
        lexc_update_unknowns(number);

    if ((hashtable+ptr)->symbol == NULL) {
        (hashtable+ptr)->symbol = xxstrdup(symbol);
        (hashtable+ptr)->sigma_number = number;
        return;
    }
    for (h = hashtable+ptr; h->next != NULL; h = h->next) {
    }
    hnew = xxmalloc(sizeof(struct lexc_hashtable));
    hnew->symbol = xxstrdup(symbol);
    hnew->sigma_number = number;
    h->next = hnew;
    hnew->next = NULL;
}

void lexc_init() {
    int i;
    lexsigma = sigma_create();
    mc = NULL;
    lexstates = NULL;
    clexicon = NULL;
    ctarget = NULL;
    statelist = NULL;
    lexc_statecount = 0;
    net_has_unknown = 0;
    lexc_clear_current_word();
    hashtable = xxcalloc(SIGMA_HASH_TABLESIZE, sizeof(struct lexc_hashtable));

    maxlen = 0;

    mchash = xxcalloc(256*256, sizeof(_Bool));
    for (i=0; i< SIGMA_HASH_TABLESIZE; i++) {
        (hashtable+i)->symbol = NULL;
        (hashtable+i)->sigma_number = -1;
        (hashtable+i)->next = NULL;
    }
}

void lexc_clear_current_word() {
    cwordin[0] = cwordout[0] = 0;
    cwordin[1] = cwordout[1] = -1;
    current_entry = WORD_ENTRY;
}

void lexc_add_state(struct states *s) {
    struct statelist *sl;    
    sl = xxmalloc(sizeof(struct statelist));
    sl->state = s;
    s->number = -1;
    sl->next = statelist;
    sl->start = 0;
    sl->final = 0;
    statelist = sl;
    lexc_statecount++;
}

/* Go through the net built so far and add new transitions for @ */
/* to reflect the new symbols we now have in sigma */
/* We should really build a fast lookup ptr for finding the @ transitions */
/* But who in their right mind is ever going to use lots of @ in a lexicon construction? */
/* Of course this only applies to the special construct < regex > inside lexicon entries */
/* since @ is impossible to produce otherwise */

void lexc_update_unknowns(int sigma_number) {
    struct statelist *s;
    struct trans *t, *newtrans;
    for (s = statelist; s != NULL; s = s->next) {
        if (s->state->mergeable == 2)
            continue;
        for (t=s->state->trans ; t!=NULL; t= t->next) {
            if (t->in == IDENTITY || t->out == IDENTITY) {
                newtrans = xxmalloc(sizeof(struct trans));
                newtrans->in = sigma_number;
                newtrans->out = sigma_number;
                newtrans->target = t->target;
                newtrans->next = t->next;
                t->next = newtrans;
                }
        }
    }   
}

void lexc_add_network() {

    struct fsm *net;
    struct fsm_state *fsm;
    struct sigma *sigma;
    struct states **slist, *sourcestate, *deststate, *newstate;
    struct statelist *s;
    struct trans *newtrans;
    int i, j, *sigreplace, signumber, maxstate, *finals, unknown_symbols, first_new_sigma, *unk = NULL;

    unknown_symbols = 0;
    first_new_sigma = 0;
    sourcestate = clexicon->state;
    deststate = ctarget->state;

    net = current_regex_network;
    fsm = net->states;

    sigreplace = xxcalloc(sigma_max(net->sigma)+1,sizeof(int));

    for (sigma = net->sigma; sigma != NULL && sigma->number != -1; sigma = sigma->next) {
        if ((signumber = lexc_find_sigma_hash(sigma->symbol)) == -1) {
            /* Add to existing lexc sigma */
            signumber = sigma_add(sigma->symbol, lexsigma);
            first_new_sigma = first_new_sigma > 0 ? first_new_sigma : signumber;
            lexc_add_sigma_hash(sigma->symbol, signumber);
            *(sigreplace+sigma->number) = signumber;
        } else {
            /* We already have it, add to conversion table */
            *(sigreplace+sigma->number) = signumber;
        }
    }

    /* Renum arcs */
    for (i=0, maxstate = 0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->in != -1)
            (fsm+i)->in = *(sigreplace+(fsm+i)->in);
        if ((fsm+i)->out != -1)
            (fsm+i)->out = *(sigreplace+(fsm+i)->out);
        maxstate = (fsm+i)->state_no > maxstate ? (fsm+i)->state_no : maxstate;
        if ((fsm+i)->in == IDENTITY || (fsm+i)->in == UNKNOWN || (fsm+i)->out == UNKNOWN)
            unknown_symbols = 1;
    }
    if (unknown_symbols == 1) {
        unk = xxcalloc(sigma_max(lexsigma)+2,sizeof(int));
        for (i=0, sigma = lexsigma; sigma != NULL && sigma->number != -1; sigma=sigma->next) {
            if (sigma->number > 2 && sigma_find(sigma->symbol, net->sigma) == -1) {
                *(unk+i) = sigma->number;
                i++;
            }
        }
    }

    slist = xxcalloc(sizeof(**slist),maxstate+1);
    finals = xxcalloc(sizeof(int),maxstate+1);

    for (i=0; i <= maxstate;i++) {
        newstate = xxmalloc(sizeof(struct states));
        *(slist+i) = newstate;
        newstate->trans = NULL;
        newstate->lexstate = NULL;
        newstate->number = -1;
        newstate->hashval = -1;
        newstate->mergeable = 0;
        newstate->distance = 0;
        newstate->merge_with = newstate;
        s = xxmalloc(sizeof(struct statelist));
        s->state = newstate;
        s->next = statelist;
        s->start = 0;
        s->final = 0;
        statelist = s;
    }
    /* Add an EPSILON transition from sourcestate to state 0 */
    newtrans = xxmalloc(sizeof(struct trans));
    newtrans->in = EPSILON;
    newtrans->out = EPSILON;
    newtrans->target = *slist;
    newtrans->next = sourcestate->trans;
    sourcestate->trans = newtrans;

    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1) {
            newstate = *(slist+(fsm+i)->state_no);
            newtrans = xxmalloc(sizeof(struct trans));
            newtrans->in = (fsm+i)->in;
            newtrans->out = (fsm+i)->out;
            newtrans->target = *(slist+(fsm+i)->target);
            newtrans->next = newstate->trans;
            newstate->trans = newtrans;
            /* Add new symbols for @:@ transitions */
            /* TODO: make this work for ?: or :? trans as well */
            if (unknown_symbols == 1) {
                if ((fsm+i)->in == IDENTITY || (fsm+i)->out == IDENTITY) {
                    for (j=0; *(unk+j) != 0; j++) {
                        newtrans = xxmalloc(sizeof(struct trans));
                        newtrans->in = *(unk+j);
                        newtrans->out = *(unk+j);
                        newtrans->target = *(slist+(fsm+i)->target);
                        newtrans->next = newstate->trans;
                        newstate->trans = newtrans;
                    }
                }
            }
        }
        finals[(fsm+i)->state_no] = (fsm+i)->final_state;
    }
    /* Add an EPSILON transition from all final states to deststate */
    for (i=0; i <= maxstate; i++) {
        if (finals[i] == 1) {
            newtrans = xxmalloc(sizeof(struct trans));
            newtrans->in = newtrans->out = EPSILON;
            newtrans->target = deststate;
            newstate = *(slist+i);
            newtrans->next = newstate->trans;
            newstate->trans = newtrans;
        }
    }
    if (unknown_symbols == 1) {
        xxfree(unk);
        net_has_unknown = 1;
    }
    xxfree(slist);
    xxfree(finals);
}

void lexc_set_network(struct fsm *net) {
    current_regex_network = net;
    current_entry = REGEX_ENTRY;
    return;
}

void lexc_set_current_lexicon(char *name, int which) {
    /* Sets the global lexicon variable to point to a new lexicon */
    /* the variable which = 0 indicates source, which = 1 indicated target */

    struct lexstates *l;
    struct states *newstate;

    for (l = lexstates; l != NULL; l = l->next) {
        if (strcmp(name,l->name) == 0) {
            if (which == 0) {
		l->has_outgoing = 1;
                clexicon = l;
	    } else {
                ctarget = l;
	    }
            return;
        }
    }
    l = xxmalloc(sizeof(struct lexstates));
    l->next = lexstates;
    l->name = xxstrdup(name);
    l->has_outgoing = 0;
    l->targeted = 0;
    lexstates = l;
    newstate = xxmalloc(sizeof(struct states));
    lexc_add_state(newstate);
    newstate->lexstate = l;
    newstate->trans = NULL;
    newstate->mergeable = 0;
    newstate->merge_with = newstate;
    l->state = newstate;
    if (which == 0) {
        clexicon = l;
	l->has_outgoing = 1;
    } else { 
        ctarget = l;
    }
}

char *lexc_find_delim(char *name, char delimiter, char escape) {
    int i;
    for (i=0; *(name+i) != '\0'; i++) {
	if (*(name+i) == escape && *(name+i+1) != '\0') {
	    i++;
	    continue;
	}
        if (*(name+i) == delimiter) {
            return name+i;
        }
    }
    return NULL;
}

void lexc_deescape_string(char *name, char escape, int mode) {
    int i, j;
    for (i=0, j=0; *(name+i) != '\0'; i++) {
        *(name+j) = *(name+i);
        if (*(name+i) == escape) {
            *(name+j) = *(name+i+1);
            j++;
            i++;
            continue;
        }
	else if (mode == 1 && *(name+i) == '0') {
	    /* Marks alignment EPSILON */
	    *(name+j) = (unsigned char) 0xff;
	    j++;
	    continue;
	}
        else if (*(name+i) != escape && *(name+i) != '0') {
            j++;
            continue;
        }
    }
    *(name+j) = '\0';
}

/* Read a string and fill cwordin, cwordout arrays */
/* with the sigma numbers of the current word, -1 terminated */

void lexc_set_current_word(char *name) {
    char *instring, *outstring;
    int i;

    carity = 1;
    instring = name;
    outstring = lexc_find_delim(name,':','%');
    /* printf("CWin: [%s] CWout: [%s]\n", instring, outstring); */
    if (outstring != NULL) {
        *outstring = '\0';
        outstring = outstring+1;
        lexc_deescape_string(outstring,'%',1);
        carity = 2;
    }
    lexc_deescape_string(instring, '%',1);
    /* printf("CWin2: [%s] CWout2: [%s]\n", instring, outstring); */
    lexc_string_to_tokens(instring, cwordin);
    if (carity == 2) {
        lexc_string_to_tokens(outstring, cwordout);
	if (g_lexc_align)
	    lexc_medpad();
	else
	    lexc_pad();
    } else {
        for (i=0; *(cwordin+i) != -1; i++) {
            *(cwordout+i) = *(cwordin+i);
        }
        *(cwordout+i) = -1;

    }
    current_entry = WORD_ENTRY;
}


#define LEV_DOWN 0
#define LEV_LEFT 1
#define LEV_DIAG 2
    
void lexc_medpad() {
    int i, j, x, y, s1len, s2len, left, down, diag, dir;

    if (*cwordin == -1 && *cwordout == -1) {
	*cwordin = *cwordout = EPSILON;
	*(cwordin+1) = *(cwordout+1) = -1;
	return;
    }
    
    for (i = 0, j = 0; cwordin[i] != -1; i++) {
    	if (cwordin[i] == EPSILON) {
    	    continue;
    	}
    	cwordin[j] = cwordin[i];
    	j++;
    }
    cwordin[j] = -1;

    for (i = 0, j = 0; cwordout[i] != -1; i++) {
    	if (cwordout[i] == EPSILON) {
    	    continue;
    	}
    	cwordout[j] = cwordout[i];
    	j++;
    }
    cwordout[j] = -1;
    
    for (i = 0; cwordin[i] != -1; i++) { }
    s1len = i;
    for (i = 0; cwordout[i] != -1; i++) { }
    s2len = i;

    // MSVC does not like dynammic size arrays
    //int matrix[s1len+2][s2len+2];
    //int dirmatrix[s1len+2][s2len+2];
    int **matrix = (int **) calloc(s1len+2,sizeof(int*));
    int **dirmatrix = (int **) calloc(s1len+2,sizeof(int*));
    for (i=0; i<s1len+2; ++i) {
      matrix[i] = (int *) calloc(s2len+2,sizeof(int));
      dirmatrix[i] = (int *) calloc(s2len+2,sizeof(int));
    }

    matrix[0][0] = 0;
    dirmatrix[0][0] = 0;
    for (x = 1; x <= s1len; x++) {
        matrix[x][0] = matrix[x-1][0] + 1;
	dirmatrix[x][0] = LEV_LEFT;
    }
    for (y = 1; y <= s2len; y++) {
        matrix[0][y] = matrix[0][y-1] + 1;
	dirmatrix[0][y] = LEV_DOWN;
    }
    for (x = 1; x <= s1len; x++) {
        for (y = 1; y <= s2len; y++) {
    	    diag = matrix[x-1][y-1] + (cwordin[x-1] == cwordout[y-1] ? 0 : 100);
    	    down =  matrix[x][y-1] + 1;
    	    left = matrix[x-1][y] + 1;
    	    if (diag <= left && diag <= down) {
    		matrix[x][y] = diag;
    		dirmatrix[x][y] = LEV_DIAG;
    	    } else if (left <= diag && left <= down) {
    		matrix[x][y] = left;
    		dirmatrix[x][y] = LEV_LEFT;
    	    } else {
    		matrix[x][y] = down ;
    		dirmatrix[x][y] = LEV_DOWN;
    	    }
    	}
    }

    for (x = s1len, y = s2len, i = 0; (x > 0) || (y > 0); i++) {
	dir = dirmatrix[x][y];
    	if (dir == LEV_DIAG) {
    	    medcwordin[i] = cwordin[x-1];
    	    medcwordout[i] = cwordout[y-1];
    	    x--;
    	    y--;
    	}
    	else if (dir == LEV_DOWN) {
    	    medcwordin[i] = EPSILON;
    	    medcwordout[i] = cwordout[y-1];
    	    y--;
    	}
    	else {
    	    medcwordin[i] = cwordin[x-1];
	    medcwordout[i] = EPSILON;
    	    x--;
    	}
    }
    for (j = 0, i-= 1; i >= 0; j++, i--) {
    	cwordin[j] = medcwordin[i];
    	cwordout[j] = medcwordout[i];
    }
    cwordin[j] = -1;
    cwordout[j] = -1;

    // free allocated space (because MSVC can't handle dynammic arrays =8-O )
    for (i=0; i<s1len+2; ++i) {
      free(matrix[i]);
      free(dirmatrix[i]);
    }
    free(matrix);
    free(dirmatrix);
}

void lexc_pad() {
    int i, pad;
    /* Pad the shorter of current in, out words in cwordin, cwordout with EPSILON */

    if (*cwordin == -1 && *cwordout == -1) {
	*cwordin = *cwordout = EPSILON;
	*(cwordin+1) = *(cwordout+1) = -1;
	return;
    }

    for (i=0, pad = 0; ;i++) {
        if (pad == 1 && *(cwordout+i) == -1) {
            *(cwordin+i) = -1;
            break;
        }
        if (pad == 2 && *(cwordin+i) == -1) {
            *(cwordout+i) = -1;
            break;
        }
        if (*(cwordin+i) == -1 && *(cwordout+i) != -1) {
            pad = 1; /* Pad upper */ 
        }
        else if (*(cwordin+i) != -1 && *(cwordout+i) == -1) {
            pad = 2; /* Pad lower */
        }
        if (pad == 1) {
            *(cwordin+i) = EPSILON;
        }
        if (pad == 2) {
            *(cwordout+i) = EPSILON;
        }
        if (pad == 0 && *(cwordin+i) == -1)
            break;
    }
}

void lexc_string_to_tokens(char *string, int *intarr) {
    int len, i, pos, skip, signumber, multi;
    unsigned int mchashval;
    char tmpstring[5];
    struct multichar_symbols *mcs;
    len = strlen(string);
    for (i=0, pos = 0; i < len; ) {

	/* EPSILON for alignment is marked as 0xff */
	if ((unsigned char) string[i] == 0xff) {
	    *(intarr+pos) = EPSILON;
	    pos++;
	    i++;
	    continue;
	}

        multi = 0;
        mchashval = (unsigned int) ((unsigned char) *(string+i)) * 256 + (unsigned int) ((unsigned char) *(string+i+1));
        if ((i < len-1) && *(mchash+mchashval) == 1) {
            for (mcs = mc; mcs != NULL; mcs = mcs->next) {
                if (strncmp(string+i,mcs->symbol,strlen(mcs->symbol)) == 0) {
                    /* printf("Found multichar: [%s][%i]\n",mcs->symbol,mcs->sigma_number); */
                    multi = 1;
                    break;
                }
            }
        }

        if (multi) {
            *(intarr+pos) = mcs->sigma_number;
            pos++;
            i += strlen(mcs->symbol);
        } else {
            skip = utf8skip(string+i);
            if ((signumber = lexc_find_sigma_hash(mystrncpy(tmpstring,string+i,skip+1))) != -1) {
                *(intarr+pos) = signumber;
                pos++;
                i = i + skip + 1;
            } else {
                signumber = sigma_add(mystrncpy(tmpstring, string+i, skip+1), lexsigma);
                lexc_add_sigma_hash(tmpstring, signumber);
                *(intarr+pos) = signumber;
                pos++;
                i = i + skip + 1;
            }
        }
    }
    *(intarr+pos) = -1;
}

char *mystrncpy(char *dest, char *src, int len) {
    int i;
    for (i=0; i < len; i++) {
        *(dest+i) = *(src+i);
        if (*(src+i) == '\0')
            return(dest);
    }
    *(dest+i) = '\0';
/*     printf("Mystrncpy: [%s]\n",dest); */
    return(dest);
}

/* Add MC to front of chain */
/* In decreasing order of length */

void lexc_add_mc(char *symbol) {
    int s, len;
    unsigned int mchashval;
    struct multichar_symbols *mcs, *mcprev, *mcnew;
    lexc_deescape_string(symbol,'%',0);
    if (!lexc_find_mc(symbol)) {
        len = utf8strlen(symbol);
        mcprev = NULL;
        for (mcs = mc; mcs != NULL && utf8strlen(mcs->symbol) > len; mcprev = mcs, mcs=mcs->next) {
        }
        mcnew = xxmalloc(sizeof(struct multichar_symbols));
        mcnew->symbol = xxstrdup(symbol);
        mcnew->next = mcs;
        if ((mc == NULL) ||(mcs != NULL && mcprev == NULL))
            mc = mcnew;
        if (mcprev != NULL)
            mcprev->next = mcnew;
        
        s = sigma_add(symbol, lexsigma);
        mchashval = (unsigned int) ((unsigned char) *(symbol)) * 256 + (unsigned int) ((unsigned char) *(symbol+1));    
        lexc_add_sigma_hash(symbol, s);
        *(mchash+mchashval) = 1;
        mcnew->sigma_number = s;
    }
}

int lexc_find_mc(char *symbol) {
    struct multichar_symbols *mcs;
    for (mcs = mc ; mcs != NULL ; mcs = mcs->next) {
        if (strcmp(symbol,mcs->symbol) == 0)
            return 1;
    }
    return 0;
}

struct states *lexc_find_lex_state(char *name) {
    struct lexstates *l;
    for (l = lexstates ; l != NULL; l = l->next) {
        if (strcmp(name,l->name) == 0)
            return (l->state);
    }
    return NULL;
}

void lexc_add_word() {
    /** Add a word from source state to destination state */
    struct trans *newtrans, *trans;
    struct states *sourcestate, *deststate, *newstate;
    int i, follow, len;

    if (current_entry == REGEX_ENTRY) {
        lexc_add_network();
        return;
    }
            
    /* find source, dest */
    sourcestate = clexicon->state;
    deststate = ctarget->state;

    for (i=0; *(cwordin+i) != -1; i++) {}
    len = i;
    maxlen = len > maxlen ? len : maxlen;
    
    /* We follow the source state if the symbols are the same */
    /* To merge prefixes */
    for (follow = 1, i=0; *(cwordin+i) != -1; i++) {
        
        if (follow == 1) {
            for (trans = sourcestate->trans; trans != NULL ; trans = trans->next) {
                if (trans->in == *(cwordin+i) && trans->out == *(cwordout+i) && trans->target->lexstate == NULL) {
                    /* Can't follow if target needs to be lexstate */
                    if (*(cwordin+i+1) == -1 && trans->target != deststate) {
                        continue;
                    }
                    sourcestate = trans->target;
                    sourcestate->mergeable = 0;
                    /* Breakout */
                    goto breakout;
                }
            }
        }
        follow = 0;

        newtrans = xxmalloc(sizeof(struct trans));
        if (*(cwordin+i+1) == -1) {
            newtrans->target = deststate;
        } else {
            newstate = xxmalloc(sizeof(struct states));
            lexc_add_state(newstate);
            newtrans->target = newstate;
            newstate->trans = NULL;
            newstate->lexstate = NULL;
            newstate->mergeable = 1;
            newstate->hashval = lexc_suffix_hash(i+1);
            newstate->distance = len - i - 1;
            newstate->merge_with = newstate;
        }
        newtrans->next = sourcestate->trans;
        sourcestate->trans = newtrans;

        newtrans->in = *(cwordin+i);
        newtrans->out = *(cwordout+i);

        sourcestate = newtrans->target;
    breakout:;
        
    }
    return;
}

void lexc_number_states() {
    int n, smax, hasroot;
    struct statelist *s;
    struct lexstates *l;

    smax = n = hasfinal = 0;

    for (hasroot = 0, s = statelist; s != NULL; s = s->next) {
        smax++;
        if (s->state->lexstate != NULL && strcmp(s->state->lexstate->name, "Root") == 0) {
            s->state->number = 0;
            s->start = 1;
            n++;
            hasroot = 1;
            break;
        }
    }
    /* If there is no Root lexicon, the first lexicon mentioned is Root */
    if (!hasroot) {
        for (s = statelist; s != NULL; s = s->next) {        
            if (s->next == NULL) {
                s->state->number = 0;
                fprintf(stderr,"*Warning: no Root lexicon, using '%s' as Root.\n",s->state->lexstate->name);
                s->start = 1;
                n++;
            }
        }
    }
    /* Mark # as the last state */
    for (s = statelist; s != NULL; s = s->next) {
        if (s->state->lexstate != NULL && strcmp(s->state->lexstate->name, "#") == 0) {
            s->state->number = smax-1;
            s->final = 1;
            hasfinal = 1;
        } else if (s->state->lexstate != NULL && strcmp(s->state->lexstate->name, "#") != 0 && s->state->lexstate->has_outgoing == 0) {
	    /* Also mark uncontinued states as final (this is warned about elsewhere) */
            s->final = 1;
	}
    }

    for (s = statelist; s != NULL; s = s->next) { 
        if (s->state->number == -1) {
            s->state->number = n;
            n++;
        }
    }
    lexc_statecount = n+1;
    for (l = lexstates; l != NULL ; l = l->next) {
        if (l->targeted == 0 && l->state->number != 0) {
	    fprintf(stderr,"*Warning: lexicon '%s' defined but not used\n",l->name);
            fflush(stdout);
        }
        if (l->has_outgoing == 0 && strcmp(l->name, "#") != 0) {
	    fprintf(stderr,"***Warning: lexicon '%s' used but never defined\n",l->name);
            fflush(stdout);
        }
    }
}

int lexc_eq_paths(struct states *one, struct states *two) {
    while (one->lexstate == NULL && two->lexstate == NULL) {
        if (one->trans->in != two->trans->in || one->trans->out != two->trans->out)
            return 0;
        one = one->trans->target;
        two = two->trans->target;
    }
    if (one->lexstate != two->lexstate)
        return 0;
    return 1;
}

void lexc_merge_states() {
    struct lenlist {
        struct states *state;
        struct lenlist *next;
    };
    struct hashstates {
        struct states *state;
        struct hashstates *next;
    } *hashstates, *currenth, *newh;

    struct lenlist *lenlist, *newl, *currentl;
    struct statelist *s, *sprev, *sf;
    struct states *state, *purgestate;
    struct trans *t, *tprev;
    int i, numstates, tablesize, hash;

    /* Create array of ptrs to states depending on string length */
    lenlist = xxcalloc(maxlen+1,sizeof(struct lenlist));
    numstates = 0;
    for (s = statelist ; s!= NULL; s = s->next) {
        if (s->state->mergeable)
            numstates++;
    }

    /* Find a suitable prime for hashing: proportional to the size of the */
    /* number of mergeable states */

    for (i = 0; primes[i] < numstates/4; i++) { }    
    tablesize = primes[i];
    hashstates = xxcalloc(tablesize,sizeof(struct hashstates));

    for (s = statelist ; s!= NULL; s = s->next) {
        if (s->state->mergeable) {
            numstates++;
            currentl = lenlist+(s->state->distance);
            if (currentl->state == NULL)
                currentl->state = s->state;
            else {
                newl = xxcalloc(1,sizeof(struct lenlist));
                newl->state = s->state;
                newl->next = currentl->next;
                currentl->next = newl;
            }           
            s->state->hashval = s->state->hashval % tablesize;
            currenth = hashstates+s->state->hashval;
            if (currenth->state == NULL) {
                currenth->state = s->state;
            } else {
                newh = xxcalloc(1,sizeof(struct hashstates));
                newh->state = s->state;
                newh->next = currenth->next;
                currenth->next = newh; 
            }
        }
    }
    
    for (i = maxlen; i >= 1 ; i--) {
        /* printf("Analyzing: [%i]...",i); fflush(stdout); */
        for (currentl = (lenlist+i); currentl != NULL; currentl = currentl->next) {
            if (currentl->state == NULL)
                break;
            if (currentl->state->mergeable != 1)
                continue;
            /* Find states hashing to same value as current */
            state = currentl->state;
            hash = state->hashval;
            for (currenth = hashstates+hash; currenth != NULL; currenth = currenth->next) {
                /* Merge */
                if (currenth->state != state && currenth->state->mergeable == 1 && currenth->state->distance == state->distance && lexc_eq_paths(currenth->state,state)) {
                    currenth->state->merge_with = state;
                    for (purgestate = currenth->state; purgestate->lexstate == NULL; purgestate = purgestate->trans->target) {
                        purgestate->mergeable = 2;
                    }
                }
            }
        }
    }

    /* Go through statelist and remove merged states and free states, trans */
    
    for (s = statelist, sprev = NULL; s != NULL; s = s->next) {
        for (t = s->state->trans, tprev = NULL; t != NULL; tprev = t, t = t->next) {
            t->target = t->target->merge_with;
            if (tprev != NULL && s->state->mergeable == 2) {
                xxfree(tprev);
            } else {
                if (t->target->lexstate != NULL)
                    t->target->lexstate->targeted = 1;
            }
        }
        if (tprev != NULL && s->state->mergeable == 2)
            xxfree(tprev);
    }
    for (s = statelist, sprev = NULL; s != NULL; ) {
        if (s->state->mergeable == 2) {
            if (sprev != NULL) {
                sprev->next = s->next;                
            } else {
                statelist = s;
            }
            xxfree(s->state);
            sf = s;
            s = s->next;
            xxfree(sf);
        } else {
            sprev = s;
            s = s ->next;
        }
    }

    /* Cleanup */

    for (i = 0; i < maxlen ; i++) {
        newl = NULL;
        for (currentl = (lenlist+i)->next; currentl != NULL ;currentl=currentl->next) {
            if (newl != NULL)
                xxfree(newl);
            newl = currentl;
        }
        if (newl != NULL)
            xxfree(newl);
    }
    for (i = 0; i < tablesize ; i++) {
        newh = NULL;
        for (currenth = (hashstates+i)->next; currenth != NULL ;currenth=currenth->next) {
            if (newh != NULL)
                xxfree(newh);
            newh = currenth;
        }
        if (newh != NULL)
            xxfree(newh);
    }
    xxfree(hashstates);
    xxfree(lenlist);
}

struct fsm *lexc_to_fsm() {
    struct statelist *s, *sa;
    struct fsm_state *fsm;
    struct fsm *net;
    struct trans *t;
    int i, j,  linecount;

    fprintf(stderr,"Building lexicon...\n");
    fflush(stdout);
    lexc_merge_states();
    net = fsm_create("");
    xxfree(net->sigma);
    net->sigma = lexsigma;
    lexc_number_states();
    if (hasfinal == 0) {
        fprintf(stderr,"Warning: # is never reached!!!\n");
        return(fsm_empty_set());
    }
    sa = xxmalloc(sizeof(struct statelist)*lexc_statecount);
    for (s = statelist; s != NULL; s = s->next) {
        sa[s->state->number].state = s->state;
        sa[s->state->number].start = s->start;
        sa[s->state->number].final = s->final;
    }
    linecount = 0;
    for (s = statelist; s != NULL; s = s->next) {
        linecount++;
        for (t = s->state->trans; t != NULL; t = t->next)
            linecount++;
    }
    fsm = xxmalloc(sizeof(struct fsm_state)*(linecount+1));
    for (i = 0, j = 0, s = sa; j < lexc_statecount; j++) {
        if (s[j].state->trans == NULL) {
            add_fsm_arc(fsm,i,s[j].state->number, -1, -1, -1, s[j].final, s[j].start);
            i++;
        } else {
            for (t = s[j].state->trans; t != NULL; t = t->next) {
                add_fsm_arc(fsm,i,s[j].state->number,t->in,t->out,t->target->number,s[j].final,s[j].start);
                i++;
            }
        }
    }
    add_fsm_arc(fsm, i, -1, -1, -1, -1, -1, -1);
    net->states = fsm;
    net->statecount = lexc_statecount;
    fsm_update_flags(net, UNK, UNK, UNK, UNK, UNK, UNK);
    if (sigma_find_number(EPSILON, lexsigma) == -1)
        sigma_add_special(EPSILON, lexsigma);
    xxfree(s);
    lexc_cleanup();
    sigma_cleanup(net,0);
    sigma_sort(net);
    
    fprintf(stderr,"Determinizing...\n");
    fflush(stdout);
    net = fsm_determinize(net);
    fprintf(stderr,"Minimizing...\n");
    fflush(stdout);
    net = fsm_topsort(fsm_minimize(net));
    fprintf(stderr,"Done!\n");
    return(net);
}

void lexc_cleanup() {
    struct lexstates *l, *ln;
    struct statelist *s, *sn;
    struct trans *t, *tn;
    struct multichar_symbols *mcs, *mcsn;
    struct lexc_hashtable *lhash, *lprev;
    int i;
    xxfree(mchash);
    for (i=0; i < SIGMA_HASH_TABLESIZE; i++) {
        for (lhash = hashtable+i; lhash != NULL; ) {
            if (lhash->symbol != NULL) {
                xxfree(lhash->symbol);
            }
            lprev = lhash;
            lhash = lhash->next;
            if (lprev != hashtable+i) { xxfree(lprev); }
        }
    }
    xxfree(hashtable);
    for (mcs = mc ; mcs != NULL ; mcs = mcsn) {
        mcsn = mcs->next;
	xxfree(mcs->symbol);
        xxfree(mcs);
    }
    for (l = lexstates ; l != NULL ; l = ln) {
        ln = l->next;
        xxfree(l->name);
        xxfree(l);
    }
    for (s = statelist; s != NULL; s = s->next) {
        for (t = s->state->trans; t != NULL; t = tn) {
            tn = t->next;
            xxfree(t);
        }
        xxfree(s->state);
    }
    for (s = statelist; s != NULL; s = sn) {
        sn = s->next;
        xxfree(s);
    }
}
