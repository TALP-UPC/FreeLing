/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2010 Mans Hulden                                     */

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
#include <string.h>
#include <stdlib.h>
#include "foma.h"

static struct defined_quantifiers *quantifiers;

char *fsm_get_library_version_string() {
    static char s[20];
    sprintf(s,"%i.%i.%i%s",MAJOR_VERSION,MINOR_VERSION,BUILD_VERSION,STATUS_VERSION);
    return(s);
}

int linesortcompin(const void *a, const void *b) {
  return ((struct fsm_state *)a)->in - ((struct fsm_state *)b)->in;
}

int linesortcompout(const void *a, const void *b) {
  return ((struct fsm_state *)a)->out - ((struct fsm_state *)b)->out;
}

void fsm_sort_arcs(struct fsm *net, int direction) {
    /* direction 1 = in, direction = 2, out */
    struct fsm_state *fsm;
    int i, lasthead, numlines;
    //int(*scin)(const void*, const void*) = linesortcompin;
    //int(*scout)(const void*, const void*) = linesortcompout;
    fsm = net->states;
    for (i=0, numlines = 0, lasthead = 0 ; (fsm+i)->state_no != -1; i++) {
	if ((fsm+i)->state_no != (fsm+i+1)->state_no || (fsm+i)->target == -1) {
	    numlines++;
	    if ((fsm+i)->target == -1) {
		numlines--;
	    }
	    if (numlines > 1) {
		/* Sort, set numlines = 0 */
		if (direction == 1)
		    qsort(fsm+lasthead, numlines, sizeof(struct fsm_state), linesortcompin);
		else
		    qsort(fsm+lasthead, numlines, sizeof(struct fsm_state), linesortcompout);		
	    }
	    numlines = 0;
	    lasthead = i + 1;
	    continue;
	}
	numlines++;
    }
    if (net->arity == 1) {
	net->arcs_sorted_in = 1;
	net->arcs_sorted_out = 1;
	return;
    }
    if (direction == 1) {
	net->arcs_sorted_in = 1;
	net->arcs_sorted_out = 0;
    }
    if (direction == 2) {
	net->arcs_sorted_out = 1;
	net->arcs_sorted_in = 0;
    }    
}

struct state_array *map_firstlines(struct fsm *net) {
    struct fsm_state *fsm;
    struct state_array *sa;
    int i, sold;
    sold = -1;
    sa = xxmalloc(sizeof(struct state_array)*((net->statecount)+1));
    fsm = net->states;
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->state_no != sold) {
            (sa+((fsm+i)->state_no))->transitions = fsm+i;
            sold = (fsm+i)->state_no;
        }
    }
    return(sa);
}

struct fsm *fsm_boolean(int value) {
    if (value == 0)
        return (fsm_empty_set());
    else
        return(fsm_empty_string());
}

struct fsm *fsm_sigma_net(struct fsm *net) {
    /* Extract sigma and create net with one arc            */
    /* from state 0 to state 1 with each (state 1 is final) */
    struct sigma *sig;
    int pathcount;

    if (sigma_size(net->sigma) == 0) {
	fsm_destroy(net);
        return(fsm_empty_set());
    }
    
    fsm_state_init(sigma_max(net->sigma));
    fsm_state_set_current_state(0, 0, 1);
    pathcount = 0;
    for (sig = net->sigma; sig != NULL; sig = sig->next) {
        if (sig->number >=3 || sig->number == IDENTITY) {
            pathcount++;
            fsm_state_add_arc(0,sig->number, sig->number, 1, 0, 1);
        }
    }
    fsm_state_end_state();
    fsm_state_set_current_state(1, 1, 0);
    fsm_state_end_state();
    xxfree(net->states);
    fsm_state_close(net);
    net->is_minimized = YES;
    net->is_loop_free = YES;
    net->pathcount = pathcount;
    sigma_cleanup(net, 1);
    return(net);
}

struct fsm *fsm_sigma_pairs_net(struct fsm *net) {
    /* Create FSM of attested pairs */
    struct fsm_state *fsm;
    char *pairs;
    short int in, out;
    int i, pathcount, smax;
    
    smax = sigma_max(net->sigma)+1;
    pairs = xxcalloc(smax*smax, sizeof(char));

