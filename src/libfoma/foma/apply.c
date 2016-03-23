/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2015 Mans Hulden                                     */

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
#include <time.h>
#include <string.h>
#include <limits.h>
#include "foma.h"

#define RANDOM 1
#define ENUMERATE 2
#define MATCH 4
#define UP 8
#define DOWN 16
#define LOWER 32
#define UPPER 64
#define SPACE 128

#define FAIL 0
#define SUCCEED 1

#define DEFAULT_OUTSTRING_SIZE 4096
#define DEFAULT_STACK_SIZE 128

#define APPLY_BINSEARCH_THRESHOLD 10

#define BITMASK(b) (1 << ((b) & 7))
#define BITSLOT(b) ((b) >> 3)
#define BITSET(a,b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a,b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a,b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)



static int apply_append(struct apply_handle *h, int cptr, int sym);
static char *apply_net(struct apply_handle *h);
static void apply_create_statemap(struct apply_handle *h,struct fsm *net);
static void apply_create_sigarray(struct apply_handle *h,struct fsm *net);
static void apply_create_sigmatch(struct apply_handle *h);
int apply_match_length(struct apply_handle *h, int symbol);
static int apply_match_str(struct apply_handle *h,int symbol, int position);
static void apply_add_flag(struct apply_handle *h,char *name);
static int apply_check_flag(struct apply_handle *h,int type, char *name, char *value);
static void apply_clear_flags(struct apply_handle *h);
void apply_set_iptr(struct apply_handle *h);
void apply_mark_flagstates(struct apply_handle *h);
void apply_clear_index(struct apply_handle *h);

static void apply_stack_clear(struct apply_handle *h);
static int apply_stack_isempty(struct apply_handle *h);
static void apply_stack_pop (struct apply_handle *h);
static void apply_stack_push (struct apply_handle *h, int vmark, char *sflagname, char *sflagvalue, int sflagneg);
static void apply_force_clear_stack(struct apply_handle *h);


void apply_set_obey_flags(struct apply_handle *h, int value) {
    h->obey_flags = value;
}

void apply_set_show_flags(struct apply_handle *h, int value) {
    h->show_flags = value;
}

void apply_set_print_space(struct apply_handle *h, int value) {
    h->print_space = value;
    h->space_symbol = strdup(" ");
}

void apply_set_separator(struct apply_handle *h, char *symbol) {
    h->separator = strdup(symbol);
}

void apply_set_epsilon(struct apply_handle *h, char *symbol) {
    xxfree(h->epsilon_symbol);
    h->epsilon_symbol = strdup(symbol);
    (h->sigs+EPSILON)->symbol = h->epsilon_symbol;
    (h->sigs+EPSILON)->length =  strlen(h->epsilon_symbol);
}

void apply_set_space_symbol(struct apply_handle *h, char *space) {
    h->space_symbol = strdup(space);
    h->print_space = 1;
}

void apply_set_print_pairs(struct apply_handle *h, int value) {
    h->print_pairs = value;
}

static void apply_force_clear_stack(struct apply_handle *h) {
    /* Make sure stack is empty and marks reset */
    if (!apply_stack_isempty(h)) {
	*(h->marks+(h->gstates+h->ptr)->state_no) = 0;
	while (!apply_stack_isempty(h)) {
	    apply_stack_pop(h);
	    *(h->marks+(h->gstates+h->ptr)->state_no) = 0;
	}
	h->iterator = 0;
	h->iterate_old = 0;
	apply_stack_clear(h);
    }
}

char *apply_enumerate(struct apply_handle *h) {

    char *result = NULL;
    
    if (h->last_net == NULL || h->last_net->finalcount == 0) {
	return (NULL);
    }
    h->binsearch = 0;
    if (h->iterator == 0) {
        h->iterate_old = 0;
	apply_force_clear_stack(h);
        result = apply_net(h);
	if ((h->mode & RANDOM) != RANDOM)
	  (h->iterator)++;
    } else {
        h->iterate_old = 1;
        result = apply_net(h);
    }
    return(result);
}

char *apply_words(struct apply_handle *h) {
    h->mode = DOWN + ENUMERATE + LOWER + UPPER;
    return(apply_enumerate(h));
}

char *apply_upper_words(struct apply_handle *h) {
    h->mode = DOWN + ENUMERATE + UPPER;
    return(apply_enumerate(h));
}

char *apply_lower_words(struct apply_handle *h) {
    h->mode = DOWN + ENUMERATE + LOWER;
    return(apply_enumerate(h));
}

char *apply_random_words(struct apply_handle *h) {
    apply_clear_flags(h);
    h->mode = DOWN + ENUMERATE + LOWER + UPPER + RANDOM;
    return(apply_enumerate(h));
}

char *apply_random_lower(struct apply_handle *h) {
    apply_clear_flags(h);
    h->mode = DOWN + ENUMERATE + LOWER + RANDOM;    
    return(apply_enumerate(h));
}

char *apply_random_upper(struct apply_handle *h) {
    apply_clear_flags(h);
    h->mode = DOWN + ENUMERATE + UPPER + RANDOM;
    return(apply_enumerate(h));
}

/* Frees memory associated with applies */
void apply_clear(struct apply_handle *h) {
    struct sigma_trie_arrays *sta, *stap;
    for (sta = h->sigma_trie_arrays; sta != NULL; ) {
	stap = sta;
	xxfree(sta->arr);
	sta = sta->next;
	xxfree(stap);
    }
    h->sigma_trie_arrays = NULL;
    if (h->statemap != NULL) {
        xxfree(h->statemap);
        h->statemap = NULL;
    }
    if (h->numlines != NULL) {
        xxfree(h->numlines);
        h->numlines = NULL;
    }
    if (h->marks != NULL) {
        xxfree(h->marks);
        h->marks = NULL;
    }
    if (h->searchstack != NULL) {
        xxfree(h->searchstack);
        h->searchstack = NULL;
    }
    if (h->sigs != NULL) {
        xxfree(h->sigs);
        h->sigs = NULL;
    }
    if (h->flag_lookup != NULL) {
        xxfree(h->flag_lookup);
        h->flag_lookup = NULL;
    }
    if (h->sigmatch_array != NULL) {
	xxfree(h->sigmatch_array);
	h->sigmatch_array = NULL;
    }
    if (h->flagstates != NULL) {
	xxfree(h->flagstates);
	h->flagstates = NULL;
    }    
    apply_clear_index(h);
    h->last_net = NULL;
    h->iterator = 0;
    xxfree(h->outstring);
    xxfree(h->separator);
    xxfree(h->epsilon_symbol);
    xxfree(h);
}

