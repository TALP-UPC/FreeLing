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
#include "foma.h"

#define INITIAL_SIZE 16384
#define SIGMA_HASH_SIZE 1021
#define MINSIGMA 3

struct foma_reserved_symbols {
    char *symbol;
    int number;
    char *prints_as;
} foma_reserved_symbols[] = {
    {"@_EPSILON_SYMBOL_@" , EPSILON , "0"},
    {"@_UNKNOWN_SYMBOL_@" , UNKNOWN , "?"},
    {"@_IDENTITY_SYMBOL_@", IDENTITY, "@"},
    {NULL,0,NULL}
};

static size_t current_fsm_size;
static unsigned int current_fsm_linecount, current_state_no, current_final, current_start, current_trans, num_finals, num_initials, arity, statecount;
static _Bool is_deterministic, is_epsilon_free;
static struct fsm_state *current_fsm_head;

static unsigned int mainloop, ssize, arccount;

struct sigma_lookup {
    int target;
    unsigned int mainloop;
};

static struct sigma_lookup *slookup;

/* Functions for directly building a fsm_state structure */
/* dynamically. */

/* fsm_state_init() is called when a new machine is constructed */

/* fsm_state_add_arc() adds an arc and possibly reallocs the array */

/* fsm_state_close() adds the sentinel entry and clears values */

struct fsm_state *fsm_state_init(int sigma_size) {
    current_fsm_head = xxmalloc(INITIAL_SIZE * sizeof(struct fsm_state));
    current_fsm_size = INITIAL_SIZE;
    current_fsm_linecount = 0;
    ssize = sigma_size+1;
    slookup = xxcalloc(ssize*ssize,sizeof(struct sigma_lookup));
    mainloop = 1;
    is_deterministic = 1;
    is_epsilon_free = 1;
    arccount = 0;
    num_finals = 0;
    num_initials = 0;
    statecount = 0;
    arity = 1;
    current_trans = 1;
    return(current_fsm_head);
}

void fsm_state_set_current_state(int state_no, int final_state, int start_state) {
    current_state_no = state_no;
    current_final = final_state;
    current_start = start_state;
    current_trans = 0;
    if (current_final == 1)
        num_finals++;
    if (current_start == 1)
	num_initials++;
}

/* Add sentinel if needed */
void fsm_state_end_state() {
    if (current_trans == 0) {
        fsm_state_add_arc(current_state_no, -1, -1, -1, current_final, current_start);
    }
    statecount++;
    mainloop++;
}

void fsm_state_add_arc(int state_no, int in, int out, int target, int final_state, int start_state) {
    struct fsm_state *cptr;
    
    if (in != out) {
        arity = 2;
    }
    /* Check epsilon moves */
    if (in == EPSILON && out == EPSILON) {
        if (state_no == target) {
            return;
        } else {
            is_deterministic = 0;
            is_epsilon_free = 0;
        }
    }

    /* Check if we already added this particular arc and skip */
    /* Also check if net becomes non-det */
    if (in != -1 && out != -1) {
        if ((slookup+(ssize*in)+out)->mainloop == mainloop) {
            if ((slookup+(ssize*in)+out)->target == target) {
	        return;
            } else {
	        is_deterministic = 0;
            }
        }
        arccount++;
        (slookup+(ssize*in)+out)->mainloop = mainloop;
        (slookup+(ssize*in)+out)->target = target;
    }
    
    current_trans = 1;
    if (current_fsm_linecount >= current_fsm_size) {
        current_fsm_size *= 2;
        current_fsm_head = xxrealloc(current_fsm_head, current_fsm_size * sizeof(struct fsm_state));
        if (current_fsm_head == NULL) {
            perror("Fatal error: out of memory\n");
            exit(1);
        }
    }
    cptr = current_fsm_head + current_fsm_linecount;
    cptr->state_no = state_no;
    cptr->in = in;
    cptr->out = out;
    cptr->target = target;
    cptr->final_state = final_state;
    cptr->start_state = start_state;    
    current_fsm_linecount++;
}

