/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2011 Mans Hulden                                     */

/*     This file is part of foma.                                            */

/*     Foma is free software: you can redistribute it and/or modify          */
/*     it under the terms of the GNU General Public License version 2 as     */
/*     published by the Free Software Foundation. */

/*     Foma is distributed in the hope that it will be useful,               */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*     GNU General Public License for more details.                          */

/*     You should have received a copy of the GNU General Public License     */
/*     along with foma.  If not, see <http://www.gnu.org/licenses/>.         */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "foma.h"

#define INITIAL_AGENDA_SIZE 256
#define INITIAL_HEAP_SIZE 256
#define INITIAL_STRING_SIZE 256
#define MED_DEFAULT_LIMIT 4              /* Default max words to find                 */
#define MED_DEFAULT_CUTOFF 15            /* Default MED cost cutoff                   */
#define MED_DEFAULT_MAX_HEAP_SIZE 262145 /* By default won't grow heap more than this */
 
#define BITMASK(b) (1 << ((b) & 7))
#define BITSLOT(b) ((b) >> 3)
#define BITSET(a,b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a,b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a,b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)
#define min_(X, Y)  ((X) < (Y) ? (X) : (Y))

static int calculate_h(struct apply_med_handle *medh, int *intword, int currpos, int state);
static struct astarnode *node_delete_min();
int node_insert(struct apply_med_handle *medh, int wordpos, int fsmstate, int g, int h, int in, int out, int parent);

char *print_sym(int sym, struct sigma *sigma) {
    while (sigma != NULL) {
        if (sigma->number == sym) {
	    return(sigma->symbol);
        }
        sigma = sigma->next;
    }
    return NULL;
}

void print_match(struct apply_med_handle *medh, struct astarnode *node, struct sigma *sigma, char *word) {
    int sym, i, wordlen , printptr;
    struct astarnode *n;
    int_stack_clear();
    wordlen = medh->wordlen;
    for (n = node; n != NULL ; n = medh->agenda+(n->parent)) {
        if (n->in == 0 && n->out == 0)
            break;
        if (n->parent == -1)
            break;
        int_stack_push(n->in);
    }
    printptr = 0;
    if (medh->outstring_length < 2*wordlen) {
	medh->outstring_length *= 2;
	medh->outstring = xxrealloc(medh->outstring, medh->outstring_length*sizeof(char));
    }
    while (!(int_stack_isempty())) {
        sym = int_stack_pop();
        if (sym > 2) {
            printptr += sprintf(medh->outstring+printptr,"%s", print_sym(sym, sigma));
        }
        if (sym == 0) {
	    if (medh->align_symbol) {
		printptr += sprintf(medh->outstring+printptr,"%s",medh->align_symbol);
	    }
        }
        if (sym == 2) {
            printptr += sprintf(medh->outstring+printptr,"@");
        }
    }
    for (n = node; n != NULL ; n = medh->agenda+(n->parent)) {
        if (n->in == 0 && n->out == 0)
            break;
        if (n->parent == -1)
            break;
        else
            int_stack_push(n->out);
    }
    printptr = 0;
    if (medh->instring_length < 2*wordlen) {
	medh->instring_length *= 2;
	medh->instring = xxrealloc(medh->instring, medh->instring_length*sizeof(char));
    }
    for (i = 0; !(int_stack_isempty()); ) {
        sym = int_stack_pop();	
        if (sym > 2) {
            printptr += sprintf(medh->instring+printptr,"%s", print_sym(sym, sigma));
            i += utf8skip(word+i)+1;
        }
        if (sym == 0) {
	    if (medh->align_symbol) {
		printptr += sprintf(medh->instring+printptr,"%s",medh->align_symbol);
	    }
        }
        if (sym == 2) {
            if (i > wordlen) {
		printptr += sprintf(medh->instring+printptr,"*");
            } else {
		//printf("%.*s", utf8skip(word+i)+1, word+i);
		printptr += sprintf(medh->instring+printptr,"%.*s", utf8skip(word+i)+1, word+i);
                i+= utf8skip(word+i)+1;
            }
        }
    }
    medh->cost = node->g;    
    // printf("Cost[f]: %i\n\n", node->g);
}