char *apply_updown(struct apply_handle *h, char *word) {

    char *result = NULL;

    if (h->last_net == NULL || h->last_net->finalcount == 0)
        return (NULL);
    
    if (word == NULL) {
        h->iterate_old = 1;
        result = apply_net(h);
    }
    else if (word != NULL) {
        h->iterate_old = 0;
        h->instring = word;
        apply_create_sigmatch(h);

	/* Remove old marks if necessary TODO: only pop marks */
	apply_force_clear_stack(h);
        result = apply_net(h);
    }
    return(result);
}

char *apply_down(struct apply_handle *h, char *word) {
    
    h->mode = DOWN;
    if (h->index_in) { 
	h->indexed = 1;
    } else {
	h->indexed = 0;
    }
    h->binsearch = (h->last_net->arcs_sorted_in == 1) ? 1 : 0;
    return(apply_updown(h, word));
}

char *apply_up(struct apply_handle *h, char *word) {

    h->mode = UP;
    if (h->index_out) {
	h->indexed = 1;
    } else {
	h->indexed = 0;
    }
    h->binsearch = (h->last_net->arcs_sorted_out == 1) ? 1 : 0;
    return(apply_updown(h, word));
}

struct apply_handle *apply_init(struct fsm *net) {
    struct apply_handle *h;

    srand((unsigned int) time(NULL));
    h = calloc(1,sizeof(struct apply_handle));
    /* Init */

    h->iterate_old = 0;
    h->iterator = 0;
    h->instring = NULL;
    h->flag_list = NULL;
    h->flag_lookup = NULL;
    h->obey_flags = 1;
    h->show_flags = 0;
    h->print_space = 0;
    h->print_pairs = 0;
    h->separator = strdup(":");
    h->epsilon_symbol = strdup("0");
    h->last_net = net;
    h->outstring = xxmalloc(sizeof(char)*DEFAULT_OUTSTRING_SIZE);
    h->outstringtop = DEFAULT_OUTSTRING_SIZE;
    *(h->outstring) = '\0';
    h->gstates = net->states;
    h->gsigma = net->sigma;
    h->printcount = 1;
    apply_create_statemap(h, net);
    h->searchstack = xxmalloc(sizeof(struct searchstack) * DEFAULT_STACK_SIZE);
    h->apply_stack_top = DEFAULT_STACK_SIZE;
    apply_stack_clear(h);
    apply_create_sigarray(h, net);
    return(h);
}

int apply_stack_isempty (struct apply_handle *h) {
    if (h->apply_stack_ptr == 0) {
	return 1;
    }
    return 0;
}

void apply_stack_clear (struct apply_handle *h) {
    h->apply_stack_ptr = 0;
}

void apply_stack_pop (struct apply_handle *h) {
    struct flag_list *flist;
    struct searchstack *ss;
    (h->apply_stack_ptr)--;
    ss = h->searchstack+h->apply_stack_ptr;

    h->iptr =  ss->iptr;
    h->ptr  =  ss->offset;
    h->ipos =  ss->ipos;
    h->opos =  ss->opos;
    h->state_has_index = ss->state_has_index;
    /* Restore mark */
    *(h->marks+(h->gstates+h->ptr)->state_no) = ss->visitmark;

    if (h->has_flags && ss->flagname != NULL) {
	/* Restore flag */
	for (flist = h->flag_list; flist != NULL; flist = flist->next) {
	    if (strcmp(flist->name, ss->flagname) == 0) {
		break;
	    }
	}
	if (flist == NULL)
	    perror("***Nothing to pop\n");
	flist->value = ss->flagvalue;
	flist->neg = ss->flagneg;
    }
}

static void apply_stack_push (struct apply_handle *h, int vmark, char *sflagname, char *sflagvalue, int sflagneg) {
    struct searchstack *ss;
    if (h->apply_stack_ptr == h->apply_stack_top) {
	h->searchstack = xxrealloc(h->searchstack, sizeof(struct searchstack)* ((h->apply_stack_top)*2));
	if (h->searchstack == NULL) {
	  perror("Apply stack full!!!\n");
	  exit(0);
	}
	h->apply_stack_top *= 2;
    }
    ss = h->searchstack+h->apply_stack_ptr;
    ss->offset     = h->curr_ptr;
    ss->ipos       = h->ipos;
    ss->opos       = h->opos;
    ss->visitmark  = vmark;
    ss->iptr       = h->iptr;
    ss->state_has_index = h->state_has_index;
    if (h->has_flags) {
	ss->flagname   = sflagname;
	ss->flagvalue  = sflagvalue;
	ss->flagneg    = sflagneg;
    }
    (h->apply_stack_ptr)++;
}

void apply_reset_enumerator(struct apply_handle *h) {
    int statecount, i;
    statecount = h->last_net->statecount;
    for (i=0; i < statecount; i++) {
	*(h->marks+i) = 0;
    }
    h->iterator = 0;
    h->iterate_old = 0;
}

void apply_clear_index_list(struct apply_handle *h, struct apply_state_index **index) {
    int i, j, statecount;
    struct apply_state_index *iptr, *iptr_tmp, *iptr_zero;
    if (index == NULL)
	return;
    statecount = h->last_net->statecount;
    for (i = 0; i < statecount; i++) {
	iptr = *(index+i);
	if (iptr == NULL) {
	    continue;
	}
	iptr_zero = *(index+i);
	for (j = h->sigma_size - 1 ; j >= 0; j--) { /* Make sure to not free the list in EPSILON    */
	    iptr = *(index+i) + j;                  /* as the other states lists' tails point to it */
	    for (iptr = iptr->next ; iptr != NULL && iptr != iptr_zero; iptr = iptr_tmp) {
		iptr_tmp = iptr->next;
		xxfree(iptr);
	    }
	}
	xxfree(*(index+i));
    }
}

void apply_clear_index(struct apply_handle *h) {
    if (h->index_in) {
	apply_clear_index_list(h, h->index_in);
	xxfree(h->index_in);
	h->index_in = NULL;
    }
    if (h->index_out) {
	apply_clear_index_list(h, h->index_out);
	xxfree(h->index_out);
	h->index_out = NULL;
    }
}