void fsm_state_close(struct fsm *net) {
    fsm_state_add_arc(-1,-1,-1,-1,-1,-1);
    current_fsm_head = xxrealloc(current_fsm_head, current_fsm_linecount * sizeof(struct fsm_state));
    net->arity = arity;
    net->arccount = arccount;
    net->statecount = statecount;
    net->linecount = current_fsm_linecount;
    net->finalcount = num_finals;
    net->pathcount = PATHCOUNT_UNKNOWN;
    if (num_initials > 1)
	is_deterministic = 0;
    net->is_deterministic = is_deterministic;
    net->is_pruned = UNK;
    net->is_minimized = UNK;
    net->is_epsilon_free = is_epsilon_free;
    net->is_loop_free = UNK;
    net->is_completed = UNK;
    net->arcs_sorted_in = 0;
    net->arcs_sorted_out = 0;

    net->states = current_fsm_head;
    xxfree(slookup);
}

/* Construction functions */

struct fsm_construct_handle *fsm_construct_init(char *name) {
    struct fsm_construct_handle *handle;
    handle = xxmalloc(sizeof(struct fsm_construct_handle));
    handle->fsm_state_list = xxcalloc(1024,sizeof(struct fsm_state_list));
    handle->fsm_state_list_size = 1024;
    handle->fsm_sigma_list = xxcalloc(1024,sizeof(struct fsm_sigma_list));
    handle->fsm_sigma_list_size = 1024;
    handle->fsm_sigma_hash = xxcalloc(SIGMA_HASH_SIZE,sizeof(struct fsm_sigma_hash));
    handle->maxstate = -1;
    handle->maxsigma = -1;
    handle->numfinals = 0;
    if (name == NULL) {
        handle->name = NULL;
    } else {
        handle->name = xxstrdup(name);
    }
    handle->hasinitial = 0;
    return(handle);
}

void fsm_construct_check_size(struct fsm_construct_handle *handle, int state_no) {
    int i, oldsize, newsize;
    struct fsm_state_list *sl;
    oldsize = handle->fsm_state_list_size;
    if (oldsize <= state_no) {
        newsize = next_power_of_two(state_no);      
        handle->fsm_state_list = xxrealloc(handle->fsm_state_list, newsize*sizeof(struct fsm_state_list));
        handle->fsm_state_list_size = newsize;
        sl = handle->fsm_state_list;
        for (i=oldsize; i<newsize;i++) {
            (sl+i)->is_final = 0;
            (sl+i)->is_initial = 0;
            (sl+i)->used = 0;
            (sl+i)->num_trans = 0;
            (sl+i)->fsm_trans_list = NULL;
        }
    }
}

void fsm_construct_set_final(struct fsm_construct_handle *handle, int state_no) {
    struct fsm_state_list *sl;
    fsm_construct_check_size(handle, state_no);

    if (state_no > handle->maxstate)
        handle->maxstate = state_no;

    sl = handle->fsm_state_list;
    if (!(sl+state_no)->is_final) {
        (sl+state_no)->is_final = 1;
        handle->numfinals++;
    }
}

void fsm_construct_set_initial(struct fsm_construct_handle *handle, int state_no) {
    struct fsm_state_list *sl;
    fsm_construct_check_size(handle, state_no);

    if (state_no > handle->maxstate)
        handle->maxstate = state_no;

    sl = handle->fsm_state_list;
    (sl+state_no)->is_initial = 1;
    handle->hasinitial = 1;
}

void fsm_construct_add_arc(struct fsm_construct_handle *handle, int source, int target, char *in, char *out) {
    struct fsm_state_list *sl;
    struct fsm_trans_list *tl;
    int symin, symout;
    fsm_construct_check_size(handle, source);
    fsm_construct_check_size(handle, target);

    if (source > handle->maxstate)
        handle->maxstate = source;
    if (target > handle->maxstate)
        handle->maxstate = target;

    sl = (handle->fsm_state_list)+target;
    sl->used = 1;
    sl = (handle->fsm_state_list)+source;
    sl->used = 1;
    tl = xxmalloc(sizeof(struct fsm_trans_list));
    tl->next = sl->fsm_trans_list;
    sl->fsm_trans_list = tl;
    if ((symin = fsm_construct_check_symbol(handle,in)) == -1)
        symin = fsm_construct_add_symbol(handle,in);
    if ((symout = fsm_construct_check_symbol(handle,out)) == -1)
        symout = fsm_construct_add_symbol(handle,out);
    tl->in = symin;
    tl->out = symout;
    tl->target = target;
}