void apply_med_clear(struct apply_med_handle *medh) {
    if (medh == NULL)
	return;
    if (medh->agenda != NULL)
	xxfree(medh->agenda);
    if (medh->instring != NULL)
	xxfree(medh->instring);
    if (medh->outstring != NULL)
	xxfree(medh->outstring);
    if (medh->heap != NULL)
	xxfree(medh->heap);
    if (medh->state_array != NULL)
	xxfree(medh->state_array);
    if (medh->align_symbol != NULL)
	xxfree(medh->align_symbol);
    if (medh->letterbits != NULL)
	xxfree(medh->letterbits);
    if (medh->nletterbits != NULL)
	xxfree(medh->nletterbits);
    if (medh->intword != NULL)
	xxfree(medh->intword);
    if (medh->sigmahash != NULL)
	sh_done(medh->sigmahash);
    xxfree(medh);
}

struct apply_med_handle *apply_med_init(struct fsm *net) {

    struct apply_med_handle *medh;
    struct sigma *sigma;
    medh = xxcalloc(1,sizeof(struct apply_med_handle));    
    medh->net = net;
    medh->agenda = xxmalloc(sizeof(struct astarnode)*INITIAL_AGENDA_SIZE);
    medh->agenda->f = -1;
    medh->agenda_size = INITIAL_AGENDA_SIZE;
    
    medh->heap = xxmalloc(sizeof(int)*INITIAL_HEAP_SIZE);
    medh->heap_size = INITIAL_HEAP_SIZE;
    *(medh->heap) = 0; /* Points to sentinel */
    medh->astarcount = 1;
    medh->heapcount = 0;
    medh->state_array = map_firstlines(net);
    if (net->medlookup != NULL && net->medlookup->confusion_matrix != NULL) {
	medh->hascm = 1;
	medh->cm = net->medlookup->confusion_matrix;
    }
    medh->maxsigma = sigma_max(net->sigma)+1;
    medh->sigmahash = sh_init();
    for (sigma = net->sigma; sigma != NULL && sigma->number != -1 ; sigma=sigma->next ) {
	if (sigma->number > IDENTITY) {
	    sh_add_string(medh->sigmahash, sigma->symbol, sigma->number);
	}
    }


    fsm_create_letter_lookup(medh, net);

    medh->instring = xxmalloc(sizeof(char)*INITIAL_STRING_SIZE);
    medh->instring_length = INITIAL_STRING_SIZE;
    medh->outstring = xxmalloc(sizeof(char)*INITIAL_STRING_SIZE);
    medh->outstring_length = INITIAL_STRING_SIZE;
    
    medh->med_limit = MED_DEFAULT_LIMIT;
    medh->med_cutoff = MED_DEFAULT_CUTOFF;
    medh->med_max_heap_size = MED_DEFAULT_MAX_HEAP_SIZE;
    return(medh);
}

void apply_med_set_heap_max(struct apply_med_handle *medh, int max) {
    if (medh != NULL) {
	medh->med_max_heap_size = max;
    }
}

void apply_med_set_align_symbol(struct apply_med_handle *medh, char *align) {
    if (medh != NULL) {
	medh->align_symbol = xxstrdup(align);
    }
}

void apply_med_set_med_limit(struct apply_med_handle *medh, int max) {
    if (medh != NULL) {
	medh->med_limit = max;
    }
}

void apply_med_set_med_cutoff(struct apply_med_handle *medh, int max) {
    if (medh != NULL) {
	medh->med_cutoff = max;
    }
}

int apply_med_get_cost(struct apply_med_handle *medh) {
    return(medh->cost);
}

char *apply_med_get_instring(struct apply_med_handle *medh) {
    return(medh->instring);
}

char *apply_med_get_outstring(struct apply_med_handle *medh) {
    return(medh->outstring);
}