void apply_index(struct apply_handle *h, int inout, int densitycutoff, int mem_limit, int flags_only) {
    struct fsm_state *fsm;
    unsigned int cnt = 0;
    int i, j, maxtrans, numtrans, laststate, sym;
    fsm = h->gstates;

    struct apply_state_index **indexptr, *iptr, *tempiptr;

    struct pre_index {
	int state_no;
	struct pre_index *next;
    } *pre_index, *tp, *tpp;
    if (flags_only && !h->has_flags) {
	return;
    }
    /* get numtrans */
    for (i=0, laststate = 0, maxtrans = 0, numtrans = 0; (fsm+i)->state_no != -1; i++) {
	if ((fsm+i)->state_no != laststate) {
	    maxtrans = numtrans > maxtrans ? numtrans : maxtrans;
	    numtrans = 0;
	}
	if ((fsm+i)->target != -1) {
	    numtrans++;
	}
	laststate = (fsm+i)->state_no;
    }

    pre_index = xxcalloc(maxtrans+1, sizeof(struct pre_index));
    for (i = 0; i <= maxtrans; i++) {
	(pre_index+i)->state_no = -1;
    }

    /* We create an array of states, indexed by how many transitions they have */
    /* so that later, we can traverse them in order densest first, in case we  */
    /* only want to index to some predefined maximum memory usage.             */

    for (i = 0, laststate = 0, maxtrans = 0, numtrans = 0; (fsm+i)->state_no != -1; i++) {
	if ((fsm+i)->state_no != laststate) {
	    if ((pre_index+numtrans)->state_no == -1) {
		(pre_index+numtrans)->state_no = laststate;
	    } else {
		tp = xxcalloc(1, sizeof(struct pre_index));
		tp->state_no = laststate;
		tp->next = (pre_index+numtrans)->next;
		(pre_index+numtrans)->next = tp;
	    }
	    maxtrans = numtrans > maxtrans ? numtrans : maxtrans;
	    numtrans = 0;
	}
	if ((fsm+i)->target != -1) {
	    numtrans++;
	}
	laststate = (fsm+i)->state_no;
    }
    indexptr = NULL;
    cnt += round_up_to_power_of_two(h->last_net->statecount*sizeof(struct apply_state_index *));

    if (cnt > mem_limit) {
	cnt -= round_up_to_power_of_two(h->last_net->statecount*sizeof(struct apply_state_index *));
	goto memlimitnoindex;
    }

    indexptr = xxcalloc(h->last_net->statecount, sizeof(struct apply_state_index *));

    if (h->has_flags && flags_only) {
	/* Mark states that have flags */
	if (!(h->flagstates)) {
	    apply_mark_flagstates(h);
	}
    }

    for (i = maxtrans; i >= 0; i--) {
	for (tp = pre_index+i; tp != NULL; tp = tp->next) {
	    if (tp->state_no >= 0) {
		if (i < densitycutoff) {
		    if (!(h->has_flags && flags_only && BITTEST(h->flagstates, tp->state_no))) {
			continue;
		    }
		}
		cnt += round_up_to_power_of_two(h->sigma_size*sizeof(struct apply_state_index));
		if (cnt > mem_limit) {
		    cnt -= round_up_to_power_of_two(h->sigma_size*sizeof(struct apply_state_index));
		    goto memlimit;
		}
		*(indexptr + tp->state_no) = xxmalloc(h->sigma_size*sizeof(struct apply_state_index));

		/* We make the tail of all index linked lists point to the index  */
		/* for EPSILON, so that we automatically when EPSILON transitions */
		/* also when traversing an index.                                 */

		for (j = 0; j < h->sigma_size; j++) {
		    (*(indexptr + tp->state_no) + j)->fsmptr = -1;
		    if (j == EPSILON)
			(*(indexptr + tp->state_no) + j)->next = NULL;
		    else
			(*(indexptr + tp->state_no) + j)->next = (*(indexptr + tp->state_no)); /* all tails point to epsilon */		    
		}
	    }
	}
    }

 memlimit:

    for (i=0; (fsm+i)->state_no != -1; i++) {
	iptr = *(indexptr + (fsm+i)->state_no);
	if (iptr == NULL || (fsm+i)->target == -1) {
	    continue;
	}
	sym = inout == APPLY_INDEX_INPUT ? (fsm+i)->in : (fsm+i)->out;

	if (h->has_flags && (h->flag_lookup+sym)->type) {
	    sym = EPSILON;
	}
	if (sym == UNKNOWN) {  /* We make the index of UNKNOWN point to IDENTITY */
	    sym = IDENTITY;    /* since these are really the same symbol         */
	}
	if ((iptr+sym)->fsmptr == -1) {
	    (iptr+sym)->fsmptr = i;
	} else {
	    cnt += round_up_to_power_of_two(sizeof(struct apply_state_index));
	    tempiptr = xxcalloc(1, sizeof(struct apply_state_index));

	    tempiptr->next = (iptr+sym)->next;
	    tempiptr->fsmptr =  i;
	    (iptr+sym)->next = tempiptr;
	}
    }

    /* Free preindex */

 memlimitnoindex:

    for (i = maxtrans; i >= 0; i--) {
	for (tp = (pre_index+i)->next; tp != NULL; tp = tpp) {
	    tpp = tp->next;
	    xxfree(tp);
	}
    }
    xxfree(pre_index);

    if (inout == APPLY_INDEX_INPUT) {
	h->index_in = indexptr;
    } else {
	h->index_out = indexptr;
    }
}

int apply_binarysearch(struct apply_handle *h) {
    int thisstate, nextsym, seeksym, thisptr, lastptr, midptr;

    thisptr = h->curr_ptr = h->ptr;
    nextsym  = (((h->mode) & DOWN) == DOWN) ? (h->gstates+h->curr_ptr)->in  : (h->gstates+h->curr_ptr)->out;
    if (nextsym == EPSILON)
	return 1;
    if (nextsym == -1)
	return 0;
    if (h->ipos >= h->current_instring_length) {
	return 0;
    }
    seeksym = (h->sigmatch_array+h->ipos)->signumber;
    if (seeksym == nextsym || (nextsym == UNKNOWN && seeksym == IDENTITY))
	return 1;

    thisstate = (h->gstates+thisptr)->state_no;
    lastptr = *(h->statemap+thisstate)+*(h->numlines+thisstate)-1;
    thisptr++;

    if (seeksym == IDENTITY || lastptr - thisptr < APPLY_BINSEARCH_THRESHOLD) {
	for ( ; thisptr <= lastptr; thisptr++) {
	    nextsym = (((h->mode) & DOWN) == DOWN) ? (h->gstates+thisptr)->in : (h->gstates+thisptr)->out;
	    if ((nextsym == seeksym) || (nextsym == UNKNOWN && seeksym == IDENTITY)) {
		h->curr_ptr = thisptr;
		return 1;
	    }
	    if (nextsym > seeksym || nextsym == -1) {
		return 0;
	    }
	}
	return 0;
    }
     
    for (;;)  {
	if (thisptr > lastptr) { return 0; }
	midptr = (thisptr+lastptr)/2;
	nextsym = (((h->mode) & DOWN) == DOWN) ? (h->gstates+midptr)->in : (h->gstates+midptr)->out;
	if (seeksym < nextsym) {
	    lastptr = midptr - 1;
	    continue;
	} else if (seeksym > nextsym) {
	    thisptr = midptr + 1;
	    continue;
	} else {

	    while (((((h->mode) & DOWN) == DOWN) ? (h->gstates+(midptr-1))->in : (h->gstates+(midptr-1))->out) == seeksym) {
		midptr--; /* Find first match in case of ties */
	    }
	    h->curr_ptr = midptr;
	    return 1;
	}
    }
}