unsigned int fsm_construct_hash_sym(char *symbol) {
    register unsigned int hash;
    hash = 0;
    
    while (*symbol != '\0')
        hash = hash +  *symbol++;
    return (hash % SIGMA_HASH_SIZE);
}

void fsm_construct_add_arc_nums(struct fsm_construct_handle *handle, int source, int target, int in, int out) {
    struct fsm_state_list *sl;
    struct fsm_trans_list *tl;
    fsm_construct_check_size(handle, source);
    fsm_construct_check_size(handle, target);

    if (source > handle->maxstate)
        handle->maxstate = source;
    if (target > handle->maxstate)
        handle->maxstate = target;

    sl = (handle->fsm_state_list)+target;
    sl->used = 1;
    sl = (handle->fsm_state_list)+source;
    sl->used = 1;
    tl = xxmalloc(sizeof(struct fsm_trans_list));
    tl->next = sl->fsm_trans_list;
    sl->fsm_trans_list = tl;
    tl->in = in;
    tl->out = out;
    tl->target = target;
}

/* Copies entire alphabet from existing network */

void fsm_construct_copy_sigma(struct fsm_construct_handle *handle, struct sigma *sigma) {

    unsigned int hash;
    int symnum;
    struct fsm_sigma_hash *fh, *newfh;
    char *symbol, *symdup;

    for (; sigma != NULL && sigma->number != -1; sigma = sigma->next) {
	symnum = sigma->number;
	if (symnum > handle->maxsigma) {
	    handle->maxsigma = symnum;
	}
	symbol = sigma->symbol;
	if (symnum >= handle->fsm_sigma_list_size) {
	    handle->fsm_sigma_list_size = next_power_of_two(handle->fsm_sigma_list_size);
	    handle->fsm_sigma_list = xxrealloc(handle->fsm_sigma_list, (handle->fsm_sigma_list_size) * sizeof(struct fsm_sigma_list));
	}
	/* Insert into list */
	symdup = xxstrdup(symbol);
	((handle->fsm_sigma_list)+symnum)->symbol = symdup;
	
	/* Insert into hashtable */
	hash = fsm_construct_hash_sym(symbol);
	fh = (handle->fsm_sigma_hash)+hash;   
	if (fh->symbol == NULL) {
	    fh->symbol = symdup;
	    fh->sym = symnum;        
	} else {
	    newfh = xxcalloc(1,sizeof(struct fsm_sigma_hash));
	    newfh->next = fh->next;
	    fh->next = newfh;
	    newfh->symbol = symdup;
	    newfh->sym = symnum;
	}
    }
}

int fsm_construct_add_symbol(struct fsm_construct_handle *handle, char *symbol) {
    int i, symnum, reserved;
    unsigned int hash;
    struct fsm_sigma_hash *fh, *newfh;
    char *symdup;

    /* Is symbol reserved? */
    for (i=0, reserved = 0; foma_reserved_symbols[i].symbol != NULL; i++) {
        if (strcmp(symbol, foma_reserved_symbols[i].symbol) == 0) {
            symnum = foma_reserved_symbols[i].number;
            reserved = 1;
            if (handle->maxsigma < symnum) {
                handle->maxsigma = symnum;
            }
            break;
        }
    }

    if (reserved == 0) {
        symnum = handle->maxsigma + 1;
        if (symnum < MINSIGMA)
            symnum = MINSIGMA;
        handle->maxsigma = symnum;
    }

    if (symnum >= handle->fsm_sigma_list_size) {
        handle->fsm_sigma_list_size = next_power_of_two(handle->fsm_sigma_list_size);
        handle->fsm_sigma_list = xxrealloc(handle->fsm_sigma_list, (handle->fsm_sigma_list_size) * sizeof(struct fsm_sigma_list));
    }
    /* Insert into list */
    symdup = xxstrdup(symbol);
    ((handle->fsm_sigma_list)+symnum)->symbol = symdup;

    /* Insert into hashtable */
    hash = fsm_construct_hash_sym(symbol);
    fh = (handle->fsm_sigma_hash)+hash;   
    if (fh->symbol == NULL) {
        fh->symbol = symdup;
        fh->sym = symnum;        
    } else {
        newfh = xxcalloc(1,sizeof(struct fsm_sigma_hash));
        newfh->next = fh->next;
        fh->next = newfh;
        newfh->symbol = symdup;
        newfh->sym = symnum;
    }
    return symnum;
}