char *apply_med(struct apply_med_handle *medh, char *word) {

    /* local ok: i, j, target, in, out, g, h, curr_node                                   */
    /* not ok: curr_ptr, curr_pos, lines, nummatches, nodes_expanded, curr_state           */

    int i, j, target, in, out, g, h, thisskip;

    int delcost, subscost, inscost;
    char temputf[5] ;

    struct astarnode *curr_node;

    delcost = subscost = inscost = 1;


    if (word == NULL) {
	goto resume;
    }

    medh->word = word;

    medh->nodes_expanded = 0;
    medh->astarcount = 1;
    medh->heapcount = 0;

    medh->wordlen = strlen(word);
    medh->utf8len = utf8strlen(word);
    if (medh->intword != NULL) {
	xxfree(medh->intword);
    }
    medh->intword = xxmalloc(sizeof(int)*(medh->utf8len+1));

   /* intword -> sigma numbers of word */
    for (i=0, j=0; i < medh->wordlen; i += thisskip, j++) {
	thisskip = utf8skip(word+i)+1;
	strncpy(temputf, word+i, thisskip);
	temputf[thisskip] = '\0';
	if (sh_find_string(medh->sigmahash, temputf) != NULL) {	    
	    *(medh->intword+j) = sh_get_value(medh->sigmahash);
	} else {
            *(medh->intword+j) = IDENTITY;
        }
    }

    

    *(medh->intword+j) = -1; /* sentinel */
    
    /* Insert (0,0) g = 0 */
    
    h = calculate_h(medh, medh->intword, 0, 0);

    /* Root node */
    
    if (!node_insert(medh,0,0,0,h,0,0,-1)) { goto out; }
    medh->nummatches = 0;

    for(;;) {

        curr_node = node_delete_min(medh);
        /* Need to save this in case we realloc and print_match() */
        medh->curr_agenda_offset = curr_node-medh->agenda;
        if (curr_node == NULL) {
	    //printf("Reached cutoff of %i.\n", medh->med_cutoff);
            goto out;
        }
	medh->curr_state = curr_node->fsmstate;
	medh->curr_ptr = (medh->state_array+medh->curr_state)->transitions;
	if (!medh->curr_ptr->final_state || !(curr_node->wordpos == medh->utf8len)) {
	    //continue;
	}

        medh->nodes_expanded++;

        if (curr_node->f > medh->med_cutoff) {
            //printf("Reached cutoff of %i\n", medh->med_cutoff);
            goto out;
        }

        medh->curr_pos = curr_node->wordpos;
        medh->curr_state = curr_node->fsmstate;
        medh->curr_g = curr_node->g;
        
        medh->lines = 0;
        medh->curr_node_has_match = 0;

        for (medh->curr_ptr = (medh->state_array+medh->curr_state)->transitions ; ;) {
            if (medh->curr_ptr->state_no == -1) {
                break;
            }
            medh->lines++;
            if (medh->curr_ptr->final_state && medh->curr_pos == medh->utf8len) {
                if (medh->curr_node_has_match == 0) {
                    /* Found a match */
                    medh->curr_node_has_match = 1;
                    print_match(medh, medh->agenda+medh->curr_agenda_offset, medh->net->sigma, medh->word);
                    medh->nummatches++;
		    return(medh->outstring);
                }
            }

	resume:

	    if (medh->nummatches == medh->med_limit) {
		goto out;
	    }
	    
            if (medh->curr_ptr->target == -1 && medh->curr_pos == medh->utf8len)
                break;
            if (medh->curr_ptr->target == -1 && medh->lines == 1)
                goto insert;
            if (medh->curr_ptr->target == -1)
                break;

            target = medh->curr_ptr->target;
            /* Add nodes to edge:0, edge:input, 0:edge */
            
            /* Delete a symbol from input */
            in = medh->curr_ptr->in;
            out = 0;
            g = medh->hascm ? medh->curr_g + *(medh->cm+in*medh->maxsigma) : medh->curr_g + delcost;
            h = calculate_h(medh, medh->intword, medh->curr_pos, medh->curr_ptr->target);

            if ((medh->curr_pos == medh->utf8len) && (medh->curr_ptr->final_state == 0) && (h == 0)) {
		// h = 1;
            }

            if (g+h <= medh->med_cutoff) {
                if (!node_insert(medh, medh->curr_pos, target, g, h, in, out, medh->curr_agenda_offset)) {
		    goto out;
		}            
	    }
            if (medh->curr_pos == medh->utf8len)
                goto skip;

            /* Match/substitute */
            in = medh->curr_ptr->in;
            out = *(medh->intword+medh->curr_pos);
            if (in != out) {
                g = medh->hascm ? medh->curr_g + *(medh->cm+in*medh->maxsigma+out) : medh->curr_g + subscost;
            } else {
                g = medh->curr_g;
            }
           
            h = calculate_h(medh, medh->intword, medh->curr_pos+1, medh->curr_ptr->target);
            if ((g+h) <= medh->med_cutoff) {
                if (!node_insert(medh,medh->curr_pos+1, target, g, h, in, out, medh->curr_agenda_offset)) {
		    goto out;
		}
            }
        insert:
            /* Insert a symbol into input */
            /* Can only be done once per state */

            if (medh->lines == 1) {

                in = 0;
                out = *(medh->intword+medh->curr_pos);
                
                g = medh->hascm ? medh->curr_g + *(medh->cm+out) : medh->curr_g + inscost;
                h = calculate_h(medh, medh->intword, medh->curr_pos+1, medh->curr_state);
                
                if (g+h <= medh->med_cutoff)
                    if (!node_insert(medh,medh->curr_pos+1, medh->curr_state, g, h, in, out, medh->curr_agenda_offset)) {
			goto out;
		    }
            }
            if (medh->curr_ptr->target == -1)
                break;
        skip:
            if ((medh->curr_ptr+1)->state_no == (medh->curr_ptr)->state_no)
                medh->curr_ptr++;
            else
                break;
        }
    }
 out:
     return(NULL);
}