    fsm_state_init(sigma_max(net->sigma));
    fsm_state_set_current_state(0, 0, 1);
    pathcount = 0;
    for (fsm = net->states, i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target == -1)
            continue;
        in = (fsm+i)->in;
        out = (fsm+i)->out;
        if (*(pairs+smax*in+out) == 0) {
            fsm_state_add_arc(0,in,out, 1, 0, 1);
            *(pairs+smax*in+out) = 1;
            pathcount++;
        }
    }
    fsm_state_end_state();
    fsm_state_set_current_state(1, 1, 0);
    fsm_state_end_state();

    xxfree(pairs);
    xxfree(net->states);

    fsm_state_close(net);
    if (pathcount == 0) {
        fsm_destroy(net);
        return(fsm_empty_set());
    }
    net->is_minimized = YES;
    net->is_loop_free = YES;
    net->pathcount = pathcount;
    sigma_cleanup(net, 1);
    return(net);
}

int fsm_sigma_destroy(struct sigma *sigma) {
    struct sigma *sig, *sigp;
    for (sig = sigma, sigp = NULL; sig != NULL; sig = sigp) {
	sigp = sig->next;
	if (sig->symbol != NULL) {
	    xxfree(sig->symbol);
	    sig->symbol = NULL;
	}
	xxfree(sig);
    }
    return 1;
}

int fsm_destroy(struct fsm *net) {
    if (net == NULL) {
        return 0;
    }
    if (net->medlookup != NULL && net->medlookup->confusion_matrix != NULL) {
        xxfree(net->medlookup->confusion_matrix);
	net->medlookup->confusion_matrix = NULL;
    }
    if (net->medlookup != NULL) {
        xxfree(net->medlookup);
	net->medlookup = NULL;
    }
    fsm_sigma_destroy(net->sigma);
    net->sigma = NULL;
    if (net->states != NULL) {
        xxfree(net->states);
	net->states = NULL;
    }
    xxfree(net);
    return(1);
}

struct fsm *fsm_create (char *name) {
  struct fsm *fsm;
  fsm = xxmalloc(sizeof(struct fsm));
  strcpy(fsm->name, name);
  fsm->arity = 1;
  fsm->arccount = 0;
  fsm->is_deterministic = NO;
  fsm->is_pruned = NO;
  fsm->is_minimized = NO;
  fsm->is_epsilon_free = NO;
  fsm->is_loop_free = NO;
  fsm->arcs_sorted_in = NO;
  fsm->arcs_sorted_out = NO;
  fsm->sigma = sigma_create();
  fsm->states = NULL;
  fsm->medlookup = NULL;
  return(fsm);
}

struct fsm *fsm_empty_string() {
  struct fsm *net;
  net = fsm_create("");
  net->states = xxmalloc(sizeof(struct fsm_state)*2);
  add_fsm_arc(net->states, 0, 0, -1, -1, -1, 1, 1);
  add_fsm_arc(net->states, 1, -1, -1, -1, -1, -1, -1);
  fsm_update_flags(net,YES,YES,YES,YES,YES,NO);
  net->statecount = 1;
  net->finalcount = 1;
  net->arccount = 0;
  net->linecount = 2;
  net->pathcount = 1;
  return(net);
}

struct fsm *fsm_identity() {
  struct fsm *net;
  struct sigma *sigma;
  net = fsm_create("");
  xxfree(net->sigma);
  net->states = xxmalloc(sizeof(struct fsm_state)*3);
  add_fsm_arc(net->states, 0, 0, 2, 2, 1, 0, 1);
  add_fsm_arc(net->states, 1, 1, -1, -1, -1, 1, 0);
  add_fsm_arc(net->states, 2, -1, -1, -1, -1, -1, -1);
  sigma = xxmalloc(sizeof(struct sigma));
  sigma->number = IDENTITY;
  sigma->symbol = xxstrdup("@_IDENTITY_SYMBOL_@");
  sigma->next = NULL;
  net->sigma = sigma;
  fsm_update_flags(net,YES,YES,YES,YES,YES,NO);
  net->statecount = 2;
  net->finalcount = 1;
  net->arccount = 1;
  net->linecount = 3;
  net->pathcount = 1;
  return(net);
}

struct fsm *fsm_empty_set() {
  struct fsm *net;
  net = fsm_create("");
  net->states = fsm_empty();
  fsm_update_flags(net,YES,YES,YES,YES,YES,NO);
  net->statecount = 1;
  net->finalcount = 0;
  net->arccount = 0;
  net->linecount = 2;
  net->pathcount = 0;
  return(net);
}