int fsm_construct_check_symbol(struct fsm_construct_handle *handle, char *symbol) {
    int hash;
    struct fsm_sigma_hash *fh;
    hash = fsm_construct_hash_sym(symbol);
    fh = (handle->fsm_sigma_hash)+hash;
    if (fh->symbol == NULL)
        return -1;
    for (; fh != NULL; fh = fh->next) {
        if (strcmp(symbol,fh->symbol) == 0) {
            return (fh->sym);
        }
    }
    return -1;
}

struct sigma *fsm_construct_convert_sigma(struct fsm_construct_handle *handle) {
    struct fsm_sigma_list *sl;
    struct sigma *sigma, *oldsigma, *newsigma;
    int i;
    oldsigma = sigma = NULL;
    sl = handle->fsm_sigma_list;
    for (i=0; i <= handle->maxsigma; i++) {
        if ((sl+i)->symbol != NULL) {
            newsigma = xxmalloc(sizeof(struct sigma));
            newsigma->number = i;
            newsigma->symbol = (sl+i)->symbol;
            newsigma->next = NULL;
            if (oldsigma != NULL) {
                oldsigma->next = newsigma;
            } else {
                sigma = newsigma;
            }
            oldsigma = newsigma;
        }
    }
    return(sigma);
}

struct fsm *fsm_construct_done(struct fsm_construct_handle *handle) {
    int i, emptyfsm;
    struct fsm *net;
    struct fsm_state_list *sl;
    struct fsm_trans_list *trans, *transnext;
    struct fsm_sigma_hash *sigmahash, *sigmahashnext;

    sl = handle->fsm_state_list;
    if (handle->maxstate == -1 || handle->numfinals == 0 || handle->hasinitial == 0) {
        return(fsm_empty_set());
    }
    fsm_state_init((handle->maxsigma)+1);

    for (i=0, emptyfsm = 1; i <= handle->maxstate; i++) {
        fsm_state_set_current_state(i, (sl+i)->is_final, (sl+i)->is_initial);
	if ((sl+i)->is_initial && (sl+i)->is_final)
	    emptyfsm = 0; /* We want to keep track of if FSM has (a) something outgoing from initial, or (b) initial is final */
        for (trans = (sl+i)->fsm_trans_list; trans != NULL; trans = trans->next) {
	    if ((sl+i)->is_initial)
		emptyfsm = 0;
            fsm_state_add_arc(i, trans->in, trans->out, trans->target, (sl+i)->is_final, (sl+i)->is_initial);
        }
        fsm_state_end_state();
    }
    net = fsm_create("");
    sprintf(net->name, "%X",rand());
    xxfree(net->sigma);
    fsm_state_close(net);
    
    net->sigma = fsm_construct_convert_sigma(handle);
    if (handle->name != NULL) {        
        strncpy(net->name, handle->name, 40);
        xxfree(handle->name);
    } else {
        sprintf(net->name, "%X",rand());
    }