int calculate_h(struct apply_med_handle *medh, int *intword, int currpos, int state) {
    int i, j, hinf, hn, curr_sym;
    uint8_t *bitptr, *nbitptr;
    hinf = 0;
    hn = 0;

    bitptr = state*medh->bytes_per_letter_array + medh->letterbits;
    nbitptr = state*medh->bytes_per_letter_array + medh->nletterbits;

   /* For n = inf */
    if (*(intword+currpos) == -1)
        return 0;
    for (i = currpos; *(intword+i) != -1; i++) {
        curr_sym = *(intword+i);
        if (!BITTEST(bitptr, curr_sym)) {
            hinf++;
        }
    }
    /* For n = maxdepth */
    if (*(intword+currpos) == -1)
        return 0;
    for (i = currpos, j = 0; j < medh->maxdepth && *(intword+i) != -1; i++, j++) {
        curr_sym = *(intword+i);
        if (!BITTEST(nbitptr, curr_sym)) {
            hn++;
        }
    }
    return(hinf > hn ? hinf : hn);
}

struct astarnode *node_delete_min(struct apply_med_handle *medh) {
    int i, child;
    struct astarnode *firstptr, *lastptr;
    if (medh->heapcount == 0) {
        return NULL;
    }
 
   /* We find the min from the heap */

    firstptr = medh->agenda+medh->heap[1];
    lastptr = medh->agenda+medh->heap[medh->heapcount];
    medh->heapcount--;
    
    /* Adjust heap */
    for (i = 1; (i<<1) <= medh->heapcount; i = child) {
        child = i<<1;
        
        /* If right child is smaller (higher priority) than left child */
        if (child != medh->heapcount && 
            ((medh->agenda+medh->heap[child+1])->f < (medh->agenda+medh->heap[child])->f || 
             ((medh->agenda+medh->heap[child+1])->f <= (medh->agenda+medh->heap[child])->f && 
              (medh->agenda+medh->heap[child+1])->wordpos > (medh->agenda+medh->heap[child])->wordpos))) {
            child++;
        }
        
        /* If child has lower priority than last element */
        if ((medh->agenda+medh->heap[child])->f < lastptr->f || 
            ((medh->agenda+medh->heap[child])->f <= lastptr->f && 
             (medh->agenda+medh->heap[child])->wordpos > lastptr->wordpos)) {
            
            medh->heap[i] = medh->heap[child];
        } else {
            break;
        }
    }
    medh->heap[i] = (lastptr-medh->agenda);
    return(firstptr);
}