struct fsm_state *fsm_empty() {
  struct fsm_state *new_fsm;
  new_fsm = xxmalloc(sizeof(struct fsm_state)*2);
  add_fsm_arc(new_fsm, 0, 0, -1, -1, -1, 0, 1);
  add_fsm_arc(new_fsm, 1, -1, -1, -1, -1, -1, -1);
  return(new_fsm);
}

int fsm_isuniversal(struct fsm *net) {
    struct fsm_state *fsm;
    net = fsm_minimize(net);
    fsm_compact(net);
    fsm = net->states;
    if ((fsm->target == 0 && fsm->final_state == 1 && (fsm+1)->state_no == 0) && 
        (fsm->in == IDENTITY && fsm->out == IDENTITY) &&
        ((fsm+1)->state_no == -1) &&
        (sigma_max(net->sigma)<3) ) {
        return 1;
    } else {
        return 0;
    }
}

int fsm_isempty(struct fsm *net) {
    struct fsm_state *fsm;
    net = fsm_minimize(net);
    fsm = net->states;
    if (fsm->target == -1 && fsm->final_state == 0 && (fsm+1)->state_no == -1)
        return 1;
    else 
        return 0;
}

int fsm_issequential(struct fsm *net) {
    int i, *sigtable, sequential, seentrans, epstrans, laststate, insym;
    struct fsm_state *fsm;
    sigtable = xxcalloc(sigma_max(net->sigma)+1,sizeof(int));
    for (i = 0 ; i < sigma_max(net->sigma)+1; i++) {
	sigtable[i] = -2;
    }
    fsm = net->states;
    seentrans = epstrans = 0;
    laststate = -1;
    for (sequential = 1, i = 0; (fsm+i)->state_no != -1 ; i++) {
	insym = (fsm+i)->in;
	if (insym < 0) {
	    continue;
	}
	if ((fsm+i)->state_no != laststate) {
	    laststate = (fsm+i)->state_no;
	    epstrans = 0;
	    seentrans = 0;
	}
	if (*(sigtable+insym) == laststate || epstrans == 1) {
	    sequential = 0;
	    break;
	}
	if (insym == EPSILON) {
	    if (epstrans == 1 || seentrans == 1) {
		sequential = 0;
		break;
	    }
	    epstrans = 1;
	}
	*(sigtable+insym) = laststate;
	seentrans = 1;
    }
    xxfree(sigtable);
    if (!sequential)
	printf("fails at state %i\n",(fsm+i)->state_no);
    return(sequential);
}

int fsm_isfunctional(struct fsm *net) {
    return(fsm_isidentity(fsm_minimize(fsm_compose(fsm_invert(fsm_copy(net)),fsm_copy(net)))));
}

int fsm_isunambiguous(struct fsm *net) {
    struct fsm *loweruniqnet, *testnet;
    int ret;
    loweruniqnet = fsm_lowerdet(fsm_copy(net));
    testnet = fsm_minimize(fsm_compose(fsm_invert(fsm_copy(loweruniqnet)),fsm_copy(loweruniqnet)));
    ret = fsm_isidentity(testnet);
    fsm_destroy(loweruniqnet);
    fsm_destroy(testnet);
    return(ret);
}

struct fsm *fsm_extract_ambiguous_domain(struct fsm *net) {
    // define AmbiguousDom(T) [_loweruniq(T) .o. _notid(_loweruniq(T).i .o. _loweruniq(T))].u;
    struct fsm *loweruniqnet, *result;
    loweruniqnet = fsm_lowerdet(net);
    result = fsm_topsort(fsm_minimize(fsm_upper(fsm_compose(fsm_copy(loweruniqnet),fsm_extract_nonidentity(fsm_compose(fsm_invert(fsm_copy(loweruniqnet)), fsm_copy(loweruniqnet)))))));
    fsm_destroy(loweruniqnet);
    sigma_cleanup(result,1);
    fsm_compact(result);
    sigma_sort(result);
    return(result);
}

struct fsm *fsm_extract_ambiguous(struct fsm *net) {
    struct fsm *result;
    result = fsm_topsort(fsm_minimize(fsm_compose(fsm_extract_ambiguous_domain(fsm_copy(net)), net)));
    return(result);
}