    /* Free transitions */
    for (i=0; i < handle->fsm_state_list_size; i++) {
        trans = (((handle->fsm_state_list)+i)->fsm_trans_list);
        while (trans != NULL) {
            transnext = trans->next;
            xxfree(trans);
            trans = transnext;
        }
    }
    /* Free hash table */
    for (i=0; i < SIGMA_HASH_SIZE; i++) {
        sigmahash = (((handle->fsm_sigma_hash)+i)->next);
        while (sigmahash != NULL) {
            sigmahashnext = sigmahash->next;
            xxfree(sigmahash);
            sigmahash = sigmahashnext;
        }
    }
    xxfree(handle->fsm_sigma_list);
    xxfree(handle->fsm_sigma_hash);
    xxfree(handle->fsm_state_list);
    xxfree(handle);
    sigma_sort(net);
    if (emptyfsm) {
	fsm_destroy(net);
	return(fsm_empty_set());
    }
    return(net);
}

/* Reading functions */

int fsm_read_is_final(struct fsm_read_handle *h, int state) {
    return (*((h->lookuptable)+state) & 2);
}

int fsm_read_is_initial(struct fsm_read_handle *h, int state) {
    return (*((h->lookuptable)+state) & 1);
}

struct fsm_read_handle *fsm_read_init(struct fsm *net) {
    struct fsm_read_handle *handle;
    struct fsm_state *fsm, **states_head;
    int i, j, k, num_states, num_initials, num_finals, sno, *finals_head, *initials_head, laststate;

    unsigned char *lookuptable;
    if (net == NULL) {return (NULL);}

    num_states = net->statecount;
    lookuptable = xxcalloc(num_states, sizeof(unsigned char));
    
    num_initials = num_finals = 0;

    handle = xxcalloc(1,sizeof(struct fsm_read_handle));
    states_head = xxcalloc(num_states+1,sizeof(struct fsm **));

    laststate = -1;
    for (i=0, fsm=net->states; (fsm+i)->state_no != -1; i++) {
        sno = (fsm+i)->state_no;
        if ((fsm+i)->start_state) {
            if (!(*(lookuptable+sno) & 1)) {
                *(lookuptable+sno) |= 1;
                num_initials++;
            }
            
        }
        if ((fsm+i)->final_state) {
            if (!(*(lookuptable+sno) & 2)) {
                *(lookuptable+sno) |= 2;
                num_finals++;
            }
        }
	if ((fsm+i)->in == UNKNOWN || (fsm+i)->out == UNKNOWN || (fsm+i)->in == IDENTITY || (fsm+i)->out == IDENTITY) {
	    handle->has_unknowns = 1;
	}
	if ((fsm+i)->state_no != laststate) {
	    *(states_head+(fsm+i)->state_no) = fsm+i;
	}
	laststate = (fsm+i)->state_no;
    }
    
    finals_head = xxcalloc(num_finals+1,sizeof(int));
    initials_head = xxcalloc(num_initials+1,sizeof(int));


    for (i=j=k=0; i < num_states; i++) {
        if (*(lookuptable+i) & 1) {
            *(initials_head+j) = i;
            j++;
        }
        if (*(lookuptable+i) & 2) {
            *(finals_head+k) = i;
            k++;
        }
    }
    *(initials_head+j) = -1;
    *(finals_head+k) = -1;
    
    handle->finals_head = finals_head;
    handle->initials_head = initials_head;
    handle->states_head = states_head;
    
    handle->fsm_sigma_list = sigma_to_list(net->sigma);
    handle->sigma_list_size = sigma_max(net->sigma)+1;
    handle->arcs_head = fsm;
    handle->lookuptable = lookuptable;
    handle->net = net;
    return(handle);
}

void fsm_read_reset(struct fsm_read_handle *handle) {
    if (handle == NULL)
	return;
    handle->arcs_cursor = NULL;
    handle->initials_cursor = NULL;
    handle->finals_cursor = NULL;
    handle->states_cursor = NULL;
}

int fsm_get_next_state_arc(struct fsm_read_handle *handle) {
    handle->arcs_cursor++;
    if ((handle->arcs_cursor->state_no != handle->current_state) || (handle->arcs_cursor->target == -1)) {
	handle->arcs_cursor--;
	return 0;
    }
    return 1;
}