int apply_follow_next_arc(struct apply_handle *h) {
    char *fname, *fvalue;
    int eatupi, eatupo, symin, symout, fneg;
    int vcount, marksource, marktarget;
    
    /* Here we follow three possible search strategies:        */
    /* (1) if the state in question has an index, we use that  */
    /* (2) if the state is binary searchable, we use that      */
    /* (3) otherwise we traverse arc-by-arc                    */
    /*     Condition (2) needs arcs to be sorted in the proper */
    /*     direction, and requires that the state be flag-free */
    /*     For those states that aren't flag-free, (3) is used */

    if (h->state_has_index) {
	for ( ; h->iptr != NULL && h->iptr->fsmptr != -1; ) {

	    h->ptr = h->curr_ptr = h->iptr->fsmptr;
	    if (((h->mode) & DOWN) == DOWN) {
		symin = (h->gstates+h->curr_ptr)->in;
		symout = (h->gstates+h->curr_ptr)->out;
	    } else {
		symin = (h->gstates+h->curr_ptr)->out;
		symout = (h->gstates+h->curr_ptr)->in;
	    }
	    
	    marksource = *(h->marks+(h->gstates+h->ptr)->state_no);
	    marktarget = *(h->marks+(h->gstates+(*(h->statemap+(h->gstates+h->curr_ptr)->target)))->state_no);
	    eatupi = apply_match_length(h, symin);
	    if (!(eatupi == -1 || -1-(h->ipos)-eatupi == marktarget)) {     /* input 2x EPSILON loop check */
		if ((eatupi = apply_match_str(h, symin, h->ipos)) != -1) {
		    eatupo = apply_append(h, h->curr_ptr, symout);
		    if (h->obey_flags && h->has_flags && ((h->flag_lookup+symin)->type & (FLAG_UNIFY|FLAG_CLEAR|FLAG_POSITIVE|FLAG_NEGATIVE))) {
			fname = (h->flag_lookup+symin)->name;
			fvalue = h->oldflagvalue;
			fneg = h->oldflagneg;
		    } else {
			fname = fvalue = NULL;
			fneg = 0;
		    }
		    /* Push old position */
		    apply_stack_push(h, marksource, fname, fvalue, fneg);
		    h->ptr = *(h->statemap+(h->gstates+h->curr_ptr)->target);
		    h->ipos += eatupi;
		    h->opos += eatupo;
		    apply_set_iptr(h);
		    return 1;
		}
	    }
	    h->iptr = h->iptr->next;
	}
	return 0;
    } else if ((h->binsearch && !(h->has_flags)) || (h->binsearch && !(BITTEST(h->flagstates, (h->gstates+h->ptr)->state_no)))) {
	for (;;) {
	    if (apply_binarysearch(h)) {
		if (((h->mode) & DOWN) == DOWN) {
		    symin = (h->gstates+h->curr_ptr)->in;
		    symout = (h->gstates+h->curr_ptr)->out;
		} else {
		    symin = (h->gstates+h->curr_ptr)->out;
		    symout = (h->gstates+h->curr_ptr)->in;
		}
		
		marksource = *(h->marks+(h->gstates+h->ptr)->state_no);
		marktarget = *(h->marks+(h->gstates+(*(h->statemap+(h->gstates+h->curr_ptr)->target)))->state_no);
		
		eatupi = apply_match_length(h, symin);
		if (eatupi != -1 && -1-(h->ipos)-eatupi != marktarget) {
		    if ((eatupi = apply_match_str(h, symin, h->ipos)) != -1) {
			eatupo = apply_append(h, h->curr_ptr, symout);
			
			/* Push old position */
			apply_stack_push(h, marksource, NULL, NULL, 0);
			
			/* Follow arc */
			h->ptr = *(h->statemap+(h->gstates+h->curr_ptr)->target);
			h->ipos += eatupi;
			h->opos += eatupo;
			apply_set_iptr(h);
			return 1;
		    }
		}
		if ((h->gstates+h->curr_ptr)->state_no == (h->gstates+h->curr_ptr+1)->state_no) {
		    h->curr_ptr++; 
		    h->ptr = h->curr_ptr;
		    if ((h->gstates+h->curr_ptr)-> target == -1) { 
			return 0; 
		    }
		    continue;
		}
	    }
	    return 0;
	}
    } else {
	for (h->curr_ptr = h->ptr; (h->gstates+h->curr_ptr)->state_no == (h->gstates+h->ptr)->state_no && (h->gstates+h->curr_ptr)-> in != -1; (h->curr_ptr)++) {
	    
	    /* Select one random arc to follow out of all outgoing arcs */	
	    if ((h->mode & RANDOM) == RANDOM) {
		vcount = 0;
		for (h->curr_ptr = h->ptr;  (h->gstates+h->curr_ptr)->state_no == (h->gstates+h->ptr)->state_no && (h->gstates+h->curr_ptr)-> in != -1; (h->curr_ptr)++) {
		    vcount++;
		}
		if (vcount > 0) {
		    h->curr_ptr = h->ptr + (rand() % vcount);
		} else {
		    h->curr_ptr = h->ptr;
		}
	    }
	    
	    if (((h->mode) & DOWN) == DOWN) {
		symin = (h->gstates+h->curr_ptr)->in;
		symout = (h->gstates+h->curr_ptr)->out;
	    } else {
		symin = (h->gstates+h->curr_ptr)->out;
		symout = (h->gstates+h->curr_ptr)->in;
	    }
	    
	    marksource = *(h->marks+(h->gstates+h->ptr)->state_no);
	    marktarget = *(h->marks+(h->gstates+(*(h->statemap+(h->gstates+h->curr_ptr)->target)))->state_no);

	    eatupi = apply_match_length(h, symin);

	    if (eatupi == -1 || -1-(h->ipos)-eatupi == marktarget) { continue; } /* loop check */
	    if ((eatupi = apply_match_str(h, symin, h->ipos)) != -1) {
		eatupo = apply_append(h, h->curr_ptr, symout);
		if (h->obey_flags && h->has_flags && ((h->flag_lookup+symin)->type & (FLAG_UNIFY|FLAG_CLEAR|FLAG_POSITIVE|FLAG_NEGATIVE))) {
		    
		    fname = (h->flag_lookup+symin)->name;
		    fvalue = h->oldflagvalue;
		    fneg = h->oldflagneg;
		} else {
		    fname = fvalue = NULL;
		    fneg = 0;
		}
		
		/* Push old position */
		apply_stack_push(h, marksource, fname, fvalue, fneg);
		
		/* Follow arc */
		h->ptr = *(h->statemap+(h->gstates+h->curr_ptr)->target);
		h->ipos += eatupi;
		h->opos += eatupo;
		apply_set_iptr(h);
		return(1);
	    }
	}
	return(0);
    }
}