struct fsm *fsm_extract_unambiguous(struct fsm *net) {
    struct fsm *result;
    result = fsm_topsort(fsm_minimize(fsm_compose(fsm_complement(fsm_extract_ambiguous_domain(fsm_copy(net))), net)));
    return(result);
}

int fsm_isidentity(struct fsm *net) {

    /* We check whether a given transducer only produces identity relations     */
    /* By doing a DFS on the graph, and storing, for each state a "discrepancy" */
    /* string, showing the current "debt" on the upper or lower side.           */
    /* We immediately fail if: */
    /* a) we encounter an already seen state with a different current           */
    /*    discrepancy than what is stored in the state.                         */
    /* b) when traversing an arc, we encounter a mismatch between the arc and   */
    /*    the current discrepancy.                                              */
    /* c) we encounter a final state and have a non-null current discrepancy.   */
    /* d) we encounter @ with a non-null discrepancy anywhere.                  */
    /* e) we encounter ? anywhere.                                              */
    
    struct discrepancy {
        short int *string;
        short int length;
        _Bool visited;
    };

    struct state_array *state_array;
    struct fsm_state *curr_ptr;
    int i, j, v, vp, num_states, factor = 0, newlength = 1, startfrom;
    short int in, out, *newstring;
    struct discrepancy *discrepancy, *currd, *targetd;

    fsm_minimize(net);
    fsm_count(net);
    
    num_states = net->statecount;
    discrepancy = xxcalloc(num_states,sizeof(struct discrepancy));
    state_array = map_firstlines(net);
    ptr_stack_clear();
    ptr_stack_push(state_array->transitions);

    while(!ptr_stack_isempty()) {

        curr_ptr = ptr_stack_pop();

    nopop:
        v = curr_ptr->state_no; /* source state number */
        vp = curr_ptr->target;  /* target state number */
        currd = discrepancy+v;
        if (v != -1)
            currd->visited = 1;
        if (v == -1 || vp == -1)
            continue;
        in = curr_ptr->in;
        out = curr_ptr->out;

        targetd = discrepancy+vp;
        /* Check arc and conditions e) d) b) */
        /* e) */
        if (in == UNKNOWN || out == UNKNOWN)
            goto fail;
        /* d) */
        if (in == IDENTITY && currd->length != 0)
            goto fail;
        /* b) */
        if (currd->length != 0) {
            if (currd->length > 0 && out != EPSILON && out != *(currd->string))
                goto fail;
            if (currd->length < 0 && in != EPSILON && in  != *(currd->string))
                goto fail;
        }
        if (currd->length == 0 && in != out && in != EPSILON && out != EPSILON) {
            goto fail;
        }

        /* Calculate new discrepancy */
        if (currd->length != 0) {
            if (in != EPSILON && out != EPSILON)
                factor = 0;
            else if (in == EPSILON)
                factor = -1;
            else if (out == EPSILON)
                factor = 1;
            
            newlength = currd->length + factor;
            startfrom = (abs(newlength) <= abs(currd->length)) ? 1 : 0;

        } else if (currd->length == 0) {
            if (in != EPSILON && out != EPSILON) {
                newlength = 0;
            } else {
                newlength = (out == EPSILON) ? 1 : -1;
            }
            startfrom = 0;
        }

        newstring = xxcalloc(abs(newlength),sizeof(int));

        for (i = startfrom, j = 0; i < abs(currd->length); i++, j++) {
            *(newstring+j) = *((currd->string)+i);
        }
        if (newlength != 0) {
            if (currd->length > 0 && newlength >= currd->length) {
                *(newstring+j) = in;
            }
            if (currd->length < 0 && newlength <= currd->length) {
                *(newstring+j) = out;
            }
            if (currd->length == 0 && newlength < currd->length) {
                *(newstring+j) = out;
            }
            if (currd->length == 0 && newlength > currd->length) {
                *(newstring+j) = in;
            }
        }

        /* Check target conditions a) c) */
        /* a) */
        if (((state_array+vp)->transitions)->final_state && newlength != 0)
            goto fail;
        if (curr_ptr->state_no == (curr_ptr+1)->state_no) {
            ptr_stack_push(curr_ptr+1);
        }
        if ((discrepancy+vp)->visited) {
            //xxfree(newstring);
            if (targetd->length != newlength)
                goto fail;
            for (i=0 ; i < abs(newlength); i++) {
                if (*((targetd->string)+i) != *(newstring+i))
                    goto fail;
            }
        } else {
            /* Add discrepancy to target state */
            targetd->length = newlength;
            targetd->string = newstring;
            curr_ptr = (state_array+vp)->transitions;
            goto nopop;
        }
    }
    xxfree(state_array);
    xxfree(discrepancy);
    return 1;
 fail:
    xxfree(state_array);
    xxfree(discrepancy);
    ptr_stack_clear();
    return 0;
}