int node_insert(struct apply_med_handle *medh, int wordpos, int fsmstate, int g, int h, int in, int out, int parent) {
    int i,j,f;
    /* We add the node in the array */
    i = medh->astarcount;
    if (i >= (medh->agenda_size-1)) {
	if (medh->agenda_size*2 >= medh->med_max_heap_size) {
	    //printf("heap limit reached by %i\n", medh->med_max_heap_size);
	    return 0;
	}
        medh->agenda_size *= 2;
        medh->agenda = xxrealloc(medh->agenda, sizeof(struct astarnode)*medh->agenda_size);
    }
    f = g + h;
    (medh->agenda+i)->wordpos = wordpos;
    (medh->agenda+i)->fsmstate = fsmstate;
    (medh->agenda+i)->f = f;
    (medh->agenda+i)->g = g;
    (medh->agenda+i)->h = h;
    (medh->agenda+i)->in = in;
    (medh->agenda+i)->out = out;
    (medh->agenda+i)->parent = parent;
    medh->astarcount++;

    /* We also put the ptr on the heap */
    medh->heapcount++;

    if (medh->heapcount == medh->heap_size-1) {
        medh->heap = xxrealloc(medh->heap, sizeof(int)*medh->heap_size*2);
        medh->heap_size *= 2;
    }
    /*                                     >= makes fifo */
    // for (j = heapcount; (agenda+heap[j/2])->f > f; j /= 2) {
    for (j = medh->heapcount; ( (medh->agenda+medh->heap[j>>1])->f > f) || ((medh->agenda+medh->heap[j>>1])->f >= f && (medh->agenda+medh->heap[j>>1])->wordpos <= wordpos) ; j = j >> 1) {
        medh->heap[j] = medh->heap[j>>1];
    }
    medh->heap[j] = i;
    return 1;
}


void letterbits_union(int v, int vp, uint8_t *ptr, int bytes_per_letter_array) {
    int i;
    uint8_t *vptr, *vpptr;
    vptr = ptr+(v*bytes_per_letter_array);
    vpptr = ptr+(vp*bytes_per_letter_array);
    for (i=0; i < bytes_per_letter_array; i++) {
        *(vptr+i) = *(vptr+i)|*(vpptr+i);
    }
}

void letterbits_copy(int source, int target, uint8_t *ptr, int bytes_per_letter_array) {
    int i;
    uint8_t  *sourceptr, *targetptr;
    sourceptr = ptr+(source*bytes_per_letter_array);
    targetptr = ptr+(target*bytes_per_letter_array);
    for (i=0; i < bytes_per_letter_array; i++) {
        *(targetptr+i) = *(sourceptr+i);
    }
}

void letterbits_add(int v, int symbol, uint8_t *ptr, int bytes_per_letter_array) {
    uint8_t *vptr;
    vptr = ptr+(v*bytes_per_letter_array);
    BITSET(vptr, symbol);
}

/* Creates, for each state, a list of symbols that can be matched             */
/* somewhere in subsequent paths (the case n = inf)                           */
/* This is needed for h() of A*-search in approximate matching                */
/* This is done by a DFS where each state gets the properties                 */
/* of the arcs it has as well as a copy of the properties of the target state */
/* At the same time we keep track of the strongly connected components        */
/* And copy the properties from each root of the SCC to the children          */
/* The SCC part is required for the algorithm to work with cyclic graphs      */
/* This is basically a modification of Tarjan's (1972) SCC algorithm          */
/* However, it's converted from recursive form to iterative form using        */
/* goto statements.  Here's the original recursive specification:             */

/* Input: FSM = (V,E)                                                        */
/* Output: bitvector v.letters for each state                                */
/* index = 1                                    DFS node number counter      */