char *apply_return_string(struct apply_handle *h) {
    /* Stick a 0 to endpos to avoid getting old accumulated gunk strings printed */
    *(h->outstring+h->opos) = '\0';
    if (((h->mode) & RANDOM) == RANDOM) {
	/* To end or not to end */
	if (!(rand() % 2)) {
	    apply_stack_clear(h);
	    h->iterator = 0;
	    h->iterate_old = 0;
	    return(h->outstring);
	}
    } else {
	return(h->outstring);
    }
    return(NULL);
}

void apply_mark_state(struct apply_handle *h) {

    /* This controls the how epsilon-loops are traversed.  Such loops can    */
    /* only be followed once to reach a state already visited in the DFS.    */
    /* This requires that we store the number of input symbols consumed      */
    /* whenever we enter a new state.  If we enter the same state twice      */
    /* with the same number of input symbols consumed, we abandon the search */
    /* for that branch. Flags are epsilons from this point of view.          */
    /* The encoding of h->marks is:                                          */
    /* 0 = unseen, +ipos = seen at ipos, -ipos = seen second time at ipos    */

    if ((h->mode & RANDOM) != RANDOM) {
	if (*(h->marks+(h->gstates+h->ptr)->state_no) == h->ipos+1) {
	    *(h->marks+(h->gstates+h->ptr)->state_no) = -(h->ipos+1);
	} else {
	    *(h->marks+(h->gstates+h->ptr)->state_no) = h->ipos+1;
	}
    }
}

void apply_skip_this_arc(struct apply_handle *h) {
    /* If we have index ptr */
    if (h->iptr) {
	h->ptr = h->iptr->fsmptr;
	h->iptr = h->iptr->next;
	/* Otherwise */
    } else {
	(h->ptr)++;
    }
}

int apply_at_last_arc(struct apply_handle *h) {
    int seeksym, nextsym;
    if (h->state_has_index) {
	if (h->iptr->next == NULL || h->iptr->next->fsmptr == -1) {
	    return 1;
	}
    } else {
	if  ((h->binsearch && !(h->has_flags)) || (h->binsearch && !(BITTEST(h->flagstates, (h->gstates+h->ptr)->state_no)))) {
	    if ((h->gstates+h->ptr)->state_no != (h->gstates+h->ptr+1)->state_no) {
		return 1;
	    }
	    seeksym = (h->sigmatch_array+h->ipos)->signumber;
	    nextsym  = (((h->mode) & DOWN) == DOWN) ? (h->gstates+h->ptr)->in  : (h->gstates+h->ptr)->out;
	    if (nextsym == -1 || seeksym < nextsym) {
		return 1;
	    }
	} else {
	    if ((h->gstates+h->ptr)->state_no != (h->gstates+h->ptr+1)->state_no) {
		return 1;
	    }
	}
    }
    return 0;
}

/* map h->ptr (line pointer) to h->iptr (index pointer) */
void apply_set_iptr(struct apply_handle *h) {
    struct apply_state_index **idx, *sidx;
    int stateno, seeksym;
    /* Check if state has index */
    if ((idx = ((h->mode) & DOWN) == DOWN ? (h->index_in) : (h->index_out)) == NULL) {
	return;
    }
 
    h->iptr = NULL;
    h->state_has_index = 0;
    stateno = (h->gstates+h->ptr)->state_no;
    if (stateno < 0) {
	return;
    }
   
    sidx = *(idx + stateno);
    if (sidx == NULL) { return; }
    seeksym = (h->sigmatch_array+h->ipos)->signumber;
    h->state_has_index = 1;
    sidx = sidx + seeksym;
    if (sidx->fsmptr == -1) {
	if (sidx->next == NULL) {
	    return;
	} else {
	    sidx = sidx->next;
	}
    }
    h->iptr = sidx;
    if (sidx->fsmptr == -1) {
	h->iptr = NULL;
    }
    h->state_has_index = 1;
}

char *apply_net(struct apply_handle *h) {

/*     We perform a basic DFS on the graph, with two minor complications:       */

/*     1. We keep a mark for each state which indicates how many input symbols  */
/*        we had consumed the last time we entered that state on the current    */
/*        "run."  If we reach a state seen twice without consuming input, we    */
/*        terminate that branch of the search.                                  */
/*        As we pop a position, we also unmark the state we came from.          */
 
/*     2. If the graph has flags, we push the previous flag value when          */
/*        traversing a flag-modifying arc (P,U,N, or C).  This is because a     */
/*        flag may have been set during the previous "run" and may not apply.   */
/*        Since we're doing a DFS, we can be sure to return to the previous     */
/*        global flag state by just remembering that last flag change.          */

/*     3. The whole system needs to work as an iterator, meaning we need to     */
/*        store the global state of the search so we can resume it later to     */
/*        to yield more possible output words with the same input string.       */

    char *returnstring;

    if (h->iterate_old == 1) {     /* If called with NULL as the input word, this will be set */
        goto resume;
    }

    h->iptr = NULL; h->ptr = 0; h->ipos = 0; h->opos = 0;
    apply_set_iptr(h);

    apply_stack_clear(h);

    if (h->has_flags) {
	apply_clear_flags(h);
    }
    
    /* "The use of four-letter words like goto can occasionally be justified */
    /*  even in the best of company." Knuth (1974).                          */

    goto L2;

    while(!apply_stack_isempty(h)) {
	apply_stack_pop(h);
	/* If last line was popped */
	if (apply_at_last_arc(h)) {
	    *(h->marks+(h->gstates+h->ptr)->state_no) = 0; /* Unmark   */
	    continue;                                      /* pop next */
	}
	apply_skip_this_arc(h);                            /* skip old pushed arc */
    L1:
	if (!apply_follow_next_arc(h)) {
	    *(h->marks+(h->gstates+h->ptr)->state_no) = 0; /* Unmark   */
	    continue;                                      /* pop next */
	}
    L2:
	/* Print accumulated string upon entry to state */
	if ((h->gstates+h->ptr)->final_state == 1 && (h->ipos == h->current_instring_length || ((h->mode) & ENUMERATE) == ENUMERATE)) {
	    if ((returnstring = (apply_return_string(h))) != NULL) {
		return(returnstring);
	    }
	}

    resume:
       	apply_mark_state(h);  /* Mark upon arrival to new state */
	goto L1;
    }
    if ((h->mode & RANDOM) == RANDOM) {
          apply_stack_clear(h);
          h->iterator = 0;
          h->iterate_old = 0;
          return(h->outstring);
    }
    apply_stack_clear(h);
    return NULL;
}