struct fsm *fsm_markallfinal(struct fsm *net) {
    struct fsm_state *fsm;
    int i;
    fsm = net->states;
    for (i=0; (fsm+i)->state_no != -1; i++) {
        (fsm+i)->final_state = YES;
    }
    return net;
}

struct fsm *fsm_lowerdet(struct fsm *net) {
    unsigned int newsym; /* Running number for new syms */
    struct fsm_state *fsm;
    char repstr[13];
    int i,j,maxsigma,maxarc;
    net = fsm_minimize(net);
    fsm_count(net);
    newsym = 8723643;
    fsm = net->states;
    maxarc = 0;
    maxsigma = sigma_max(net->sigma);

    for (i=0, j=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1)
            j++;
        if ((fsm+i+1)->state_no != (fsm+i)->state_no) {
            maxarc = maxarc > j ? maxarc : j;
            j = 0;
        }
    }
    if (maxarc > (maxsigma-2)) {
        for (i=maxarc; i > (maxsigma-2); i--) {
            sprintf(repstr,"%012X",newsym++);        
            sigma_add(repstr, net->sigma);
        }
        sigma_sort(net);
    }
    for (i=0, j=3; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1) {
            (fsm+i)->out = j++;
            (fsm+i)->in = ((fsm+i)->in == IDENTITY) ? UNKNOWN : (fsm+i)->in;
        }
        if ((fsm+i+1)->state_no != (fsm+i)->state_no) {
            j = 3;
        }
    }
    return(net);
}

struct fsm *fsm_lowerdeteps(struct fsm *net) {
    unsigned int newsym; /* Running number for new syms */
    struct fsm_state *fsm;
    char repstr[13];
    int i,j,maxsigma,maxarc;
    net = fsm_minimize(net);
    fsm_count(net);
    newsym = 8723643;
    fsm = net->states;
    maxarc = 0;
    maxsigma = sigma_max(net->sigma);

    for (i=0, j=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1)
            j++;
        if ((fsm+i+1)->state_no != (fsm+i)->state_no) {
            maxarc = maxarc > j ? maxarc : j;
            j = 0;
        }
    }
    if (maxarc > (maxsigma-2)) {
        for (i=maxarc; i > (maxsigma-2); i--) {
            sprintf(repstr,"%012X",newsym++);        
            sigma_add(repstr, net->sigma);
        }
        sigma_sort(net);
    }
    for (i=0, j=3; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1 && (fsm+i)->out != EPSILON) {
            (fsm+i)->out = j++;
            (fsm+i)->in = ((fsm+i)->in == IDENTITY) ? UNKNOWN : (fsm+i)->in;
        }
        if ((fsm+i+1)->state_no != (fsm+i)->state_no) {
            j = 3;
        }
    }
    return(net);
}

struct fsm *fsm_extract_nonidentity(struct fsm *net) {

    /* Same algorithm as for test identity, except we mark the arcs that cause nonidentity */
    /* Experimental. */

    struct discrepancy {
        short int *string;
        short int length;
        _Bool visited;
    };

    struct state_array *state_array;
    struct fsm_state *curr_ptr;
    struct fsm *net2;
    int i, j, v, vp, num_states, factor = 0, newlength = 1, startfrom, killnum;
    short int in, out, *newstring;
    struct discrepancy *discrepancy, *currd, *targetd;

    fsm_minimize(net);
    fsm_count(net);
    killnum = sigma_add("@KILL@", net->sigma);
    
    num_states = net->statecount;
    discrepancy = xxcalloc(num_states,sizeof(struct discrepancy));
    state_array = map_firstlines(net);
    ptr_stack_push(state_array->transitions);