/* procedure Mark(v)                                                         */
/*   v.index = index                            Set the depth index for v    */
/*   v.lowlink = index                                                       */
/*   index++                                                                 */
/*   push(v)                                    Push v on the stack          */
/*   forall edge = (v, v') in E do              Consider successors of v     */
/*     v.letters |= v'.letters | edge           Copy target v'.letters       */
/*     if (v'.index == 0)                       Was successor v' visited?    */
/*       Mark(v')                               Recurse                      */
/*       v.lowlink = min(v.lowlink, v'.lowlink)                              */
/*     elseif (v' is on stack)                  Was successor v' in stack S? */
/*       v.lowlink = min(v.lowlink, v'.index)                                */
/*   if (v.lowlink == v.index)                  Is v the root of a SCC?      */
/*     do                                                                    */
/*       pop(v')                                                             */
/*       v'.letters = v.letters                                              */
/*     while (v' != v)                                                       */

/* For keeping track of the strongly connected components */
/* when doing the DFS                                     */

struct sccinfo {
    int index;
    int lowlink;
    int on_t_stack;
};

void fsm_create_letter_lookup(struct apply_med_handle *medh, struct fsm *net) {

    int num_states, num_symbols, index, v, vp, copystate, i, j;
    struct fsm_state *curr_ptr;
    struct sccinfo *sccinfo;
    int depth;
    medh->maxdepth = 2;

    num_states = net->statecount;
    num_symbols = sigma_max(net->sigma);
    
    medh->bytes_per_letter_array = BITNSLOTS(num_symbols+1);
    medh->letterbits = xxcalloc(medh->bytes_per_letter_array*num_states,sizeof(uint8_t));
    medh->nletterbits = xxcalloc(medh->bytes_per_letter_array*num_states,sizeof(uint8_t));

    sccinfo = xxcalloc(num_states,sizeof(struct sccinfo));
    
    index = 1;
    curr_ptr = net->states;
    goto l1;

    /* Here we go again, converting a recursive algorithm to an iterative one */
    /* by gotos */

    while(!ptr_stack_isempty()) {

        curr_ptr = ptr_stack_pop();

        v = curr_ptr->state_no; /* source state number */
        vp = curr_ptr->target;  /* target state number */

        /* T: v.letterlist = list_union(v'->list, current edge label) */

        letterbits_union(v, vp, medh->letterbits,medh->bytes_per_letter_array);         /* add v' target bits to v */
        letterbits_add(v, curr_ptr->in, medh->letterbits,medh->bytes_per_letter_array); /* add current arc label to v */

        (sccinfo+v)->lowlink = min_((sccinfo+v)->lowlink,(sccinfo+vp)->lowlink);

        if ((curr_ptr+1)->state_no != curr_ptr->state_no) {
            goto l4;
        } else {
            goto l3;
        }
        
    l1:
        v = curr_ptr->state_no;
        vp = curr_ptr->target;  /* target */
        /* T: v.lowlink = index, index++, Tpush(v) */
        (sccinfo+v)->index = index;
        (sccinfo+v)->lowlink = index;
        index++;
        int_stack_push(v);
        (sccinfo+v)->on_t_stack = 1;
        /* if v' not visited (is v'.index set) */

        /* If done here, check lowlink, pop */

        if (vp == -1)
            goto l4;
    l2:
        letterbits_add(v, curr_ptr->in, medh->letterbits,medh->bytes_per_letter_array);
        if ((sccinfo+vp)->index == 0) {
            /* push (v,e) ptr on stack */
            ptr_stack_push(curr_ptr);
            curr_ptr = (medh->state_array+(curr_ptr->target))->transitions;
            /* (v,e) = (v',firstedge), goto init */
            goto l1;
            /* if v' visited */
            /* T: v.lowlink = min(v.lowlink, v'.lowlink), union v.list with e, move to next edge in v, goto loop */
        } else if ((sccinfo+vp)->on_t_stack) {
            (sccinfo+v)->lowlink = min_((sccinfo+v)->lowlink,(sccinfo+vp)->lowlink);
        }
        /* If node is visited, copy its bits */
        letterbits_union(v,vp,medh->letterbits,medh->bytes_per_letter_array);

    l3:
        if ((curr_ptr+1)->state_no == curr_ptr->state_no) {
            curr_ptr++;
            v = curr_ptr->state_no;
            vp = curr_ptr->target;  /* target */
            goto l2;
        }

        
        /* T: if lastedge, v.lowlink == v.index, pop Tstack until v is popped and copy v.list to others */
        /* Copy all bits from root of SCC to descendants */
    l4:
        if ((sccinfo+v)->lowlink == (sccinfo+v)->index) {
            //printf("\nSCC: [%i] ",v);
            while((copystate = int_stack_pop()) != v) {
                (sccinfo+copystate)->on_t_stack = 0;
                letterbits_copy(v, copystate, medh->letterbits, medh->bytes_per_letter_array);
                //printf("%i ", copystate);
            }
            (sccinfo+v)->on_t_stack = 0;
            //printf("\n");
        }
    }
    
    for (i=0; i < num_states; i++) {
        //printf("State %i: ",i);
        for (j=0; j <= num_symbols; j++) {
            if (BITTEST(medh->letterbits+(i*medh->bytes_per_letter_array),j)) {
                //printf("[%i]",j);
            }
        }    
        //printf("\n");
    }
    int_stack_clear();

    /* We do the same thing for some finite n (up to maxdepth) */
    /* and store the result in nletterbits                     */

    for (v=0; v < num_states; v++) {
        ptr_stack_push((medh->state_array+v)->transitions);
        int_stack_push(0);
        while (!ptr_stack_isempty()) {
            curr_ptr = ptr_stack_pop();
            depth = int_stack_pop();
        looper:
            if (depth == medh->maxdepth)
                continue;
            if (curr_ptr->in != -1) {
                letterbits_add(v, curr_ptr->in, medh->nletterbits, medh->bytes_per_letter_array);
            }
            if (curr_ptr->target != -1) {
                if (curr_ptr->state_no == (curr_ptr+1)->state_no) {
                    ptr_stack_push(curr_ptr+1);
                    int_stack_push(depth);
                }
                depth++;
                curr_ptr = (medh->state_array+(curr_ptr->target))->transitions;
                goto looper;
            }
        }
    }
    for (i=0; i < num_states; i++) {
        //printf("State %i: ",i);
        for (j=0; j <= num_symbols; j++) {
            if (BITTEST(medh->nletterbits+(i*medh->bytes_per_letter_array),j)) {
                //printf("[%i]",j);
            }
        }
        //printf("\n");
    }
    xxfree(sccinfo);
}