int fsm_get_next_arc(struct fsm_read_handle *handle) {
    if (handle->arcs_cursor == NULL) {
        handle->arcs_cursor = handle->arcs_head;
        while (handle->arcs_cursor->state_no != -1 && handle->arcs_cursor->target == -1) {
            handle->arcs_cursor++;
        }
        if (handle->arcs_cursor->state_no == -1) {
            return 0;
        }
    } else {
        if (handle->arcs_cursor->state_no == -1) {
            return 0;
        }
        do {
            handle->arcs_cursor++;
        } while (handle->arcs_cursor->state_no != -1 && handle->arcs_cursor->target == -1);
        if (handle->arcs_cursor->state_no == -1) {
            return 0;
        }
    }
    return 1;
}

int fsm_get_arc_source(struct fsm_read_handle *handle) {
    if (handle->arcs_cursor == NULL) { return -1;}
    return(handle->arcs_cursor->state_no);
}

int fsm_get_arc_target(struct fsm_read_handle *handle) {
    if (handle->arcs_cursor == NULL) { return -1;}    
    return(handle->arcs_cursor->target);
}

int fsm_get_symbol_number(struct fsm_read_handle *handle, char *symbol) {
    int i;
    for (i=0; i < handle->sigma_list_size; i++) {
	if ((handle->fsm_sigma_list+i)->symbol == NULL)
	    continue;
	if (strcmp(symbol,  (handle->fsm_sigma_list+i)->symbol) == 0) {
	    return i;
	}
    }
    return -1;
}

char *fsm_get_arc_in(struct fsm_read_handle *handle) {
    int index;
    char *sym;
    if (handle->arcs_cursor == NULL) { return NULL;}
    index = handle->arcs_cursor->in;
    sym = (handle->fsm_sigma_list+index)->symbol;
    return(sym);
}

int fsm_get_arc_num_in(struct fsm_read_handle *handle) {
    if (handle->arcs_cursor == NULL) { return -1;}    
    return(handle->arcs_cursor->in);
}

int fsm_get_arc_num_out(struct fsm_read_handle *handle) {
    if (handle->arcs_cursor == NULL) { return -1;}
    return(handle->arcs_cursor->out);
}

char *fsm_get_arc_out(struct fsm_read_handle *handle) {
    int index;
    char *sym;
    if (handle->arcs_cursor == NULL) { return NULL; }
    index = handle->arcs_cursor->out;
    sym = (handle->fsm_sigma_list+index)->symbol;
    return(sym);
}

int fsm_get_next_initial(struct fsm_read_handle *handle) {
    if (handle->initials_cursor == NULL) {
        handle->initials_cursor = handle->initials_head;
    } else {
        if (*(handle->initials_cursor) == -1) {
            return -1;
        }
        handle->initials_cursor++;
    }
    return *(handle->initials_cursor);
}

int fsm_get_next_final(struct fsm_read_handle *handle) {
    if (handle->finals_cursor == NULL) {
        handle->finals_cursor = handle->finals_head;
    } else {
        if (*(handle->finals_cursor) == -1) {
            return -1;
        }
        handle->finals_cursor++;
    }
    return *(handle->finals_cursor);
}

int fsm_get_num_states(struct fsm_read_handle *handle) {
    return(handle->net->statecount);
}

int fsm_get_has_unknowns(struct fsm_read_handle *handle) {
    return(handle->has_unknowns);
}

int fsm_get_next_state(struct fsm_read_handle *handle) {
    int stateno;
    if (handle->states_cursor == NULL) {
        handle->states_cursor = handle->states_head;
    } else {
	handle->states_cursor++;
    }
    if (handle->states_cursor - handle->states_head >= fsm_get_num_states(handle)) {
	return -1;
    }
    handle->arcs_cursor = *(handle->states_cursor);
    stateno = (*(handle->states_cursor))->state_no;
    handle->arcs_cursor--;
    handle->current_state = stateno;
    return (stateno);
}

void fsm_read_done(struct fsm_read_handle *handle) {
    xxfree(handle->lookuptable);
    xxfree(handle->fsm_sigma_list);
    xxfree(handle->finals_head);
    xxfree(handle->initials_head);
    xxfree(handle->states_head);
    xxfree(handle);
}