    while(!ptr_stack_isempty()) {

        curr_ptr = ptr_stack_pop();

    nopop:
        v = curr_ptr->state_no; /* source state number */
        vp = curr_ptr->target;  /* target state number */
        currd = discrepancy+v;
        if (v != -1)
            currd->visited = 1;
        if (v == -1 || vp == -1)
            continue;
        in = curr_ptr->in;
        out = curr_ptr->out;

        targetd = discrepancy+vp;
        /* Check arc and conditions e) d) b) */
        /* e) */
        if (in == UNKNOWN || out == UNKNOWN)
            goto fail;
        /* d) */
        if (in == IDENTITY && currd->length != 0)
            goto fail;
        /* b) */
        if (currd->length != 0) {
            if (currd->length > 0 && out != EPSILON && out != *(currd->string))
                goto fail;
            if (currd->length < 0 && in != EPSILON && in  != *(currd->string))
                goto fail;
        }
        if (currd->length == 0 && in != out && in != EPSILON && out != EPSILON) {
            goto fail;
        }

        /* Calculate new discrepancy */
        if (currd->length != 0) {
            if (in != EPSILON && out != EPSILON)
                factor = 0;
            else if (in == EPSILON)
                factor = -1;
            else if (out == EPSILON)
                factor = 1;
            
            newlength = currd->length + factor;
            startfrom = (abs(newlength) <= abs(currd->length)) ? 1 : 0;

        } else if (currd->length == 0) {
            if (in != EPSILON && out != EPSILON) {
                newlength = 0;
            } else {
                newlength = (out == EPSILON) ? 1 : -1;
            }
            startfrom = 0;
        }

        newstring = xxcalloc(abs(newlength),sizeof(int));

        for (i = startfrom, j = 0; i < abs(currd->length); i++, j++) {
            *(newstring+j) = *((currd->string)+i);
        }
        if (newlength != 0) {
            if (currd->length > 0 && newlength >= currd->length) {
                *(newstring+j) = in;
            }
            if (currd->length < 0 && newlength <= currd->length) {
                *(newstring+j) = out;
            }
            if (currd->length == 0 && newlength < currd->length) {
                *(newstring+j) = out;
            }
            if (currd->length == 0 && newlength > currd->length) {
                *(newstring+j) = in;
            }

        }

        /* Check target conditions a) c) */
        /* a) */
        if (((state_array+vp)->transitions)->final_state && newlength != 0)
            goto fail;
        if (curr_ptr->state_no == (curr_ptr+1)->state_no) {
            ptr_stack_push(curr_ptr+1);
        }

        if ((discrepancy+vp)->visited) {
            //xxfree(newstring);
            if (targetd->length != newlength)
                goto fail;
            for (i=0 ; i < abs(newlength); i++) {
                if (*((targetd->string)+i) != *(newstring+i))
                    goto fail;
            }
        } else {
            /* Add discrepancy to target state */
            targetd->length = newlength;
            targetd->string = newstring;
            curr_ptr = (state_array+vp)->transitions;
            goto nopop;
        }
        continue;
    fail:        
        curr_ptr->out = killnum;
        if (curr_ptr->state_no == (curr_ptr+1)->state_no) {
            ptr_stack_push(curr_ptr+1);
        }        
    }
    ptr_stack_clear();
    sigma_sort(net);
    net2 = fsm_upper(fsm_compose(net,fsm_contains(fsm_symbol("@KILL@"))));
    sigma_remove("@KILL@",net2->sigma);
    sigma_sort(net2);
    xxfree(state_array);
    xxfree(discrepancy);
    return(net2);
}

struct fsm *fsm_copy (struct fsm *net) {
    struct fsm *net_copy;
    if (net == NULL)
        return net;

    net_copy = xxmalloc(sizeof(struct fsm));
    memcpy(net_copy, net, sizeof(struct fsm));

    fsm_count(net);
    net_copy->sigma = sigma_copy(net->sigma);
    net_copy->states = fsm_state_copy(net->states, net->linecount);      
    return(net_copy);
}

struct fsm_state *fsm_state_copy(struct fsm_state *fsm_state, int linecount) {
  struct fsm_state *new_fsm_state;
  
  new_fsm_state = xxmalloc(sizeof(struct fsm_state)*(linecount));
  memcpy(new_fsm_state, fsm_state, linecount*sizeof(struct fsm_state));
  return(new_fsm_state);
}

/* TODO: separate linecount and arccount */
int find_arccount(struct fsm_state *fsm) {
  int i;
  for (i=0;(fsm+i)->state_no != -1; i++) {
  }
  return i;
}

void clear_quantifiers() {
    quantifiers = NULL;
}