void cmatrix_print_att(struct fsm *net, FILE *outfile) {
    int i, j, *cm, maxsigma;
    maxsigma = sigma_max(net->sigma) + 1;
    cm = net->medlookup->confusion_matrix;


    for (i = 0; i < maxsigma ; i++) {        
        for (j = 0; j < maxsigma ; j++) {
            if ((i != 0 && i < 3) || (j != 0 && j < 3)) { continue; }
            if (i == 0 && j != 0) {
                fprintf(outfile,"0\t0\t%s\t%s\t%i\n", "@0@", sigma_string(j, net->sigma), *(cm+i*maxsigma+j));
            } else if (j == 0 && i != 0) {
                fprintf(outfile,"0\t0\t%s\t%s\t%i\n", sigma_string(i,net->sigma), "@0@", *(cm+i*maxsigma+j));
            } else if (j != 0 && i != 0) {
                fprintf(outfile,"0\t0\t%s\t%s\t%i\n", sigma_string(i,net->sigma),sigma_string(j, net->sigma), *(cm+i*maxsigma+j));
            }
        }
    }
    fprintf(outfile,"0\n");
}

void cmatrix_print(struct fsm *net) {
    int lsymbol, i, j, *cm, maxsigma;
    char *thisstring;
    struct sigma *sigma;
    maxsigma = sigma_max(net->sigma) + 1;
    cm = net->medlookup->confusion_matrix;

    lsymbol = 0 ;
    for (sigma = net->sigma; sigma != NULL; sigma = sigma->next) {
        if (sigma->number < 3)
            continue;
        lsymbol = strlen(sigma->symbol) > lsymbol ? strlen(sigma->symbol) : lsymbol;
    }
    printf("%*s",lsymbol+2,"");
    printf("%s","0 ");

    for (i = 3; ; i++) {
        if ((thisstring = sigma_string(i, net->sigma)) != NULL) {
            printf("%s ",thisstring);
        } else {
            break;
        }
    }

    printf("\n");

    for (i = 0; i < maxsigma ; i++) {        
        for (j = 0; j < maxsigma ; j++) {
            if (j == 0) {
                if (i == 0) {
                    printf("%*s",lsymbol+1,"0");
                    printf("%*s",2,"*");
                } else {
                    printf("%*s",lsymbol+1, sigma_string(i, net->sigma));
                    printf("%*d",2,*(cm+i*maxsigma+j));
                }
                j++;
                j++;
                continue;
            }
            if (i == j) {
                printf("%.*s",(int)strlen(sigma_string(j, net->sigma))+1,"*");
            } else {
                printf("%.*d",(int)strlen(sigma_string(j, net->sigma))+1,*(cm+i*maxsigma+j));
            }
        }
        printf("\n");
        if (i == 0) {
            i++; i++;
        }
    }
}