int apply_append(struct apply_handle *h, int cptr, int sym) {

    char *astring, *bstring, *pstring;
    int symin, symout, len, alen, blen, idlen;
    
    symin = (h->gstates+cptr)->in;
    symout = (h->gstates+cptr)->out;
    astring = ((h->sigs)+symin)->symbol;
    alen =  ((h->sigs)+symin)->length;
    bstring = ((h->sigs)+symout)->symbol;
    blen =  ((h->sigs)+symout)->length;
    
    while (alen + blen + h->opos + 2 + strlen(h->separator) >= h->outstringtop) {
	//    while (alen + blen + h->opos + 3 >= h->outstringtop) {
	h->outstring = xxrealloc(h->outstring, sizeof(char) * ((h->outstringtop) * 2));
	(h->outstringtop) *= 2;
    }
    
    if ((h->has_flags) && !h->show_flags && (h->flag_lookup+symin)->type) {
	astring = ""; alen = 0;
    }
    if (h->has_flags && !h->show_flags && (h->flag_lookup+symout)->type) {
	bstring = ""; blen = 0;
    }
    if (((h->mode) & ENUMERATE) == ENUMERATE) {
	/* Print both sides separated by colon */
	/* if we're printing "words" */
	if (((h->mode) & (UPPER | LOWER)) == (UPPER|LOWER)) {
	    
	    if (astring == bstring) {
		strcpy(h->outstring+h->opos, astring);
		len = alen;
	    } else {
		strcpy(h->outstring+h->opos, astring);
		//		strcpy(h->outstring+h->opos+alen,":");
		strcpy(h->outstring+h->opos+alen,h->separator);
		//strcpy(h->outstring+h->opos+alen+1,bstring);
		strcpy(h->outstring+h->opos+alen+strlen(h->separator),bstring);
		//		len = alen+blen+1;
		len = alen+blen+strlen(h->separator);
	    }
	}
	
	/* Print one side only */
	if (((h->mode) & (UPPER|LOWER)) != (UPPER|LOWER)) {
	    
	    if (symin == EPSILON) {
		astring = ""; alen = 0;
	    }
	    if (symout == EPSILON) {
		bstring = ""; blen = 0;
	    }
	    if (((h->mode) & (UPPER|LOWER)) == UPPER) {
		pstring = astring; 
		len = alen;
	    } else {
		pstring = bstring;
		len = blen;
	    }
	    //strcpy(h->outstring+h->opos, pstring);
	    memcpy(h->outstring+h->opos, pstring, len);
	}
    }
    if (((h->mode) & ENUMERATE) != ENUMERATE) {
	/* Print pairs is ON and symbols are different */
	if (h->print_pairs && (symin != symout)) {

	    if (symin == UNKNOWN && ((h->mode) & DOWN) == DOWN)
		strncpy(astring, h->instring+h->ipos, 1);
	    if (symout == UNKNOWN && ((h->mode) & UP) == UP)
		strncpy(bstring, h->instring+h->ipos, 1);
	    strcpy(h->outstring+h->opos, "<");
	    strcpy(h->outstring+h->opos+1, astring);
	    //strcpy(h->outstring+h->opos+alen+1,":");
	    strcpy(h->outstring+h->opos+alen+1,h->separator);
	    //strcpy(h->outstring+h->opos+alen+2,bstring);
	    strcpy(h->outstring+h->opos+alen+1+strlen(h->separator), bstring);
	    //strcpy(h->outstring+h->opos+alen+blen+2,">");
	    strcpy(h->outstring+h->opos+alen+blen+1+strlen(h->separator),">");
	    //len = alen+blen+3;
	    len = alen+blen+2+strlen(h->separator);
	}

	else if (sym == IDENTITY) {
	    /* Apply up/down */
	    //idlen = utf8skip(h->instring+h->ipos)+1;
	    idlen = (h->sigmatch_array+h->ipos)->consumes; // here
	    strncpy(h->outstring+h->opos, h->instring+h->ipos, idlen);
	    strncpy(h->outstring+h->opos+idlen,"", 1);
	    len = idlen;
	} else if (sym == EPSILON) {
	    return(0);
	} else {
	    if (((h->mode) & DOWN) == DOWN) {
		pstring = bstring;
		len = blen;
	    } else {
		pstring = astring;
		len = alen;
	    }
	    memcpy(h->outstring+h->opos, pstring, len);
	}
    }
    if (h->print_space && len > 0) {
	strcpy(h->outstring+h->opos+len, h->space_symbol);
	len++;
    }
    return(len);
}

int apply_match_length(struct apply_handle *h, int symbol) {
    if (symbol == EPSILON) {
	return 0;
    }
    if (h->has_flags && (h->flag_lookup+symbol)->type) {
	return 0;
    }
    if (((h->mode) & ENUMERATE) == ENUMERATE) {
	return 0;
    }
    if (h->ipos >= h->current_instring_length) {
	return -1;
    }
    if ((h->sigmatch_array+(h->ipos))->signumber == symbol) {
	    return((h->sigmatch_array+(h->ipos))->consumes);
    }
    if ((symbol == IDENTITY) || (symbol == UNKNOWN)) {
	if ((h->sigmatch_array+h->ipos)->signumber == IDENTITY) {
	    return((h->sigmatch_array+(h->ipos))->consumes);
	}
    }
    return -1;
}

/* Match a symbol from sigma against the current position in string */
/* Return the number of symbols consumed in input string            */
/* For flags, we consume 0 symbols of the input string, naturally   */

int apply_match_str(struct apply_handle *h, int symbol, int position) {
    if (((h->mode) & ENUMERATE) == ENUMERATE) {
	if (h->has_flags && (h->flag_lookup+symbol)->type) {
	    if (!h->obey_flags) {
		return 0;
	    }
	    if (apply_check_flag(h,(h->flag_lookup+symbol)->type, (h->flag_lookup+symbol)->name, (h->flag_lookup+symbol)->value) == SUCCEED) {
		return 0;
	    } else {
		return -1;
	    }
	}
	return(0);
    }


    if (symbol == EPSILON) {
	return 0;
    }
    
    /* If symbol is a flag, we need to check consistency */
    if (h->has_flags && (h->flag_lookup+symbol)->type) {
	if (!h->obey_flags) {
	    return 0;
	}
	if (apply_check_flag(h,(h->flag_lookup+symbol)->type, (h->flag_lookup+symbol)->name, (h->flag_lookup+symbol)->value) == SUCCEED) {
	    return 0;
	} else {
	    return -1;
	}
    }
    
    if (position >= h->current_instring_length) {
	return -1;
    }
    if ((h->sigmatch_array+position)->signumber == symbol) {
	return((h->sigmatch_array+position)->consumes);
    }
    if ((symbol == IDENTITY) || (symbol == UNKNOWN)) {
	if ((h->sigmatch_array+position)->signumber == IDENTITY) {
	    return((h->sigmatch_array+position)->consumes);
	}
    }
    return -1;
}