int count_quantifiers() {
    struct defined_quantifiers *q;
    int i;
    for (i = 0, q = quantifiers; q != NULL; q = q->next) {
	i++;
    }
    return(i);
}

void add_quantifier (char *string) {
    struct defined_quantifiers *q;
    if (quantifiers == NULL) {
	q = xxmalloc(sizeof(struct defined_quantifiers));
	quantifiers = q;
    } else { 
	for (q = quantifiers; q->next != NULL; q = q->next) {
	    
	}
	q->next = xxmalloc(sizeof(struct defined_quantifiers));
	q = q->next;
    }
    q->name = xxstrdup(string);
    q->next = NULL;
}

struct fsm *union_quantifiers() {
/*     We create a FSM that simply accepts the union of all */
/*     quantifier symbols */
    
    struct fsm *net;
    struct defined_quantifiers *q;
    int i, syms, s, symlo;
    
    net = fsm_create("");
    fsm_update_flags(net, YES, YES, YES, YES, NO, NO);
    
    for (syms = 0, symlo = 0, q = quantifiers; q != NULL; q = q->next) {
      s = sigma_add(q->name, net->sigma);
      if (symlo == 0) {
	symlo = s;
      }
      syms++;
    }
    net->states = xxmalloc(sizeof(struct fsm_state)*(syms+1));
    for (i = 0; i < syms; i++) {
	add_fsm_arc(net->states, i, 0, symlo+i, symlo+i, 0, 1, 1);
    }
    add_fsm_arc(net->states, i, -1, -1, -1, -1 ,-1 ,-1);
    net->arccount = syms;
    net->statecount = net->finalcount = 1;
    net->linecount = syms;
    return(net);
}

char *find_quantifier (char *string) {
    struct defined_quantifiers *q;
    for (q = quantifiers; q != NULL; q = q->next) {
	if (strcmp(string,q->name) == 0)
	    return q->name;
    }
    return(NULL);
}

void purge_quantifier (char *string) {
    struct defined_quantifiers *q, *q_prev;    
    for (q = quantifiers, q_prev = NULL; q != NULL; q_prev = q, q = q->next) {
	if (strcmp(string, q->name) == 0) {
	    if (q_prev != NULL) {
		q_prev->next = q->next;
	    } else {
		quantifiers = q->next;
	    }
	}
    }
}

struct fsm *fsm_quantifier(char *string) {

    /* \x* x \x* x \x* */
    return(fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol(string))),fsm_concat(fsm_symbol(string),fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol(string))),fsm_concat(fsm_symbol(string),fsm_kleene_star(fsm_term_negation(fsm_symbol(string))))))));

}

struct fsm *fsm_logical_precedence(char *string1, char *string2) {
    /* x < y = \y* x \y* [x | y Q* x] ?* */
    /*          1  2  3        4           5 */
    
    return(fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol(string2))),fsm_concat(fsm_symbol(string1),fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol(string2))),fsm_concat(fsm_union(fsm_symbol(string1),fsm_concat(fsm_symbol(string2),fsm_concat(union_quantifiers(),fsm_symbol(string1)))),fsm_universal())))));

/*    1,3   fsm_kleene_star(fsm_term_negation(fsm_symbol(string2))) */
/*        2 = fsm_symbol(string1) */
/*        5 = fsm_universal() */
/* 4 =    fsm_union(fsm_symbol(string1),fsm_concat(fsm_symbol(string2),fsm_concat(union_quantifiers(),fsm_symbol(string1)))) */

}

/** Logical equivalence, i.e. where two variables span the same substring */
/** x = y = ?* [x y|y x]/Q ?* [x y|y x]/Q ?* */
struct fsm *fsm_logical_eq(char *string1, char *string2) {
  return(fsm_concat(fsm_universal(),fsm_concat(fsm_ignore(fsm_union(fsm_concat(fsm_symbol(string1),fsm_symbol(string2)),fsm_concat(fsm_symbol(string2),fsm_symbol(string1))),union_quantifiers(),OP_IGNORE_ALL),fsm_concat(fsm_universal(),fsm_concat(fsm_ignore(fsm_union(fsm_concat(fsm_symbol(string1),fsm_symbol(string2)),fsm_concat(fsm_symbol(string2),fsm_symbol(string1))),union_quantifiers(),OP_IGNORE_ALL),fsm_universal())))));
}