void cmatrix_init(struct fsm *net) {
    int maxsigma, *cm, i, j;
    if (net->medlookup == NULL) {
        net->medlookup = xxcalloc(1,sizeof(struct medlookup));
    }
    maxsigma = sigma_max(net->sigma)+1;
    cm = xxcalloc(maxsigma*maxsigma, sizeof(int));
    net->medlookup->confusion_matrix = cm;
    for (i = 0; i < maxsigma; i++) {
        for (j = 0; j < maxsigma; j++) {
            if (i == j)
                *(cm+i*maxsigma+j) = 0;
            else
                *(cm+i*maxsigma+j) = 1;
        }
    } 
}

void cmatrix_default_substitute(struct fsm *net, int cost) {
    int i, j, maxsigma, *cm;
    cm = net->medlookup->confusion_matrix;
    maxsigma = sigma_max(net->sigma)+1;
    for (i = 1; i < maxsigma; i++) {
        for (j = 1; j < maxsigma; j++) {
            if (i == j) {
                *(cm+i*maxsigma+j) = 0;                
            } else {
                *(cm+i*maxsigma+j) = cost;
            }
        }
    } 
}

void cmatrix_default_insert(struct fsm *net, int cost) {
    int i, maxsigma, *cm;
    cm = net->medlookup->confusion_matrix;
    maxsigma = sigma_max(net->sigma)+1;
    for (i = 0; i < maxsigma; i++) {
        *(cm+i) = cost;
    }
}

void cmatrix_default_delete(struct fsm *net, int cost) {
    int i, maxsigma, *cm;
    cm = net->medlookup->confusion_matrix;
    maxsigma = sigma_max(net->sigma)+1;
    for (i = 0; i < maxsigma; i++) {
        *(cm+i*maxsigma) = cost;
    }
}

void cmatrix_set_cost(struct fsm *net, char *in, char *out, int cost) {
    int i, o, maxsigma, *cm;
    cm = net->medlookup->confusion_matrix;
    maxsigma = sigma_max(net->sigma) + 1;
    if (in == NULL) {
        i = 0;
    } else {
        i = sigma_find(in, net->sigma);
    }
    if (out == NULL) {
        o = 0;
    } else {
        o = sigma_find(out, net->sigma);
    }
    if (i == -1) {
        printf("Warning, symbol '%s' not in alphabet\n",in);
        return;
    }
    if (o == -1) {
        printf("Warning, symbol '%s' not in alphabet\n",out);
        return;
    }
    *(cm+i*maxsigma+o) = cost;
}