void apply_create_statemap(struct apply_handle *h, struct fsm *net) {
    int i;
    struct fsm_state *fsm;
    fsm = net->states;
    h->statemap = xxmalloc(sizeof(int)*net->statecount);
    h->marks = xxmalloc(sizeof(int)*net->statecount);
    h->numlines = xxmalloc(sizeof(int)*net->statecount);

    for (i=0; i < net->statecount; i++) {
	*(h->numlines+i) = 0;  /* Only needed in binary search */
	*(h->statemap+i) = -1;
	*(h->marks+i) = 0;
    }
    for (i=0; (fsm+i)->state_no != -1; i++) {
	*(h->numlines+(fsm+i)->state_no) = *(h->numlines+(fsm+i)->state_no)+1;
	if (*(h->statemap+(fsm+i)->state_no) == -1) {
	    *(h->statemap+(fsm+i)->state_no) = i;
	}
    }
}

void apply_add_sigma_trie(struct apply_handle *h, int number, char *symbol, int len) {

    /* Create a trie of sigma symbols (prefixes) so we can    */
    /* quickly (in O(n)) tokenize an arbitrary string into    */
    /* integer sequences representing symbols, using longest- */
    /* leftmost factorization.                                */

    int i;
    struct sigma_trie *st;
    struct sigma_trie_arrays *sta;

    st = h->sigma_trie;
    for (i = 0; i < len; i++) {
	st = st+(unsigned char)*(symbol+i);
	if (i == (len-1)) {
	    st->signum = number;
	} else {
	    if (st->next == NULL) {
		st->next = xxcalloc(256,sizeof(struct sigma_trie));		
		st = st->next;
		/* store these arrays to free them later */
		sta = xxmalloc(sizeof(struct sigma_trie_arrays));
		sta->arr = st;
		sta->next = h->sigma_trie_arrays;
		h->sigma_trie_arrays = sta;
	    } else {
		st = st->next;
	    }
	}
    }
}

void apply_mark_flagstates(struct apply_handle *h) {
    int i;
    struct fsm_state *fsm;

    /* Create bitarray with those states that have a flag symbol on an arc */
    /* This is needed to decide whether we can perform a binary search.    */

    if (!h->has_flags || h->flag_lookup == NULL) {
	return;
    }
    if (h->flagstates) {
	xxfree(h->flagstates);
    }
    h->flagstates = xxcalloc(BITNSLOTS(h->last_net->statecount), sizeof(uint8_t));
    fsm = h->last_net->states;
    for (i=0; (fsm+i)->state_no != -1; i++) {
	if ((fsm+i)->target == -1) { 
	    continue;
	}
	if ((h->flag_lookup+(fsm+i)->in)->type) {
	    BITSET(h->flagstates,(fsm+i)->state_no);
	}
	if ((h->flag_lookup+(fsm+i)->out)->type) {
	    BITSET(h->flagstates,(fsm+i)->state_no);
	}
    }
}

void apply_create_sigarray(struct apply_handle *h, struct fsm *net) {
    struct sigma *sig;
    int i, maxsigma;
    
    maxsigma = sigma_max(net->sigma);
    h->sigma_size = maxsigma+1;
    // Default size created at init, resized later if necessary
    h->sigmatch_array = xxcalloc(1024,sizeof(struct sigmatch_array));
    h->sigmatch_array_size = 1024;

    h->sigs = xxmalloc(sizeof(struct sigs)*(maxsigma+1));
    h->has_flags = 0;
    h->flag_list = NULL;

    /* Malloc first array of trie and store trie ptrs to be able to free later */
    /* when apply_clear() is called.                                           */

    h->sigma_trie = xxcalloc(256,sizeof(struct sigma_trie));
    h->sigma_trie_arrays = xxmalloc(sizeof(struct sigma_trie_arrays));
    h->sigma_trie_arrays->arr = h->sigma_trie;
    h->sigma_trie_arrays->next = NULL;

    for (i=0;i<256;i++)
	(h->sigma_trie+i)->next = NULL;
    for (sig = h->gsigma; sig != NULL && sig->number != -1; sig = sig->next) {
	if (flag_check(sig->symbol)) {
	    h->has_flags = 1;
	    apply_add_flag(h, flag_get_name(sig->symbol));
	}
	(h->sigs+(sig->number))->symbol = sig->symbol;
	(h->sigs+(sig->number))->length = strlen(sig->symbol);
	/* Add sigma entry to trie */
	if (sig->number > IDENTITY) {
	    apply_add_sigma_trie(h, sig->number, sig->symbol, (h->sigs+(sig->number))->length);
	}
    }
    if (maxsigma >= IDENTITY) {
	(h->sigs+EPSILON)->symbol = h->epsilon_symbol;
	(h->sigs+EPSILON)->length =  strlen(h->epsilon_symbol);
	(h->sigs+UNKNOWN)->symbol = "?";
	(h->sigs+UNKNOWN)->length =  1;
	(h->sigs+IDENTITY)->symbol = "@";
	(h->sigs+IDENTITY)->length =  1;
    }
    if (h->has_flags) {

	h->flag_lookup = xxmalloc(sizeof(struct flag_lookup)*(maxsigma+1));
	for (i=0; i <= maxsigma; i++) {
	    (h->flag_lookup+i)->type = 0;
	    (h->flag_lookup+i)->name = NULL;
	    (h->flag_lookup+i)->value = NULL;
	}
	for (sig = h->gsigma; sig != NULL ; sig = sig->next) {
	    if (flag_check(sig->symbol)) {
		(h->flag_lookup+sig->number)->type = flag_get_type(sig->symbol);
		(h->flag_lookup+sig->number)->name = flag_get_name(sig->symbol);
		(h->flag_lookup+sig->number)->value = flag_get_value(sig->symbol);		
	    }
	}
	apply_mark_flagstates(h);
    }
}

/* We need to know which symbols in sigma we can match for all positions           */
/* in the input string.  Alternatively, if there is no input string as is the case */
/* when we just list the words or randomly search the graph, we can match          */
/* any symbol in sigma.                                                            */

/* We create an array that for each position in the input string        */
/* has information on which symbol we can match at that position        */
/* as well as how many symbols matching consumes                        */

void apply_create_sigmatch(struct apply_handle *h) {
    char *symbol;
    struct sigma_trie *st;
    int i, j, inlen, lastmatch, consumes, cons;
    /* We create a sigmatch array only in case we match against a real string */
    if (((h->mode) & ENUMERATE) == ENUMERATE) {
	return;
    }
    symbol = h->instring;
    inlen = strlen(symbol);
    h->current_instring_length = inlen;
    if (inlen >= h->sigmatch_array_size) {
	xxfree(h->sigmatch_array);
	h->sigmatch_array = xxmalloc(sizeof(struct sigmatch_array)*(inlen));
	h->sigmatch_array_size = inlen;
    }
    /* Find longest match in alphabet at current position */
    /* by traversing the trie of alphabet symbols         */
    for (i=0; i < inlen; i += consumes ) {
	st = h->sigma_trie;
	for (j=0, lastmatch = 0; ; j++) {
	    if (*(symbol+i+j) == '\0') {
		break;
	    }
	    st = st+(unsigned char)*(symbol+i+j);
	    if (st->signum != 0) {
		lastmatch = st->signum;
		if (st->next == NULL)
		    break;
		st = st->next;
	    } else if (st->next != NULL) {
		st = st->next;
	    } else {
		break;
	    }
	}
	if (lastmatch != 0) {
	    (h->sigmatch_array+i)->signumber = lastmatch;
	    consumes = (h->sigs+lastmatch)->length;
	} else {
	    /* Not found in trie */
	    (h->sigmatch_array+i)->signumber = IDENTITY;
	    consumes = utf8skip(symbol+i)+1;
	}

	/* If we now find trailing unicode combining characters (0300-036F):      */
	/* (1) Merge them all with current symbol                                 */
	/* (2) Declare the whole sequence one ? (IDENTITY) symbol                 */
        /*     Step 2 is motivated by the fact that                               */
	/*     if the input is S(symbol) + D(diacritic)                           */
        /*     and SD were a symbol in the alphabet, then this would have been    */
        /*     found when searching the alphabet symbols earlier, so SD+ => ?     */
        /*     Note that this means that a multi-char symbol followed by a        */
        /*     diacritic gets converted to a single ?, e.g.                       */
        /*     [TAG] + D => ? if [TAG] is in the alphabet, but [TAG]+D isn't.     */

	for (  ; (cons = utf8iscombining((unsigned char *)(symbol+i+consumes))); consumes += cons) {
	    (h->sigmatch_array+i)->signumber = IDENTITY;
	}
	(h->sigmatch_array+i)->consumes = consumes;
    }
}

void apply_add_flag(struct apply_handle *h, char *name) {
    struct flag_list *flist, *flist_prev;
    if (h->flag_list == NULL) {
	flist = h->flag_list = xxmalloc(sizeof(struct flag_list));
    } else {
	for (flist = h->flag_list; flist != NULL; flist_prev = flist, flist = flist->next) {
	    if (strcmp(flist->name, name) == 0) {
		return;
	    }
	}
	flist = xxmalloc(sizeof(struct flag_list));
	flist_prev->next = flist;
    }
    flist->name = name;
    flist->value = NULL;
    flist->neg = 0;
    flist->next = NULL;
    return;
}

void apply_clear_flags(struct apply_handle *h) {
    struct flag_list *flist;
    for (flist = h->flag_list; flist != NULL; flist = flist->next) {
	flist->value = NULL;
	flist->neg = 0;
    }
    return;
}

/* Check for flag consistency by looking at the current states of */
/* flags in flist */

int apply_check_flag(struct apply_handle *h, int type, char *name, char *value) {
    struct flag_list *flist, *flist2;
    for (flist = h->flag_list; flist != NULL; flist = flist->next) {
	if (strcmp(flist->name, name) == 0) {
	    break;
	}
    }
    h->oldflagvalue = flist->value;
    h->oldflagneg = flist->neg;
    
    if (type == FLAG_UNIFY) {
	if (flist->value == NULL) {
	    flist->value = xxstrdup(value);
	    return SUCCEED;
	}
	else if (strcmp(value, flist->value) == 0 && flist->neg == 0) {
	    return SUCCEED;	    
	} else if (strcmp(value, flist->value) != 0 && flist->neg == 1) {
	    flist->value = xxstrdup(value);
	    flist->neg = 0;
	    return SUCCEED;
	}  
	return FAIL;
    }

    if (type == FLAG_CLEAR) {
	flist->value = NULL;
	flist->neg = 0;
	return SUCCEED;
    }

    if (type == FLAG_DISALLOW) {
	if (flist->value == NULL) {
	    return SUCCEED;
	}
	if (value == NULL && flist->value != NULL) {
	    return FAIL;
	}
	if (strcmp(value, flist->value) != 0) {
            if (flist->neg == 1)
                return FAIL;
            return SUCCEED;
	}
	if (strcmp(value, flist->value) == 0 && flist->neg == 1) {
            return SUCCEED;
        }
        return FAIL;
    }

    if (type == FLAG_NEGATIVE) {
	flist->value = value;
	flist->neg = 1;
	return SUCCEED;
    }

    if (type == FLAG_POSITIVE) {
	flist->value = value;
	flist->neg = 0;
	return SUCCEED;
    }

    if (type == FLAG_REQUIRE) {

	if (value == NULL) {
	    if (flist->value == NULL) {
		return FAIL;
	    } else {
		return SUCCEED;
	    }
	} else {
	    if (flist->value == NULL) {
		return FAIL;
	    }
	    if (strcmp(value, flist->value) != 0) {
		return FAIL;
	    } else {
                if (flist->neg == 1) {
                    return FAIL;
                }
		return SUCCEED;
	    }
	}
    }

    if (type == FLAG_EQUAL) {
	for (flist2 = h->flag_list; flist2 != NULL; flist2 = flist2->next) {
	    if (strcmp(flist2->name, value) == 0) {
		break;
	    }
	}

	if (flist2 == NULL && flist->value != NULL)
	    return FAIL;
	if (flist2 == NULL && flist->value == NULL) {
	    return SUCCEED;
	}
	if (flist2->value == NULL || flist->value == NULL) {
	    if (flist2->value == NULL && flist->value == NULL && flist->neg == flist2->neg) {
		return SUCCEED;
	    } else {
		return FAIL;
	    }
	}  else if (strcmp(flist2->value, flist->value) == 0 && flist->neg == flist2->neg) {
	    return SUCCEED;
	}
	return FAIL;	
    }
    fprintf(stderr,"***Don't know what do with flag [%i][%s][%s]\n", type, name, value);
    return FAIL;
}
