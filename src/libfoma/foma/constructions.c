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

#define KLEENE_STAR 0
#define KLEENE_PLUS 1
#define OPTIONALITY 2

#define COMPLEMENT 0
#define COMPLETE 1

#define STACK_3_PUSH(a,b,c) int_stack_push(a); int_stack_push(b); int_stack_push(c);
#define STACK_2_PUSH(a,b) int_stack_push(a); int_stack_push(b);

struct mergesigma {
  char *symbol;
  unsigned char presence; /* 1 = in net 1, 2 = in net 2, 3 = in both */
  int number;
  struct mergesigma *next;
};

int sort_cmp(const void *a, const void *b) {
  return (((const struct fsm_state *)a)->state_no - ((const struct fsm_state *)b)->state_no);
}

INLINE int add_fsm_arc(struct fsm_state *fsm, int offset, int state_no, int in, int out, int target, int final_state, int start_state);

struct fsm *fsm_kleene_closure(struct fsm *net, int optionality);

struct fsm *fsm_kleene_star(struct fsm *net) {
  return (fsm_kleene_closure(net, KLEENE_STAR));
}

struct fsm *fsm_kleene_plus(struct fsm *net) {
  return (fsm_kleene_closure(net, KLEENE_PLUS));
}

struct fsm *fsm_optionality(struct fsm *net) {
  return (fsm_kleene_closure(net, OPTIONALITY));
}

struct fsm *fsm_escape(char *symbol) {
  return(fsm_symbol(symbol+1));
}

/* Convert a multicharacter-string-containing machine */
/* to the equivalent "letter" machine where all arcs  */
/* are single utf8 letters.                           */

struct fsm *fsm_letter_machine(struct fsm *net) {
   
    struct fsm *outnet;
    struct fsm_read_handle *inh;
    struct fsm_construct_handle *outh;
    int i, steps, source, target, addstate, innum, outnum, inlen, outlen;
    char *in, *out, *currin, *currout, tmpin[128], tmpout[128];

    inh = fsm_read_init(fsm_minimize(net));
    outh = fsm_construct_init("name");
    addstate = net->statecount;

    while (fsm_get_next_arc(inh)) {
        in = fsm_get_arc_in(inh);
	out = fsm_get_arc_out(inh);
	innum = fsm_get_arc_num_in(inh);
	outnum = fsm_get_arc_num_out(inh);
	source = fsm_get_arc_source(inh);
	target = fsm_get_arc_target(inh);
	
	if (((innum > IDENTITY) && utf8strlen(in) > 1) || ((outnum > IDENTITY) && utf8strlen(out) > 1)) {
	    inlen = innum <= IDENTITY ? 1 : utf8strlen(in);
	    outlen = outnum <= IDENTITY ? 1 : utf8strlen(out);
	    steps = inlen > outlen ? inlen : outlen;

	    target = addstate;
	    for (i = 0; i < steps; i++) {
		if (innum <= IDENTITY || inlen < 1) {
		    if (inlen < 1)
			currin = "@_EPSILON_SYMBOL_@";
		    else 
			currin = in;
		} else {
		    strncpy(tmpin, in, utf8skip(in)+1);
		    *(tmpin+utf8skip(in)+1) = '\0';
		    currin = tmpin;
		    inlen--;
		    in = in+utf8skip(in)+1;
		}
		if (outnum <= IDENTITY || outlen < 1) {
		    if (outlen < 1)
			currout = "@_EPSILON_SYMBOL_@";
		    else
			currout = out;
		} else {
		    strncpy(tmpout, out, utf8skip(in)+1);
		    *(tmpout+utf8skip(out)+1) = '\0';
		    currout = tmpout;
		    out = out+utf8skip(out)+1;
		    outlen--;
		}
		if (i == 0 && steps > 1) {		    
		    target = addstate;
		    addstate++;
		}
		if (i > 0 && (steps-i == 1)) {
		    source = addstate - 1;
		    target = fsm_get_arc_target(inh);
		}
		if (i > 0 && (steps-i != 1)) {
		    source = addstate-1;
		    target = addstate;
		    addstate++;
		}
		fsm_construct_add_arc(outh,source,target,currin,currout);
	    }
	} else {
	    fsm_construct_add_arc(outh,source,target,in,out);
	}
    }
    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(outh, i);
    }
    while ((i = fsm_get_next_initial(inh)) != -1) {
	fsm_construct_set_initial(outh, i);
    }
    fsm_read_done(inh);
    outnet = fsm_construct_done(outh);
    return(outnet);
}

struct fsm *fsm_explode(char *symbol) {
    struct fsm *net;
    struct fsm_construct_handle *h;
    char *tempstring;
    int length, i, j, skip;

    h = fsm_construct_init("");
    fsm_construct_set_initial(h,0);
    
    length = strlen(symbol)-2;
    for (i=1, j=1; i <= length; i += skip, j++) {
	skip = utf8skip(symbol+i)+1;
	tempstring = xxstrndup(((symbol+i)),skip);
	fsm_construct_add_arc(h,j-1,j,tempstring,tempstring);
	xxfree(tempstring);
    }
    fsm_construct_set_final(h, j-1);
    net = fsm_construct_done(h);
    return(net);
}

struct fsm *fsm_symbol(char *symbol) {
  struct fsm *net;
  int symbol_no;

  net = fsm_create("");
  fsm_update_flags(net, YES, YES, YES, YES, YES, NO);
  if (strcmp(symbol,"@_EPSILON_SYMBOL_@")==0) {
    /* Epsilon */
    (void)sigma_add_special(EPSILON, net->sigma);
    net->states = xxmalloc(sizeof(struct fsm_state)*2);
    add_fsm_arc(net->states, 0, 0, -1,-1,-1,1,1);
    add_fsm_arc(net->states, 1, -1,-1,-1,-1,-1,-1);
    net->arccount = 0;
    net->statecount = 1;
    net->linecount = 1;
    net->finalcount = 1;
    net->is_deterministic = NO;
    net->is_minimized = NO;
    net->is_epsilon_free = NO;
  } else {
    if ((strcmp(symbol,"@_IDENTITY_SYMBOL_@") == 0)) {
      symbol_no = sigma_add_special(IDENTITY,net->sigma);
    } else {
      symbol_no = sigma_add(symbol,net->sigma);
    }
    net->states = xxmalloc(sizeof(struct fsm_state)*3);
    add_fsm_arc(net->states, 0, 0, symbol_no, symbol_no, 1, 0, 1);
    add_fsm_arc(net->states, 1, 1, -1, -1, -1, 1, 0);
    add_fsm_arc(net->states, 2, -1, -1, -1, -1, -1, -1);
    net->arity = 1;
    net->pathcount = 1;
    net->arccount = 1;
    net->statecount = 2;
    net->linecount = 2;
    net->finalcount = 1;
    net->arcs_sorted_in = YES;
    net->arcs_sorted_out = YES;
    net->is_deterministic = YES;
    net->is_minimized = YES;
    net->is_epsilon_free = YES;
  }
  return(net);
}

void fsm_sort_lines(struct fsm *net) {
  struct fsm_state *fsm;
  fsm = net->states;
  qsort(fsm, find_arccount(fsm), sizeof(struct fsm_state), sort_cmp);
}

void fsm_update_flags(struct fsm *net, int det, int pru, int min, int eps, int loop, int completed) {
  net->is_deterministic = det;
  net->is_pruned = pru;
  net->is_minimized = min;
  net->is_epsilon_free = eps;
  net->is_loop_free = loop;
  net->is_completed = completed;
  net->arcs_sorted_in = NO;
  net->arcs_sorted_out = NO;
}

int fsm_count_states(struct fsm_state *fsm) {
  int i, temp = -1, states = 0;
  for(i=0; (fsm+i)->state_no != -1; i++) {
    if (temp != (fsm+i)->state_no) {
      states++;
      temp = (fsm+i)->state_no;
    }
  }
  return(states);
}

struct state_arr {
  int final;
  int start;
  struct fsm_state *transitions;
};

struct state_arr *init_state_pointers(struct fsm_state *fsm_state) {

  /* Create an array for quick lookup of whether states are final, and a pointer to the first line regarding each state */

  struct state_arr *state_arr;
  int states, i, sold;
  sold = -1;
  states = fsm_count_states(fsm_state);
  state_arr = xxmalloc(sizeof(struct state_arr)*(states+1));
  for (i=0; i<states; i++) {
    (state_arr+i)->final = 0;
    (state_arr+i)->start = 0;
  }
  
  for (i=0; (fsm_state+i)->state_no != -1; i++) {
    if ((fsm_state+i)->final_state == 1)
      (state_arr+((fsm_state+i)->state_no))->final = 1;
    if ((fsm_state+i)->start_state == 1)
      (state_arr+((fsm_state+i)->state_no))->start = 1;
    if ((fsm_state+i)->state_no != sold) {
      (state_arr+((fsm_state+i)->state_no))->transitions = (fsm_state+i);
      sold = (fsm_state+i)->state_no;
    }
  }  
  return(state_arr);
}

/* An open addressing scheme (with linear probing) to store triplets of ints */
/* and hashing them with an automatically increasing key at every insert     */
/* The table is rehashed whenever occupancy reaches 0.5                      */

struct triplethash_triplets {
    int a;
    int b;
    int c;
    int key;
};

struct triplethash {
    struct triplethash_triplets *triplets;
    unsigned int tablesize;
    int occupancy;
};

struct triplethash *triplet_hash_init() {
    struct triplethash *th;
    int i;
    th = xxmalloc(sizeof(struct triplethash));
    th->tablesize = 128;
    th->occupancy = 0;
    th->triplets = xxmalloc(sizeof(struct triplethash_triplets) * th->tablesize);
    for (i = 0; i < th->tablesize; i++) {
	(th->triplets+i)->key = -1;
    }
    return(th);
}

unsigned int triplethash_hashf(int a, int b, int c) {
    return((unsigned int)(a * 7907 + b * 86028157 + c * 7919));
}

void triplet_hash_free(struct triplethash *th) {
    if (th != NULL) {
	if (th->triplets != NULL) {
	    xxfree(th->triplets);
	}
	xxfree(th);
    }    
}

void triplet_hash_rehash(struct triplethash *th);

void triplet_hash_insert_with_key(struct triplethash *th, int a, int b, int c, int key) {
    struct triplethash_triplets *th_table;
    unsigned int hash;
    th_table = th->triplets;
    hash = triplethash_hashf(a, b, c) % th->tablesize;
    for (;;) {
	if ((th_table + hash)->key == -1) {
	    (th_table + hash)->key = key;
	    (th_table + hash)->a = a;
	    (th_table + hash)->b = b;
	    (th_table + hash)->c = c;
	    break;
	}
	hash++;
	if (hash >= th->tablesize)
	    hash -= th->tablesize;
    }
}

int triplet_hash_insert(struct triplethash *th, int a, int b, int c) {
    struct triplethash_triplets *th_table;
    unsigned int hash;
    th_table = th->triplets;
    hash = triplethash_hashf(a,b,c) % th->tablesize;
    for (;;) {
	if ((th_table + hash)->key == - 1) {
	    (th_table + hash)->key = th->occupancy;
	    (th_table + hash)->a = a;
	    (th_table + hash)->b = b;
	    (th_table + hash)->c = c;
	    th->occupancy = th->occupancy + 1;
	    if (th->occupancy > th->tablesize / 2) {
		triplet_hash_rehash(th);
	    }
	    return(th->occupancy - 1);
	}
	hash++;
	if (hash >= th->tablesize)
	    hash -= th->tablesize;
    }
}

void triplet_hash_rehash(struct triplethash *th) {
    int i;
    unsigned int newtablesize, oldtablesize;
    struct triplethash_triplets *oldtriplets;
    newtablesize = th->tablesize * 2;
    oldtablesize = th->tablesize;
    oldtriplets = th->triplets;
    th->triplets = xxmalloc(sizeof(struct triplethash_triplets) * newtablesize);
    th->tablesize = newtablesize;
    for (i = 0; i < newtablesize; i++) {
	(th->triplets+i)->key = -1;
    }
    for (i = 0; i < oldtablesize; i++) {
	if ((oldtriplets+i)-> key != -1) {
	    triplet_hash_insert_with_key(th, (oldtriplets+i)->a, (oldtriplets+i)->b, (oldtriplets+i)->c, (oldtriplets+i)->key);
	}
    }
    xxfree(oldtriplets);
}

int triplet_hash_find(struct triplethash *th, int a, int b, int c) {
    struct triplethash_triplets *th_table;
    unsigned int hash, j;
    th_table = th->triplets;
    hash = triplethash_hashf(a, b, c) % th->tablesize;
    for (j = 0; j < th->tablesize; j++) {
	if ((th_table + hash)->key == - 1)
	    return -1;
	if ((th_table + hash)->a == a && (th_table + hash)->b == b && (th_table + hash)->c == c) {
	    return((th_table + hash)->key);
	}
	hash++;
	if (hash >= th->tablesize)
	    hash -= th->tablesize;
    }
    return -1;
}

struct fsm *fsm_intersect(struct fsm *net1, struct fsm *net2) {

    struct blookup {int mainloop; int target; } *array, *bptr;
    int a,b,current_state, current_start, current_final, target_number, sigma2size, mainloop;
    struct fsm_state *machine_a, *machine_b;
    struct state_arr *point_a, *point_b;
    struct fsm *new_net;
    struct triplethash *th;

    net1 = fsm_minimize(net1);
    net2 = fsm_minimize(net2);

    if (fsm_isempty(net1) || fsm_isempty(net2)) {
	fsm_destroy(net1);
	fsm_destroy(net2);
	return(fsm_empty_set());
    }

    fsm_merge_sigma(net1, net2);
    
    fsm_update_flags(net1, YES, NO, UNK, YES, UNK, UNK);
    
    machine_a = net1->states;
    machine_b = net2->states;

    sigma2size = sigma_max(net2->sigma)+1;
    array = xxcalloc(sigma2size*sigma2size, sizeof(struct blookup));
    mainloop = 0;

    /* Intersect two networks by the running-in-parallel method */
    /* new state 0 = {0,0} */
    
    STACK_2_PUSH(0,0);
    
    th = triplet_hash_init();
    triplet_hash_insert(th, 0, 0, 0);

    fsm_state_init(sigma_max(net1->sigma));
    
    point_a = init_state_pointers(machine_a);
    point_b = init_state_pointers(machine_b);
    
    while (!int_stack_isempty()) {
        
        /* Get a pair of states to examine */
        
        a = int_stack_pop();
        b = int_stack_pop();
        
	current_state = triplet_hash_find(th, a, b, 0);
        current_start = (((point_a+a)->start == 1) && ((point_b+b)->start == 1)) ? 1 : 0;
        current_final = (((point_a+a)->final == 1) && ((point_b+b)->final == 1)) ? 1 : 0;
        
        fsm_state_set_current_state(current_state, current_final, current_start);

        /* Create a lookup index for machine b */
        /* array[in][out] holds the target for this state and the symbol pair in:out */
        /* Also, we keep track of whether an entry is fresh by the mainloop counter */
        /* so we don't mistakenly use an old entry and don't have to clear the table */
        /* between each state pair we encounter */

        for (mainloop++, machine_b = (point_b+b)->transitions; machine_b->state_no == b; machine_b++) {
            if (machine_b->in < 0) continue;
            bptr = array+(machine_b->in*sigma2size)+machine_b->out;
            bptr->mainloop = mainloop;
            bptr->target = machine_b->target;            
        }

        /* The main loop where we run the machines in parallel */
        /* We look at each transition of a in this state, and consult the index of b */
        /* we just created */

        for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a ; machine_a++) {
            if (machine_a->in < 0 || machine_a->out < 0) continue;
            bptr = array+(machine_a->in*sigma2size)+machine_a->out;

            if (bptr->mainloop != mainloop)
                continue;
                
            if ((target_number = triplet_hash_find(th, machine_a->target, bptr->target, 0)) == -1) {
                STACK_2_PUSH(bptr->target, machine_a->target);
                target_number = triplet_hash_insert(th, machine_a->target, bptr->target, 0);		
            }
            
            fsm_state_add_arc(current_state, machine_a->in, machine_a->out, target_number, current_final, current_start);
            
        }
        fsm_state_end_state();
    }
    new_net = fsm_create("");
    fsm_sigma_destroy(new_net->sigma);
    new_net->sigma = net1->sigma;
    net1->sigma = NULL;
    fsm_destroy(net2);
    fsm_destroy(net1);
    fsm_state_close(new_net);
    xxfree(point_a);
    xxfree(point_b);
    xxfree(array);
    triplet_hash_free(th);
    return(fsm_coaccessible(new_net));
}

struct fsm *fsm_compose(struct fsm *net1, struct fsm *net2) {

    
    /* The composition algorithm is the basic naive composition where we lazily      */
    /* take the cross-product of states P and Q and move to a new state with symbols */
    /* ain, bout if the symbols aout = bin.  Also, if aout = 0 state p goes to       */
    /* its target, while q stays.  Similarly, if bin = 0, q goes to its target       */
    /* while p stays.                                                                */

    /* We have two variants of the algorithm to avoid creating multiple paths:       */
    /* 1) Bistate composition.  In this variant, when we create a new state, we call it */
    /*    (p,q,mode) where mode = 0 or 1, depending on what kind of an arc we followed  */
    /*    to get there.  If we followed an x:y arc where x and y are both real symbols  */
    /*    we always go to mode 0, however, if we followed an 0:y arc, we go to mode 1.  */
    /*    from mode 1, we do not follow x:0 arcs.  Each (p,q,mode) is unique, and       */
    /*    from (p,q,X) we always consider the transitions from p and q.                 */
    /*    We never create arcs (x:0 0:y) yielding x:y.                                  */

    /* 2) Tristate composition. Here we always go to mode 0 with a x:y arc.             */
    /*    (x:0,0:y) yielding x:y is allowed, but only in mode 0                         */
    /*    (x:y y:z) is always allowed and results in target = mode 0                    */
    /*    0:y arcs lead to mode 2, and from there we stay in mode 2 with 0:y            */
    /*    in mode 2 we only consider 0:y and x:y arcs                                   */
    /*    x:0 arcs lead to mode 1, and from there we stay in mode 1 with x:0            */
    /*    in mode 1 we only consider x:0 and x:y arcs                                   */

    /* It seems unsettled which type of composition is better.  Tristate is similar to  */
    /* the filter transducer given in Mohri, Pereira and Riley (1996) and works well    */
    /* for cases such as [a:0 b:0 c:0 .o. 0:d 0:e 0:f], yielding the shortest path.     */
    /* However, for generic cases, bistate seems to yield smaller transducers.          */
    /* The global variable g_compose_tristate is set to OFF by default                  */

    struct outarray {
        short int symin;
        short int symout;
        int target;
        int mainloop;
    } *outarray, *iptr, *currtail;

    struct index {
        struct outarray *tail;
    } *index;

    extern int g_compose_tristate, g_flag_is_epsilon;
    int a,b,i,mainloop,current_state, current_start, current_final, target_number, ain, bin, aout, bout, asearch, max2sigma;
    struct fsm_state *machine_a, *machine_b;
    struct state_arr *point_a, *point_b;
    struct triplethash *th;
    int mode;
    _Bool *is_flag = NULL;


    net1 = fsm_minimize(net1);
    net2 = fsm_minimize(net2);

    if (fsm_isempty(net1) || fsm_isempty(net2)) {
	fsm_destroy(net1);
	fsm_destroy(net2);
	return(fsm_empty_set());
    }
    
    /* If flag-is-epsilon is on, we need to add the flag symbols    */
    /* in both networks to each other's sigma so that UNKNOWN       */
    /* or IDENTITY symbols do not match these flags, since they are */
    /* supposed to have the behavior of EPSILON                     */
    /* And we need to do this before merging the sigmas, of course  */

    if (g_flag_is_epsilon) {
        struct sigma *sig1, *sig2;
        int flags1, flags2;
        flags1 = flags2 = 0;
        sig2 = net2->sigma;
        max2sigma = sigma_max(net2->sigma);
        for (sig1 = net1->sigma; sig1 != NULL; sig1 = sig1->next) {
            if (flag_check(sig1->symbol)) {
                flags1 = 1;
                if (sigma_find(sig1->symbol, sig2) == -1) {                    
                    sigma_add(sig1->symbol, sig2);
                }
            }
        }

        sig1 = net1->sigma;
        for (sig2 = net2->sigma; sig2 != NULL ; sig2 = sig2->next) {
            if (flag_check(sig2->symbol)) {
                if (sig2->number <= max2sigma) {
                    flags2 = 1;
                }
                if (sigma_find(sig2->symbol, sig1) == -1) {
                    sigma_add(sig2->symbol, sig1);
                }
            }
        }
        sigma_sort(net2);        
        sigma_sort(net1);
        if (flags1 && flags2) {
            printf("***Warning: flag-is-epsilon is ON and both networks contain flags in composition.  This may yield incorrect results.  Set flag-is-epsilon to OFF.\n");
        }
    }

    fsm_merge_sigma(net1, net2);

    if (g_flag_is_epsilon) {
        /* Create lookup table for quickly checking if a symbol is a flag */
        struct sigma *sig1;
        is_flag = xxmalloc(sizeof(_Bool)*(sigma_max(net1->sigma)+1));
        for (sig1 = net1->sigma; sig1 != NULL; sig1=sig1->next) {
            if (flag_check(sig1->symbol)) {
                *(is_flag+(sig1->number)) = 1;
            } else {
                *(is_flag+(sig1->number)) = 0;
            }
        }
    }

    fsm_update_flags(net1, YES, NO, UNK, YES, UNK, UNK);
    
    machine_a = net1->states;
    machine_b = net2->states;    

    max2sigma = sigma_max(net2->sigma);

    /* Create an index for looking up input symbols in machine b quickly */
    /* We store each machine_b->in symbol in outarray[symin][...] */
    /* the array index[symin] points to the tail of the current list in outarray */
    /* (we may have many entries for one input symbol as there may be many outputs */
    /* The field mainloop tells us whether the entry is current as we want to loop */
    /* UNKNOWN and IDENTITY are indexed as UNKNOWN because we need to find both */
    /* as they share some semantics */

    index = xxcalloc(max2sigma+1, sizeof(struct index));
    outarray = xxcalloc((max2sigma+2)*(max2sigma+1), sizeof(struct outarray));

    for (i=0; i <= max2sigma; i++) {
        (index+i)->tail = outarray + ((max2sigma+2)*i);
    }


    /* Mode, a, b */
    STACK_3_PUSH(0,0,0);

    th = triplet_hash_init();
    triplet_hash_insert(th, 0, 0, 0);

    fsm_state_init(sigma_max(net1->sigma));
    
    point_a = init_state_pointers(machine_a);
    point_b = init_state_pointers(machine_b);
    
    mainloop = 0;

    while (!int_stack_isempty()) {
        
        /* Get a pair of states to examine */
        
        a = int_stack_pop();
        b = int_stack_pop();
        mode = int_stack_pop();
        
	current_state = triplet_hash_find(th, a,b,mode);
        current_start = (((point_a+a)->start == 1) && ((point_b+b)->start == 1) && (mode == 0)) ? 1 : 0;
        current_final = (((point_a+a)->final == 1) && ((point_b+b)->final == 1)) ? 1 : 0;
        
        fsm_state_set_current_state(current_state, current_final, current_start);

        /* Create the index for machine b in this state */
        for (mainloop++, machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
            int bindex;
            /* Index b */
            bindex = (machine_b->in == IDENTITY) ? UNKNOWN : machine_b->in;
            if (bindex < 0 || machine_b->target < 0)
                continue;

            iptr = (index+bindex)->tail;
            if (iptr->mainloop != mainloop) {
                iptr = (index+bindex)->tail = outarray+(bindex*(max2sigma+2));
            } else {
                iptr++;
            }
            iptr->symin = machine_b->in;
            iptr->symout = machine_b->out;
            iptr->mainloop = mainloop;
            iptr->target = machine_b->target;
            (index+bindex)->tail = iptr;
        }

        for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a ; machine_a++) {

            /* If we have the same transition from (a,b)-> some state */
            /* If we have x:y y:z trans to some state */
            aout = machine_a->out;
            ain = machine_a->in;
            /* IDENTITY is indexed under UNKNOWN (see above) */
            asearch = (aout == IDENTITY) ? UNKNOWN : aout;
            if (aout < 0) continue;
            iptr = outarray+(asearch*(max2sigma+2));
            currtail = (index+asearch)->tail + 1;
            for ( ; iptr != currtail && iptr->mainloop == mainloop ; iptr++) {
                
                bin = iptr->symin;
                bout = iptr->symout;
                
                if (aout == IDENTITY && bin == UNKNOWN) {
                    ain = aout = UNKNOWN;
                }
                else if (aout == UNKNOWN && bin == IDENTITY) {
                    bin = bout = UNKNOWN;
                }
                
                if (!g_compose_tristate) {
                    if (bin == aout && bin != -1 && bin != EPSILON) {
                        /* mode -> 0 */
                        if ((target_number = triplet_hash_find(th, machine_a->target, iptr->target, 0)) == -1) {
                            STACK_3_PUSH(0, iptr->target, machine_a->target);
                            target_number = triplet_hash_insert(th, machine_a->target, iptr->target, 0);
                        }
                        
                        fsm_state_add_arc(current_state, ain, bout, target_number, current_final, current_start);
                    }
                }

                else if (g_compose_tristate) {
                    if (bin == aout && bin != -1 && ((bin != EPSILON || mode == 0))) {
                        /* mode -> 0 */
                        if ((target_number = triplet_hash_find(th, machine_a->target, iptr->target, 0)) == -1) {
                            STACK_3_PUSH(0, iptr->target, machine_a->target);
                            target_number = triplet_hash_insert(th, machine_a->target, iptr->target, 0);
                        }
                        
                        fsm_state_add_arc(current_state, ain, bout, target_number, current_final, current_start);
                    }
                }
                                
            }
        }
        
        /* Treat epsilon outputs on machine a (may include flags) */
        for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a ; machine_a++) {            
            aout = machine_a->out;
            if (aout != EPSILON && g_flag_is_epsilon == 0)
                continue;
            ain = machine_a->in;

            if (g_flag_is_epsilon && aout != -1 && mode == 0 && *(is_flag+aout)) {
                if ((target_number = triplet_hash_find(th, machine_a->target, b, 0)) == -1) {
                    STACK_3_PUSH(0, b, machine_a->target);
		    target_number = triplet_hash_insert(th, machine_a->target, b, 0);
                }
                fsm_state_add_arc(current_state, ain, aout, target_number, current_final, current_start);
            }

            if (!g_compose_tristate) {
                /* Check A:0 arcs on upper side */
                if (aout == EPSILON && mode == 0) {
                    /* mode -> 0 */        
                    if ((target_number = triplet_hash_find(th, machine_a->target, b, 0)) == -1) {
                        STACK_3_PUSH(0, b, machine_a->target);
                        target_number = triplet_hash_insert(th, machine_a->target, b, 0);                    
                    }
                    
                    fsm_state_add_arc(current_state, ain, EPSILON, target_number, current_final, current_start);
                }
            }

            else if (g_compose_tristate) {
                if (aout == EPSILON && (mode != 2)) {
                    /* mode -> 1 */
                    if ((target_number = triplet_hash_find(th, machine_a->target, b, 1)) == -1) {
                        STACK_3_PUSH(1, b, machine_a->target);
                        target_number = triplet_hash_insert(th, machine_a->target, b, 1);                    
                    }
                    
                    fsm_state_add_arc(current_state, ain, EPSILON, target_number, current_final, current_start);
                    
                }
            }
            
        }
        /* Treat epsilon inputs on machine b (may include flags) */
        for (machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
            bin = machine_b->in;
            if (bin != EPSILON && g_flag_is_epsilon == 0)
                continue;

            bout = machine_b->out;
            
            if (g_flag_is_epsilon && bin != -1 && *(is_flag+bin)) {
                if ((target_number = triplet_hash_find(th, a, machine_b->target, 1)) == -1) {
                    STACK_3_PUSH(1, machine_b->target,a);
                    target_number = triplet_hash_insert(th, a, machine_b->target, 1);
                }
                fsm_state_add_arc(current_state, bin, bout, target_number, current_final, current_start);
            }

            if (!g_compose_tristate) {
                /* Check 0:A arcs on lower side */
                if (bin == EPSILON) {
                    /* mode -> 1 */
                    if ((target_number = triplet_hash_find(th, a, machine_b->target, 1)) == -1) {
                        STACK_3_PUSH(1, machine_b->target,a);
                        target_number = triplet_hash_insert(th, a, machine_b->target, 1);
                    }
                    
                    fsm_state_add_arc(current_state, EPSILON, bout, target_number, current_final, current_start);
                }
            }

            else if (g_compose_tristate) {
                /* Check 0:A arcs on lower side */
                if (bin == EPSILON && mode != 1) {
                    /* mode -> 1 */
                    if ((target_number = triplet_hash_find(th, a, machine_b->target, 2)) == -1) {
                        STACK_3_PUSH(2, machine_b->target, a);
                        target_number = triplet_hash_insert(th, a, machine_b->target, 2);
                    }
                    
                    fsm_state_add_arc(current_state, EPSILON, bout, target_number, current_final, current_start);
                }
            }
        }
        fsm_state_end_state();
    }
    
    xxfree(net1->states);
    fsm_destroy(net2);
    fsm_state_close(net1);
    xxfree(point_a);
    xxfree(point_b);
    xxfree(index);
    xxfree(outarray);

    if (g_flag_is_epsilon)
        xxfree(is_flag);
    triplet_hash_free(th);
    net1 = fsm_topsort(fsm_coaccessible(net1));
    return(fsm_coaccessible(net1));
}

struct mergesigma *add_to_mergesigma(struct mergesigma *msigma, struct sigma *sigma, short presence) {
  int number = 0;

  if (msigma->number == -1) {
    number = 2;
  } else {
    msigma->next = xxmalloc(sizeof(struct mergesigma));
    number = msigma->number;
    msigma = msigma->next;
    msigma->next = NULL;
  }

  if (sigma->number < 3) {
    msigma->number = sigma->number;
  } else {
    if (number < 3)
      number = 2;
    msigma->number = number+1;
  }
  msigma->symbol = sigma->symbol;
  msigma->presence = presence;
  return(msigma);
}

struct sigma *copy_mergesigma(struct mergesigma *mergesigma) {
    struct sigma *sigma, *new_sigma;
    
    sigma = new_sigma = NULL;
    while(mergesigma != NULL) {
	if (sigma == NULL) {
	    sigma = xxmalloc(sizeof(struct sigma));
	    new_sigma = sigma;
	} else {
	    sigma->next = xxmalloc(sizeof(struct sigma));
	    sigma = sigma->next;
	}
	sigma->next = NULL;
	sigma->number = mergesigma->number;
	
	sigma->symbol = NULL;
	if (mergesigma->symbol != NULL)
	    sigma->symbol = xxstrdup(mergesigma->symbol);
	mergesigma = mergesigma->next;
    }
    return(new_sigma);
}

void fsm_merge_sigma(struct fsm *net1, struct fsm *net2) {

  struct sigma *sigma_1, *sigma_2, *new_sigma_1 = NULL, *new_sigma_2 = NULL;
  struct mergesigma *mergesigma, *mergesigma2, *start_mergesigma;
  struct fsm_state *fsm_state, *new_1_state, *new_2_state;
  int i, j, end_1 = 0, end_2 = 0, sigmasizes, *mapping_1, *mapping_2, equal = 1, unknown_1 = 0, unknown_2 = 0, net_unk = 0, net_adds = 0, net_lines;

  i = sigma_find(".#.", net1->sigma);
  j = sigma_find(".#.", net2->sigma);
  if (i != -1 && j == -1) {
      sigma_add(".#.", net2->sigma);
      sigma_sort(net2);
  }
  if (j != -1 && i == -1) {
      sigma_add(".#.", net1->sigma);
      sigma_sort(net1);
  }

  sigma_1 = net1->sigma;
  sigma_2 = net2->sigma;

  sigmasizes = sigma_size(sigma_1) + sigma_size(sigma_2);

  mapping_1 = xxmalloc(sizeof(int)*(sigmasizes+3));
  mapping_2 = xxmalloc(sizeof(int)*(sigmasizes+3));

  /* Fill mergesigma */

  mergesigma = xxmalloc(sizeof(struct mergesigma));
  mergesigma->number = -1;
  mergesigma->symbol = NULL;
  mergesigma->next = NULL;
  start_mergesigma = mergesigma;

  /* Loop over sigma 1, sigma 2 */
  for (;;) {
    if (sigma_1 == NULL)
      end_1 = 1;
    if (sigma_2 == NULL)
      end_2 = 1;
    if (end_1 && end_2)
      break;
    if (end_2) {
      /* Treating only 1 now */
      mergesigma = add_to_mergesigma(mergesigma, sigma_1, 1);
      *(mapping_1+(sigma_1->number)) = mergesigma->number;
      sigma_1 = sigma_1->next;
      equal = 0;
      continue;
    }
    else if (end_1) {
      /* Treating only 2 now */
      mergesigma = add_to_mergesigma(mergesigma, sigma_2, 2);
      *(mapping_2+(sigma_2->number)) = mergesigma->number;
      sigma_2 = sigma_2->next;
      equal = 0;
      continue;
    }

    else {

      /* Both alive */

      /* 1 or 2 contains special characters */
      if ((sigma_1->number <= IDENTITY) || (sigma_2->number <= IDENTITY)) {

	/* Treating zeros or unknowns */
	
	if ((sigma_1->number == UNKNOWN) || (sigma_1->number == IDENTITY))
	  unknown_1 = 1;
	if ((sigma_2->number == UNKNOWN) || (sigma_2->number == IDENTITY))
	  unknown_2 = 1;

	if (sigma_1->number == sigma_2->number) {
	  mergesigma = add_to_mergesigma(mergesigma, sigma_1, 3);
	  sigma_1 = sigma_1->next;
	  sigma_2 = sigma_2->next;
	}
	else if (sigma_1->number < sigma_2->number) {
	  mergesigma = add_to_mergesigma(mergesigma, sigma_1, 1);
	  sigma_1 = sigma_1->next;
	  equal = 0;
	}
	else {
	  mergesigma = add_to_mergesigma(mergesigma, sigma_2, 2);
	  sigma_2 = sigma_2->next;
	  equal = 0;
	}
	continue;
      }
      /* Both contain non-special chars */
      if (strcmp(sigma_1->symbol, sigma_2->symbol) == 0) {
        mergesigma = add_to_mergesigma(mergesigma, sigma_1, 3);
	/* Add symbol numbers to mapping */
	*(mapping_1+(sigma_1->number)) = mergesigma->number;
	*(mapping_2+(sigma_2->number)) = mergesigma->number;

	sigma_1 = sigma_1->next;
	sigma_2 = sigma_2->next;
      }
      else if (strcmp(sigma_1->symbol, sigma_2->symbol) < 0) {
	mergesigma = add_to_mergesigma(mergesigma, sigma_1, 1);
	*(mapping_1+(sigma_1->number)) = mergesigma->number;
	sigma_1 = sigma_1->next;
	equal = 0;
      }
      else {
	mergesigma = add_to_mergesigma(mergesigma, sigma_2, 2);
	*(mapping_2+(sigma_2->number)) = mergesigma->number;
	sigma_2 = sigma_2->next;
	equal = 0;
      }
      continue;
    }    
  }
  
  /* Go over both net1 and net2 and replace arc numbers with new mappings */
  
  fsm_state = net1->states;
  for (i=0; (fsm_state+i)->state_no != -1; i++) {
    if ((fsm_state+i)->in > 2)
      (fsm_state+i)->in = *(mapping_1+(fsm_state+i)->in);
    if ((fsm_state+i)->out > 2)
      (fsm_state+i)->out = *(mapping_1+(fsm_state+i)->out);
  }
  fsm_state = net2->states;
  for (i=0; (fsm_state+i)->state_no != -1; i++) {
    if ((fsm_state+i)->in > 2)
      (fsm_state+i)->in = *(mapping_2+(fsm_state+i)->in);
    if ((fsm_state+i)->out > 2)
      (fsm_state+i)->out = *(mapping_2+(fsm_state+i)->out);
  }

  /* Copy mergesigma to net1, net2 */
  
  new_sigma_1 = copy_mergesigma(start_mergesigma);
  new_sigma_2 = copy_mergesigma(start_mergesigma);
  
  fsm_sigma_destroy(net1->sigma);
  fsm_sigma_destroy(net2->sigma);

  net1->sigma = new_sigma_1;
  net2->sigma = new_sigma_2;

  /* Expand on ?, ?:x, y:? */

  if (unknown_1 && !equal) {
    /* Expand net 1 */
    fsm_state = net1->states;
    net_lines = find_arccount(net1->states);
    for(mergesigma = start_mergesigma; mergesigma != NULL; mergesigma=mergesigma->next) {
      if(mergesigma->presence == 2) {
	net_unk++;
      }
    }
    for(net_adds = 0, i=0; (fsm_state+i)->state_no != -1; i++) {
      if ((fsm_state+i)->in == IDENTITY)
	net_adds += net_unk;
      if (((fsm_state+i)->in == UNKNOWN) && ((fsm_state+i)->out != UNKNOWN))
	net_adds += net_unk;
      if (((fsm_state+i)->out == UNKNOWN) && ((fsm_state+i)->in != UNKNOWN))
	net_adds += net_unk;
      if (((fsm_state+i)->in == UNKNOWN) && ((fsm_state+i)->out == UNKNOWN))
	net_adds += net_unk*net_unk - net_unk + 2*net_unk;
    }

    new_1_state = xxmalloc(sizeof(struct fsm_state)*(net_adds+net_lines+1));
    for(i=0,j=0; (fsm_state+i)->state_no != -1; i++) {    
      
      if ((fsm_state+i)->in == IDENTITY) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma != NULL; mergesigma=mergesigma->next) {
	  if ((mergesigma->presence == 2) && (mergesigma->number > IDENTITY)) {
	    add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, mergesigma->number, mergesigma->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      if ((fsm_state+i)->in == UNKNOWN && (fsm_state+i)->out != UNKNOWN) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	  if ((mergesigma->presence == 2) && (mergesigma->number > IDENTITY)) {
	    add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, mergesigma->number, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      if (((fsm_state+i)->in != UNKNOWN) && ((fsm_state+i)->out == UNKNOWN)) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma != NULL; mergesigma = mergesigma->next) {
	  if ((mergesigma->presence == 2) && (mergesigma->number > IDENTITY)) {
	    add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, mergesigma->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      /* Replace ?:? with ?:[all unknowns] [all unknowns]:? and [all unknowns]:[all unknowns] where a != b */
      if ((fsm_state+i)->in == UNKNOWN && (fsm_state+i)->out == UNKNOWN) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma2=start_mergesigma; mergesigma2 != NULL ; mergesigma2 = mergesigma2->next) {
	  for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	    if (((mergesigma->presence == 2 && mergesigma2->presence == 2 && mergesigma->number > IDENTITY && mergesigma2->number > IDENTITY) || (mergesigma->number == UNKNOWN && mergesigma2->number > IDENTITY && mergesigma2->presence == 2) || (mergesigma2->number == UNKNOWN && mergesigma->number > IDENTITY && mergesigma->presence == 2)) && mergesigma->number != mergesigma2->number) {
	      add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, mergesigma->number, mergesigma2->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	      j++;
	    }
	  }
	}
      }

      /* Simply copy arcs that are not IDENTITY or UNKNOWN */
      if (((fsm_state+i)->in > IDENTITY || (fsm_state+i)->in == EPSILON) && ((fsm_state+i)->out > IDENTITY || (fsm_state+i)->out == EPSILON)) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
      }
      
      if ((fsm_state+i)->in == -1) {
	add_fsm_arc(new_1_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
      }
    }

    add_fsm_arc(new_1_state, j, -1, -1, -1, -1, -1, -1);
    xxfree(net1->states);
    net1->states = new_1_state;
  }

  if (unknown_2 && !equal) {
    /* Expand net 2 */
    fsm_state = net2->states;
    net_lines = find_arccount(net2->states);
    for(net_unk = 0, mergesigma = start_mergesigma; mergesigma != NULL; mergesigma=mergesigma->next) {
      if(mergesigma->presence == 1) {
	net_unk++;
      }
    }

    for(net_adds = 0, i=0; (fsm_state+i)->state_no != -1; i++) {
      if ((fsm_state+i)->in == IDENTITY)
	net_adds += net_unk;
      if (((fsm_state+i)->in == UNKNOWN) && ((fsm_state+i)->out != UNKNOWN))
	net_adds += net_unk;
      if (((fsm_state+i)->out == UNKNOWN) && ((fsm_state+i)->in != UNKNOWN))
	net_adds += net_unk;
      if (((fsm_state+i)->in == UNKNOWN) && ((fsm_state+i)->out == UNKNOWN))
	net_adds += net_unk*net_unk - net_unk + 2*net_unk;
    }

    /* We need net_add new lines in fsm_state */
    new_2_state = xxmalloc(sizeof(struct fsm_state)*(net_adds+net_lines+1));
    for(i=0,j=0; (fsm_state+i)->state_no != -1; i++) {    

      if ((fsm_state+i)->in == IDENTITY) {
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	  if ((mergesigma->presence == 1) && (mergesigma->number > IDENTITY)) {
	    add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, mergesigma->number, mergesigma->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      if ((fsm_state+i)->in == UNKNOWN && (fsm_state+i)->out != UNKNOWN) {
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	  if (mergesigma->presence == 1 && mergesigma->number > IDENTITY) {
	    add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, mergesigma->number, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      if ((fsm_state+i)->in != UNKNOWN && (fsm_state+i)->out == UNKNOWN) {
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	  if ((mergesigma->presence == 1) && (mergesigma->number > IDENTITY)) {
	    add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, mergesigma->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	    j++;
	  }
	}
      }

      if ((fsm_state+i)->in == UNKNOWN && (fsm_state+i)->out == UNKNOWN) {
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
	for (mergesigma2=start_mergesigma; mergesigma2 != NULL ; mergesigma2 = mergesigma2->next) {
	  for (mergesigma=start_mergesigma; mergesigma!=NULL; mergesigma=mergesigma->next) {
	    if (((mergesigma->presence == 1 && mergesigma2->presence == 1 && mergesigma->number > IDENTITY && mergesigma2->number > IDENTITY) || (mergesigma->number == UNKNOWN && mergesigma2->number > IDENTITY && mergesigma2->presence == 1) || (mergesigma2->number == UNKNOWN && mergesigma->number > IDENTITY && mergesigma->presence == 1)) && mergesigma->number != mergesigma2->number) {
	      add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, mergesigma->number, mergesigma2->number, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	      j++;
	    }
	  }
	}
      }

      /* Simply copy arcs that are not IDENTITY or UNKNOWN */
      if (((fsm_state+i)->in > IDENTITY || (fsm_state+i)->in == EPSILON) && ((fsm_state+i)->out > IDENTITY || (fsm_state+i)->out == EPSILON)) {
	
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
      }

      if ((fsm_state+i)->in == -1) {
	add_fsm_arc(new_2_state, j, (fsm_state+i)->state_no, (fsm_state+i)->in, (fsm_state+i)->out, (fsm_state+i)->target, (fsm_state+i)->final_state, (fsm_state+i)->start_state);
	j++;
      }
    }

    add_fsm_arc(new_2_state, j, -1, -1, -1, -1, -1, -1);
    xxfree(net2->states);
    net2->states = new_2_state;
  }
  xxfree(mapping_1);
  xxfree(mapping_2);

  /* Free structure */
  for (mergesigma2 = NULL; start_mergesigma != NULL; ) {
      mergesigma2 = start_mergesigma;
      start_mergesigma = start_mergesigma->next;
      xxfree(mergesigma2);
  }
}


int add_fsm_arc(struct fsm_state *fsm, int offset, int state_no, int in, int out, int target, int final_state, int start_state) {

  (fsm+offset)->state_no = state_no;
  (fsm+offset)->in = in;
  (fsm+offset)->out= out;
  (fsm+offset)->target = target;
  (fsm+offset)->final_state = final_state;
  (fsm+offset)->start_state = start_state;
  offset++;
  return(offset);
}


void fsm_count(struct fsm *net) {
  struct fsm_state *fsm;
  int i, linecount, arccount, oldstate, finalcount, maxstate;
  linecount = arccount = finalcount = maxstate = 0;

  oldstate = -1;

  fsm = net->states;
  for (i=0; (fsm+i)->state_no != -1; i++) {
    if ((fsm+i)->state_no > maxstate) 
      maxstate = (fsm+i)->state_no;

    linecount++;
    if ((fsm+i)->target != -1) {
        arccount++;
	//        if (((fsm+i)->in != (fsm+i)->out) || ((fsm+i)->in == UNKNOWN) || ((fsm+i)->out == UNKNOWN))
        //    arity = 2;
    }
    if ((fsm+i)->state_no != oldstate) {
        if ((fsm+i)->final_state) {
            finalcount++;
        }
        oldstate = (fsm+i)->state_no;
    }    
  }

  linecount++;
  net->statecount = maxstate+1;
  net->linecount = linecount;
  net->arccount = arccount;
  net->finalcount = finalcount;
}

static void fsm_add_to_states(struct fsm *net, int add) {
  struct fsm_state *fsm;
  int i;

  fsm=net->states;
  for(i=0; (fsm+i)->state_no != -1; i++) {
    (fsm+i)->state_no = (fsm+i)->state_no + add;
    if ((fsm+i)->target != -1)
      (fsm+i)->target = (fsm+i)->target + add;
  }
}

struct fsm *fsm_concat_m_n(struct fsm *net1, int m, int n) {
  struct fsm *acc;
    int i;
    acc = fsm_empty_string();
    for (i = 1; i <= n ;i++) {
        if (i > m)
            acc = fsm_concat(acc, fsm_optionality(fsm_copy(net1)));
        else
            acc = fsm_concat(acc, fsm_copy(net1));
    }
    fsm_destroy(net1);
    return(acc);
}

struct fsm *fsm_concat_n(struct fsm *net1, int n) {
    return(fsm_concat_m_n(net1,n,n));
}

struct fsm *fsm_concat(struct fsm *net1, struct fsm *net2) {
  struct fsm_state *fsm1, *fsm2, *new_fsm;
  int i,j,current_final;

  fsm_merge_sigma(net1, net2);

  fsm1 = net1->states;
  fsm2 = net2->states;
  fsm_count(net1);
  fsm_count(net2);
  /* The concatenation of a language with no final state should yield the empty language */
  if ((net1->finalcount == 0) || (net2->finalcount == 0)) {
      fsm_destroy(net1);
      fsm_destroy(net2);
      net1 = fsm_empty_set();
      return(net1);
  }

  /* Add |fsm1| states to the state numbers of fsm2 */
  fsm_add_to_states(net2, net1->statecount);

  new_fsm = xxmalloc(((sizeof(struct fsm_state))*(net1->linecount + net2->linecount + net1->finalcount + 2 )));
  current_final = -1;
  /* Copy fsm1, fsm2 after each other, adding appropriate epsilon arcs */
  for(i=0,j=0; (fsm1+i)->state_no != -1; i++) {
    if (((fsm1+i)->final_state == 1) && ((fsm1+i)->state_no != current_final)) {
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, EPSILON, EPSILON, net1->statecount, 0, (fsm1+i)->start_state);
      current_final = (fsm1+i)->state_no;
      j++;
    }
    if (!(((fsm1+i)->target == -1) && ((fsm1+i)->final_state == 1))) {
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, (fsm1+i)->in, (fsm1+i)->out, (fsm1+i)->target, 0, (fsm1+i)->start_state);
      j++;
    }
  }

  for(i=0; (fsm2+i)->state_no != -1; i++, j++) {
    add_fsm_arc(new_fsm, j, (fsm2+i)->state_no, (fsm2+i)->in, (fsm2+i)->out, (fsm2+i)->target, (fsm2+i)->final_state, 0);
  }
  add_fsm_arc(new_fsm, j, -1, -1, -1, -1, -1, -1);
  xxfree(net1->states);
  fsm_destroy(net2);
  net1->states = new_fsm;
  if (sigma_find_number(EPSILON, net1->sigma) == -1) {
    sigma_add_special(EPSILON, net1->sigma);
  }
  fsm_count(net1);
  net1->is_epsilon_free = NO;
  net1->is_deterministic = NO;
  net1->is_minimized = NO;
  net1->is_pruned = NO;
  return(fsm_minimize(net1));
}

struct fsm *fsm_union(struct fsm *net1, struct fsm *net2) {
    struct fsm_state *new_fsm, *fsm1, *fsm2;
    int i, j, net1_offset, net2_offset, new_target, arccount;
    
    fsm_merge_sigma(net1, net2);

    fsm_count(net1);
    fsm_count(net2);

    fsm1 = net1->states;
    fsm2 = net2->states;

    net1_offset = 1;
    net2_offset = net1->statecount + 1;
    new_fsm = xxmalloc((net1->linecount + net2->linecount + 2) * sizeof(struct fsm_state));

    j = 0;

    add_fsm_arc(new_fsm, j++, 0, EPSILON, EPSILON, net1_offset, 0 , 1);
    add_fsm_arc(new_fsm, j++, 0, EPSILON, EPSILON, net2_offset, 0 , 1);
    arccount = 2;
    for (i=0 ; (fsm1+i)->state_no != -1; i++) {
        new_target = (fsm1+i)->target == -1 ? -1 : (fsm1+i)->target + net1_offset;
        add_fsm_arc(new_fsm, j++, (fsm1+i)->state_no + net1_offset, (fsm1+i)->in, (fsm1+i)->out, new_target, (fsm1+i)->final_state, 0);
        if (new_target != -1) arccount++;
    }
    for (i=0 ; (fsm2+i)->state_no != -1; i++) {
        new_target = (fsm2+i)->target == -1 ? -1 : (fsm2+i)->target + net2_offset;
        add_fsm_arc(new_fsm, j++, (fsm2+i)->state_no + net2_offset, (fsm2+i)->in, (fsm2+i)->out, new_target, (fsm2+i)->final_state, 0);
        if (new_target != -1) arccount++;
    }
    add_fsm_arc(new_fsm, j++, -1, -1, -1, -1, -1, -1);
    xxfree(net1->states);
    net1->states = new_fsm;
    net1->statecount = net1->statecount + net2->statecount + 1;
    net1->linecount = j;
    net1->arccount = arccount;
    net1->finalcount = net1->finalcount + net2->finalcount;
    fsm_destroy(net2);
    fsm_update_flags(net1,NO,NO,NO,NO,UNK,NO);
    if (sigma_find_number(EPSILON, net1->sigma) == -1) {
        sigma_add_special(EPSILON, net1->sigma);
    }
    return(net1);
}

struct fsm *fsm_completes(struct fsm *net, int operation) {
  struct fsm_state *fsm, *new_fsm;
  int i, j, offset, statecount, sigsize, *state_table, sink_state, target, last_sigma = 0, arccount = 0, incomplete;
  short *starts, *finals, *sinks;
  
  /* TODO: this currently relies on that the sigma is gap-free in its numbering  */
  /* which can't always be counted on, especially when reading external machines */

  /* TODO: check arity */

  if (net->is_minimized != YES)
      net = fsm_minimize(net);

  incomplete = 0;
  fsm = net->states;
  if (sigma_find_number(UNKNOWN, net->sigma) != -1) {
      sigma_remove("@_UNKNOWN_SYMBOL_@",net->sigma);
  }
  if (sigma_find_number(IDENTITY, net->sigma) == -1) {
    sigma_add_special(IDENTITY, net->sigma);
    incomplete = 1;
  }

  sigsize = sigma_size(net->sigma);
  last_sigma = sigma_max(net->sigma);

  if (sigma_find_number(EPSILON, net->sigma) != -1)
      sigsize--;

  fsm_count(net);
  statecount = net->statecount;
  starts = xxmalloc(sizeof(short)*(statecount+1)); /* +1 for sink state */
  finals = xxmalloc(sizeof(short)*(statecount+1));
  sinks = xxmalloc(sizeof(short)*(statecount+1));

  /* Init starts, finals, sinks arrays */

  for (i=0; i < statecount; i++) {
    *(sinks+i) = 1;
    *(finals+i) = 0;
    *(starts+i) = 0;
  }
  for (i=0; (fsm+i)->state_no != -1; i++) {
    if (operation == COMPLEMENT) {
      if ((fsm+i)->final_state == 1) {
	(fsm+i)->final_state = 0;
      } else if ((fsm+i)->final_state == 0) {
	(fsm+i)->final_state = 1;
      }		
    }
    if ((fsm+i)->target != -1)
      arccount++;
    starts[(fsm+i)->state_no] = (fsm+i)->start_state;
    finals[(fsm+i)->state_no] = (fsm+i)->final_state;
    if ((fsm+i)->final_state && operation != COMPLEMENT)
      *(sinks+((fsm+i)->state_no)) = 0;
    if ((fsm+i)->final_state == 0 && (operation == COMPLEMENT))
      *(sinks+((fsm+i)->state_no)) = 0;
    if (((fsm+i)->target != -1) && ((fsm+i)->state_no != (fsm+i)->target))
      *(sinks+((fsm+i)->state_no)) = 0;
  }

  net->is_loop_free = NO;
  net->pathcount = PATHCOUNT_CYCLIC;

  if (incomplete == 0 && (arccount == (sigsize)*statecount)) {
    /*    printf("Already complete!\n"); */

/*     if (operation == COMPLEMENT) { */
/*       for (i=0; (fsm+i)->state_no != -1; i++) { */
/* 	if ((fsm+i)->final_state) { */
/* 	  (fsm+i)->final_state = 0; */
/* 	} else { */
/* 	  (fsm+i)->final_state = 1; */
/* 	} */
/*       } */
/*     } */
    xxfree(starts);
    xxfree(finals);
    xxfree(sinks);
    net->is_completed = YES;
    net->is_minimized = YES;
    net->is_pruned = NO;
    net->is_deterministic = YES;
    return(net);
  }

  /* Find an existing sink state, or invent a new one */

  for (i=0, sink_state = -1; i<statecount; i++) {
    if (sinks[i] == 1) {
      sink_state = i;
      break;
    }
  }

  if (sink_state == -1) {
    sink_state = statecount;
    *(starts+sink_state) = 0;
    if (operation == COMPLEMENT) {
      *(finals+sink_state) = 1;
    } else {
      *(finals+sink_state) = 0;
    }
    statecount++;
  }


  /* We can build a state table without memory problems since the size */
  /* of the completed machine will be |Sigma| * |States| in all cases */

  sigsize += 2;

  state_table = xxmalloc(sizeof(int)*sigsize*statecount);

  /* Init state table */
  /* i = state #, j = sigma # */
  for (i=0; i<statecount; i++) {
    for (j=0; j<sigsize; j++) {
      *(state_table+(i*sigsize+j)) = -1;
    }
  }
  
  for (i=0; (fsm+i)->state_no != -1; i++) {
    if ((fsm+i)->target != -1) {
      *(state_table+(((fsm+i)->state_no)*sigsize+((fsm+i)->in))) = (fsm+i)->target;
    }
  }
  /* Add looping arcs from and to sink state */
  for (j=2; j<=last_sigma; j++) 
      *(state_table+(sink_state*sigsize+j)) = sink_state;
  /* Add missing arcs to sink state from all states */
  for (i=0; i<statecount; i++) {
    for (j=2; j<=last_sigma; j++) {
      if (*(state_table+(i*sigsize+j)) == -1)
	*(state_table+(i*sigsize+j)) = sink_state;
    }
  }
  
  new_fsm = xxmalloc(sizeof(struct fsm_state)*(sigsize*statecount+1));
  
/* Complement requires toggling final, nonfinal states */
/*   if (operation == COMPLEMENT) */
/*     for (i=0; i < statecount; i++) */
/*       *(finals+i) = *(finals+i) == 0 ? 1 : 0; */
  
  for (i=0, offset = 0; i<statecount; i++) {
    for (j=2; j<=last_sigma; j++) {
      target = *(state_table+(i*sigsize+j)) == -1 ? sink_state : *(state_table+(i*sigsize+j));
      add_fsm_arc(new_fsm, offset, i, j, j, target, finals[i], starts[i]);
      offset++;
    }
  }
  add_fsm_arc(new_fsm, offset, -1, -1, -1, -1, -1, -1);
  offset++;
  xxfree(net->states);
  net->states = new_fsm;
  xxfree(starts);
  xxfree(finals);
  xxfree(sinks);
  xxfree(state_table);
  net->is_minimized = NO;
  net->is_pruned = NO;
  net->is_completed = YES;
  net->statecount = statecount;
  return(net);
}

struct fsm *fsm_complete(struct fsm *net) {
  return(fsm_completes(net, COMPLETE));
}

struct fsm *fsm_complement(struct fsm *net) {
  return(fsm_completes(net, COMPLEMENT));
}

struct fsm *fsm_kleene_closure(struct fsm *net, int operation) {
    struct fsm_state *fsm, *new_fsm;
    int i, j, laststate, curr_state, curr_target, arccount;

    if (operation == OPTIONALITY) {
        return(fsm_union(net,fsm_empty_string()));
    }

    net = fsm_minimize(net);
    fsm_count(net);    

    fsm = net->states;
    
    new_fsm = xxmalloc( (net->linecount + net->finalcount + 1) * sizeof(struct fsm_state));

    j = 0;
    if (operation == KLEENE_STAR)
        add_fsm_arc(new_fsm, j++, 0, EPSILON, EPSILON, 1, 1, 1);
    if (operation == KLEENE_PLUS)            
        add_fsm_arc(new_fsm, j++, 0, EPSILON, EPSILON, 1, 0, 1);
    laststate = 0;
    arccount = 1;
    for (i = 0 ; (fsm+i)->state_no != -1; i++, laststate = curr_state) {
        curr_state = (fsm+i)->state_no + 1;
        curr_target = (fsm+i)->target == -1 ? -1 : (fsm+i)->target + 1;
        if (curr_target == -1 && (fsm+i)->final_state == 1) {
            add_fsm_arc(new_fsm, j++, curr_state, EPSILON, EPSILON, 0, 1, 0);
            arccount++;
            continue;
        }
        if (curr_state != laststate && (fsm+i)->final_state == 1) {
            arccount++;
            add_fsm_arc(new_fsm, j++, curr_state, EPSILON, EPSILON, 0, 1, 0);
        }
        add_fsm_arc(new_fsm, j++, curr_state, (fsm+i)->in, (fsm+i)->out, curr_target, (fsm+i)->final_state, 0);
        if (curr_target != -1) arccount++;
    }
    add_fsm_arc(new_fsm, j++, -1,-1,-1,-1,-1,-1);
    net->statecount = net->statecount+1;
    net->linecount = j;
    net->finalcount = operation == KLEENE_STAR ? net->finalcount+1 : net->finalcount;
    net->arccount = arccount;
    net->pathcount = PATHCOUNT_UNKNOWN;
    xxfree(net->states);
    net->states = new_fsm;
    if (sigma_find_number(EPSILON, net->sigma) == -1)
        sigma_add_special(EPSILON, net->sigma);
    fsm_update_flags(net,NO,NO,NO,NO,UNK,NO);
    return(net);
}

char *fsm_network_to_char(struct fsm *net) {
    struct sigma *sigma, *sigprev;
    sigma = net->sigma;
    if (sigma->number == -1) {
        return NULL;
    }
    for (; sigma != NULL && sigma->number != -1 ; sigma = sigma->next) {
	sigprev = sigma;
    }
    return(strdup(sigprev->symbol));
}

struct fsm *fsm_substitute_label(struct fsm *net, char *original, struct fsm *substitute) {
   
    struct fsm *outnet, *subnet2;
    struct fsm_read_handle *inh, *subh, *subh2;
    struct fsm_construct_handle *outh;
    char *subin, *subout;
    int i, repsym, source, target, in, out, addstate1, addstate2;
        
    fsm_merge_sigma(net, substitute);
    addstate1 = net->statecount;
    addstate2 = substitute->statecount;

    inh = fsm_read_init(net);
    subh = fsm_read_init(substitute);
    repsym = fsm_get_symbol_number(inh, original);
    if (repsym == -1) {
	fsm_read_done(inh);
	return(net);
    }
    outh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(outh, net->sigma);
    while (fsm_get_next_arc(inh)) {
	source = fsm_get_arc_source(inh);
	target = fsm_get_arc_target(inh);
	in = fsm_get_arc_num_in(inh);
	out = fsm_get_arc_num_out(inh);

	/* Double-sided arc, splice in substitute network */
	if (in == repsym && out == repsym) {
	    fsm_read_reset(subh);
	    fsm_construct_add_arc_nums(outh, source, addstate1, EPSILON, EPSILON);
	    while (fsm_get_next_arc(subh)) {
		source = fsm_get_arc_source(subh);
		target = fsm_get_arc_target(subh);
		subin = fsm_get_arc_in(subh);
		subout = fsm_get_arc_out(subh);
		fsm_construct_add_arc(outh, source+addstate1, target+addstate1, subin, subout);
	    }
	    while ((i = fsm_get_next_final(subh)) != -1) {
		target = fsm_get_arc_target(inh);
		fsm_construct_add_arc_nums(outh, addstate1+i, target, EPSILON, EPSILON);
	    }
	    addstate1 = addstate1 + addstate2;
	    /* One-sided replace, splice in repsym .x. sub or sub .x. repsym */
	} else if (in == repsym || out == repsym) {
	    if (in == repsym) {
		subnet2 = fsm_minimize(fsm_cross_product(fsm_copy(substitute), fsm_symbol(fsm_get_arc_out(inh))));
	    } else {
		subnet2 = fsm_minimize(fsm_cross_product(fsm_symbol(fsm_get_arc_in(inh)),fsm_copy(substitute)));
	    }
	    fsm_construct_add_arc_nums(outh, source, addstate1, EPSILON, EPSILON);		
	    subh2 = fsm_read_init(subnet2);
	    while (fsm_get_next_arc(subh2)) {
		source = fsm_get_arc_source(subh2);
		target = fsm_get_arc_target(subh2);
		subin = fsm_get_arc_in(subh2);
		subout = fsm_get_arc_out(subh2);
		fsm_construct_add_arc(outh, source+addstate1, target+addstate1, subin, subout);
	    }
	    while ((i = fsm_get_next_final(subh2)) != -1) {
		target = fsm_get_arc_target(inh);
		fsm_construct_add_arc_nums(outh, addstate1+i, target, EPSILON, EPSILON);
	    }
	    fsm_read_done(subh2);
	    addstate1 = addstate1 + subnet2->statecount;
	    fsm_destroy(subnet2);
	} else {
	    /* Default, just copy arc */
	    fsm_construct_add_arc_nums(outh, source, target, in, out);
	}
    }

    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(outh, i);
    }
    while ((i = fsm_get_next_initial(inh)) != -1) {
	fsm_construct_set_initial(outh, i);
    }
    fsm_read_done(inh);
    fsm_read_done(subh);
    outnet = fsm_construct_done(outh);
    return(outnet);
}

struct fsm *fsm_substitute_symbol(struct fsm *net, char *original, char *substitute) {
    struct fsm_state *fsm;
    int i,o,s = EPSILON;
    if (strcmp(original,substitute) == 0)
        return(net);
    if ((o = sigma_find(original, net->sigma)) == -1) {
	printf("\nSymbol '%s' not found in network!\n",original);
	return(net);
    }
    if (strcmp(substitute,"0") == 0)
        s = EPSILON;
    else if (substitute != NULL && (s = sigma_find(substitute, net->sigma)) == -1) {
        s = sigma_add(substitute, net->sigma);
    }
    for (i=0, fsm = net->states; (fsm+i)->state_no != -1; i++) {
	if ((fsm+i)->in == o) {
	    (fsm+i)->in = s;
        }
	if ((fsm+i)->out == o) {
	    (fsm+i)->out = s;
        }
    }
    net->sigma = sigma_remove(original, net->sigma);
    sigma_sort(net);
    fsm_update_flags(net, NO, NO, NO, NO, NO, NO);
    sigma_cleanup(net,0);
    /* if s = epsilon */
    net->is_minimized = NO;
    return(fsm_determinize(net));
}

struct fsm *fsm_cross_product(struct fsm *net1, struct fsm *net2) {
  int i, a, b, current_state, current_start, current_final, target_number, symbol1, symbol2, epsilon = 0, unknown = 0;
  struct fsm_state *machine_a, *machine_b, *fsm;
  struct state_arr *point_a, *point_b;
  struct triplethash *th;

  /* Perform a cross product by running two machines in parallel */
  /* The approach here allows a state to stay, creating a a:0 or 0:b transition */
  /* with the a/b-state waiting, and the arc going to {a,stay} or {stay,b} */
  /* the wait maneuver is only possible if the waiting state is final */
  
  /* For the rewrite rules compilation, a different cross-product is used:  */
  /* rewrite_cp() synchronizes A and B as long as possible to get a unique  */
  /* output match for each cross product.                                   */

  /* This behavior where we postpone zeroes on either side and perform */
  /* and equal length cross-product as long as possible and never intermix */
  /* ?:0 and 0:? arcs (i.e. we keep both machines synchronized as long as possible */
  /* can be done by [A .x. B] & ?:?* [?:0*|0:?*] at the cost of possibly */
  /* up to three times larger transducers. */
  /* This is very similar to the idea in "tristate composition" in fsm_compose() */

  /* This function is only used for explicit cross products */
  /* such as a:b or A.x.B, etc.  In rewrite rules, we use rewrite_cp() */

  net1 = fsm_minimize(net1);
  net2 = fsm_minimize(net2);

  fsm_merge_sigma(net1, net2);
  
  fsm_count(net1);
  fsm_count(net2);

  machine_a = net1->states;
  machine_b = net2->states;
  
  /* new state 0 = {0,0} */

  STACK_2_PUSH(0,0);

  th = triplet_hash_init();
  triplet_hash_insert(th, 0, 0, 0);

  fsm_state_init(sigma_max(net1->sigma));

  point_a = init_state_pointers(machine_a);
  point_b = init_state_pointers(machine_b);

  while (!int_stack_isempty()) {
 
   /* Get a pair of states to examine */
 
    a = int_stack_pop();
    b = int_stack_pop();
    
   /* printf("Treating pair: {%i,%i}\n",a,b); */

    current_state = triplet_hash_find(th, a, b, 0);
    current_start = (((point_a+a)->start == 1) && ((point_b+b)->start == 1)) ? 1 : 0;
    current_final = (((point_a+a)->final == 1) && ((point_b+b)->final == 1)) ? 1 : 0;

    fsm_state_set_current_state(current_state, current_final, current_start);

    for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a  ; machine_a++) {
      for (machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
	
	if ((machine_a->target == -1) && (machine_b->target == -1)) {
	  continue;
	}
	if ((machine_a->target == -1) && (machine_a->final_state == 0)) {
	  continue;
	}
	if ((machine_b->target == -1) && (machine_b->final_state == 0)) {
	  continue;
	}
	/* Main check */
	if (!((machine_a->target == -1) || (machine_b->target == -1))) {
	    if ((target_number = triplet_hash_find(th, machine_a->target, machine_b->target, 0)) == -1) {
              STACK_2_PUSH(machine_b->target, machine_a->target);
              target_number = triplet_hash_insert(th, machine_a->target, machine_b->target, 0);
	  }
	  symbol1 = machine_a->in;
	  symbol2 = machine_b->in;
	  if (symbol1 == IDENTITY && symbol2 != IDENTITY)
	    symbol1 = UNKNOWN;
	  if (symbol2 == IDENTITY && symbol1 != IDENTITY)
	    symbol2 = UNKNOWN;
	  
          fsm_state_add_arc(current_state, symbol1, symbol2, target_number, current_final, current_start);
	  /* @:@ -> @:@ and also ?:? */
	  if ((machine_a->in == IDENTITY) && (machine_b->in == IDENTITY)) {
              fsm_state_add_arc(current_state, UNKNOWN, UNKNOWN, target_number, current_final, current_start);
	  }
	}
	if (machine_a->final_state == 1 && machine_b->target != -1) {
            
	  /* Add 0:b i.e. stay in state A */
	    if ((target_number = triplet_hash_find(th, machine_a->state_no, machine_b->target, 0)) == -1) {
		STACK_2_PUSH(machine_b->target, machine_a->state_no);
		target_number = triplet_hash_insert(th, machine_a->state_no, machine_b->target, 0);
	    }
	  /* @:0 becomes ?:0 */
	  symbol2 = machine_b->in == IDENTITY ? UNKNOWN : machine_b->in;
          fsm_state_add_arc(current_state, EPSILON, symbol2, target_number, current_final, current_start);
	}

	if (machine_b->final_state == 1 && machine_a->target != -1) {
	  
	  /* Add a:0 i.e. stay in state B */
	    if ((target_number = triplet_hash_find(th, machine_a->target, machine_b->state_no, 0)) == -1) {
              STACK_2_PUSH(machine_b->state_no, machine_a->target);
              target_number = triplet_hash_insert(th, machine_a->target, machine_b->state_no, 0);
	  }
	  /* @:0 becomes ?:0 */
	  symbol1 = machine_a->in == IDENTITY ? UNKNOWN : machine_a->in;
          fsm_state_add_arc(current_state, symbol1, EPSILON, target_number, current_final, current_start);
	}
      }
    }
    /* Check arctrack */
    fsm_state_end_state();
  }
  
  xxfree(net1->states);
  fsm_state_close(net1);

  for (i=0, fsm = net1->states; (fsm+i)->state_no != -1; i++) {
      if (((fsm+i)->in == EPSILON) || ((fsm+i)->out == EPSILON))
          epsilon = 1;
      if (((fsm+i)->in == UNKNOWN) || ((fsm+i)->out == UNKNOWN))
          unknown = 1;    
  }
  if (epsilon == 1) {
      if (sigma_find_number(EPSILON, net1->sigma) == -1) {
          sigma_add_special(EPSILON, net1->sigma);
      }
  }
  if (unknown == 1) {
      if (sigma_find_number(UNKNOWN, net1->sigma) == -1) {
          sigma_add_special(UNKNOWN, net1->sigma);
      }
  }
  xxfree(point_a);
  xxfree(point_b);
  fsm_destroy(net2);
  triplet_hash_free(th);
  return(fsm_coaccessible(net1));
}

struct fsm *fsm_precedes(struct fsm *net1, struct fsm *net2) {
    return(fsm_complement(fsm_minimize(fsm_contains(fsm_minimize(fsm_concat(fsm_minimize(fsm_copy(net2)),fsm_concat(fsm_universal(),fsm_minimize(fsm_copy(net1)))))))));
}

struct fsm *fsm_follows(struct fsm *net1, struct fsm *net2) {
    return(fsm_complement(fsm_minimize(fsm_contains(fsm_minimize(fsm_concat(fsm_minimize(fsm_copy(net1)),fsm_concat(fsm_universal(),fsm_minimize(fsm_copy(net2)))))))));
}

struct fsm *fsm_shuffle(struct fsm *net1, struct fsm *net2) {
  int a, b, current_state, current_start, current_final, target_number;
  struct fsm_state *machine_a, *machine_b;
  struct state_arr *point_a, *point_b;
  struct triplethash *th;

  /* Shuffle A and B by making alternatively A move and B stay at each or */
  /* vice versa at each step */

  fsm_minimize(net1);
  fsm_minimize(net2);

  fsm_merge_sigma(net1, net2);

  fsm_count(net1);
  fsm_count(net2);

  machine_a = net1->states;
  machine_b = net2->states;
  
  /* new state 0 = {0,0} */

  STACK_2_PUSH(0,0);

  th = triplet_hash_init();
  triplet_hash_insert(th, 0, 0, 0);

  fsm_state_init(sigma_max(net1->sigma));

  point_a = init_state_pointers(machine_a);
  point_b = init_state_pointers(machine_b);

  while (!int_stack_isempty()) {
 
   /* Get a pair of states to examine */
 
    a = int_stack_pop();
    b = int_stack_pop();
    
   /* printf("Treating pair: {%i,%i}\n",a,b); */

    current_state = triplet_hash_find(th, a, b, 0);
    current_start = (((point_a+a)->start == 1) && ((point_b+b)->start == 1)) ? 1 : 0;
    current_final = (((point_a+a)->final == 1) && ((point_b+b)->final == 1)) ? 1 : 0;

    fsm_state_set_current_state(current_state, current_final, current_start);

    /* Follow A, B stays */
    for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a  ; machine_a++) {
	if (machine_a->target == -1) {
	  continue;
	}
	if ((target_number = triplet_hash_find(th, machine_a->target, b, 0)) == -1) {
          STACK_2_PUSH(b, machine_a->target);
	  target_number = triplet_hash_insert(th, machine_a->target, b, 0);
	}

        fsm_state_add_arc(current_state, machine_a->in, machine_a->out, target_number, current_final, current_start);
    }

    /* Follow B, A stays */
      for (machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
	
	if (machine_b->target == -1) {
	  continue;
	}

	if ((target_number = triplet_hash_find(th, a, machine_b->target, 0)) == -1) {
              STACK_2_PUSH(machine_b->target, a);
              target_number = triplet_hash_insert(th, a, machine_b->target, 0);
	  }
          fsm_state_add_arc(current_state, machine_b->in, machine_b->out, target_number, current_final, current_start);
      }

      /* Check arctrack */
      fsm_state_end_state();
  }

  xxfree(net1->states);
  fsm_state_close(net1);
  xxfree(point_a);
  xxfree(point_b);
  fsm_destroy(net2);
  triplet_hash_free(th);
  return(net1);
}

int fsm_equivalent(struct fsm *net1, struct fsm *net2) {
    /* Test path equivalence of two FSMs by traversing both in parallel */
    int a, b, matching_arc, equivalent;
    struct fsm_state *machine_a, *machine_b;
    struct state_arr *point_a, *point_b;
    struct triplethash *th;
    
    fsm_merge_sigma(net1, net2);
    
    fsm_count(net1);
    fsm_count(net2);
    
    machine_a = net1->states;
    machine_b = net2->states;
    
    equivalent = 0;
    /* new state 0 = {0,0} */
    STACK_2_PUSH(0,0);
    
    th = triplet_hash_init();
    triplet_hash_insert(th, 0, 0, 0);
    
    point_a = init_state_pointers(machine_a);
    point_b = init_state_pointers(machine_b);
    
    while (!int_stack_isempty()) {
	
	/* Get a pair of states to examine */
	
	a = int_stack_pop();
	b = int_stack_pop();
   	
	if ((point_a+a)->final != (point_b+b)->final) {	  
	    goto not_equivalent;
	}
	/* Check that all arcs in A have matching arc in B, push new state pair on stack */
	for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a  ; machine_a++) {
	    if (machine_a->target == -1) {
		break;
	    }
	    matching_arc = 0;
	    for (machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
		if (machine_b->target == -1) {
		    break;
		}
		if (machine_a->in == machine_b->in && machine_a->out == machine_b->out) {
		    matching_arc = 1;
		    if ((triplet_hash_find(th, machine_a->target, machine_b->target, 0)) == -1) {
			STACK_2_PUSH(machine_b->target, machine_a->target);
			triplet_hash_insert(th, machine_a->target, machine_b->target, 0);
		    }
		    break;
		}
	    }
	    if (matching_arc == 0) {
		goto not_equivalent;
	    }
	}
	for (machine_b = (point_b+b)->transitions; machine_b->state_no == b ; machine_b++) {
	    if (machine_b->target == -1) {
		break;
	    }
	    matching_arc = 0;
	    for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a  ; machine_a++) {
		if (machine_a->in == machine_b->in && machine_a->out == machine_b->out) {
		    matching_arc = 1;
		    break;
		}
	    }
	    if (matching_arc == 0) {
		goto not_equivalent;
	    }
	}
    }
    equivalent = 1;
 not_equivalent:
    fsm_destroy(net1);
    fsm_destroy(net2);
    xxfree(point_a);
    xxfree(point_b);
    triplet_hash_free(th);
    return(equivalent);
}


struct fsm *fsm_minus(struct fsm *net1, struct fsm *net2) {
    int a, b, current_state, current_start, current_final, target_number, b_has_trans, btarget, statecount;
    struct fsm_state *machine_a, *machine_b;
    struct state_arr *point_a, *point_b;
    struct triplethash *th;
    statecount = 0;

    net1 = fsm_minimize(net1);
    net2 = fsm_minimize(net2);
    
    fsm_merge_sigma(net1, net2);
    
    fsm_count(net1);
    fsm_count(net2);
    
    machine_a = net1->states;
    machine_b = net2->states;
    
    /* new state 0 = {1,1} */

    int_stack_clear();
    STACK_2_PUSH(1,1);

    th = triplet_hash_init();
    triplet_hash_insert(th, 1, 1, 0);

    point_a = init_state_pointers(machine_a);
    point_b = init_state_pointers(machine_b);

    fsm_state_init(sigma_max(net1->sigma));

  while (!int_stack_isempty()) {
      statecount++;
      /* Get a pair of states to examine */
 
      a = int_stack_pop();
      b = int_stack_pop();

      current_state = triplet_hash_find(th, a, b, 0);
      a--;
      b--;
    
      if (b == -1) {
          current_start = 0; 
          current_final = (point_a+a)->final; 
      } else { 
          current_start = (a == 0 && b == 0) ? 1 : 0;
          current_final = (((point_a+a)->final == 1) && ((point_b+b)->final == 0)) ? 1 : 0; 
      } 
      
      fsm_state_set_current_state(current_state, current_final, current_start);
      
      for (machine_a = (point_a+a)->transitions ; machine_a->state_no == a  ; machine_a++) {
          if (machine_a->target == -1) {
              break;
              continue;
          }
          if (b == -1) {
              /* b is dead */
              if ((target_number = triplet_hash_find(th, (machine_a->target)+1, 0, 0)) == -1) {
                  STACK_2_PUSH(0, (machine_a->target)+1);
                  target_number = triplet_hash_insert(th, (machine_a->target)+1, 0, 0);
              }
          } else {
              /* b is alive */
              b_has_trans = 0;
              for (machine_b = (point_b+b)->transitions ; machine_b->state_no == b ; machine_b++) {
                  if (machine_a->in == machine_b->in && machine_a->out == machine_b->out) {
                      b_has_trans = 1;
                      btarget = machine_b->target;
                      break;
                  }
              }
              if (b_has_trans) {
                  if ((target_number = triplet_hash_find(th, (machine_a->target)+1, btarget+1, 0)) == -1) {
                      STACK_2_PUSH(btarget+1, (machine_a->target)+1);
		      target_number = triplet_hash_insert(th, (machine_a->target)+1, (machine_b->target)+1, 0);
                  }
              } else {
                  /* b is dead */
                  if ((target_number = triplet_hash_find(th, (machine_a->target)+1, 0, 0)) == -1) {
                      STACK_2_PUSH(0, (machine_a->target)+1);
		      target_number = triplet_hash_insert(th, (machine_a->target)+1, 0, 0);
                  }
              }
          }
          fsm_state_add_arc(current_state, machine_a->in, machine_a->out, target_number, current_final, current_start);
      }
      fsm_state_end_state();
  }
  
  xxfree(net1->states);
  fsm_state_close(net1);
  xxfree(point_a);
  xxfree(point_b);
  fsm_destroy(net2);
  triplet_hash_free(th);
  return(fsm_minimize(net1));
}

struct fsm *fsm_contains(struct fsm *net) {
  /* [?* A ?*] */
  struct fsm *net2;
  
  net2 = fsm_concat(fsm_concat(fsm_universal(),net),fsm_universal());
  return(net2);
}

struct fsm *fsm_universal() {
    struct fsm *net;
    int s;
    net = fsm_create("");
    fsm_update_flags(net, YES, YES, YES, YES, NO, NO);
    net->states = xxmalloc(sizeof(struct fsm_state)*2);
    s = sigma_add_special(IDENTITY,net->sigma);
    add_fsm_arc(net->states, 0, 0, s, s, 0, 1, 1);
    add_fsm_arc(net->states, 1, -1, -1, -1, -1, -1, -1);
    net->arccount = 1;
    net->statecount = 1;
    net->linecount = 2;
    net->finalcount = 1;
    net->pathcount = PATHCOUNT_CYCLIC;
    return(net);
}

struct fsm *fsm_contains_one(struct fsm *net) {
  /* $A - $[[?+ A ?* & A ?*] | [A ?+ & A]] */
    struct fsm *ret;
    ret = fsm_minus(fsm_contains(fsm_copy(net)),fsm_contains(fsm_union(fsm_intersect(fsm_concat(fsm_kleene_plus(fsm_identity()),fsm_concat(fsm_copy(net),fsm_universal())) , fsm_concat(fsm_copy(net),fsm_universal())),fsm_intersect(fsm_concat(fsm_copy(net),fsm_kleene_plus(fsm_identity())), fsm_copy(net)))));
    fsm_destroy(net);
    return(ret);
}

struct fsm *fsm_contains_opt_one(struct fsm *net) {
  /* $.A | ~$A */
    struct fsm *ret;
    ret = fsm_union(fsm_contains_one(fsm_copy(net)),fsm_complement(fsm_contains(fsm_copy(net))));
    fsm_destroy(net);
    return(ret);
}

struct fsm *fsm_simple_replace(struct fsm *net1, struct fsm *net2) {
  /* [~[?* [A-0] ?*] [A.x.B]]* ~[?* [A-0] ?*] */
  
    struct fsm *UPlus, *ret;
    UPlus = fsm_minimize(fsm_kleene_plus(fsm_identity()));
    ret = fsm_concat(fsm_minimize(fsm_kleene_star(fsm_minimize(fsm_concat(fsm_complement(fsm_minimize(fsm_concat(fsm_concat(fsm_universal(),fsm_minimize(fsm_intersect(fsm_copy(net1),fsm_copy(UPlus)))),fsm_universal()))),fsm_minimize(fsm_cross_product(fsm_copy(net1),fsm_copy(net2))))))),fsm_minimize(fsm_complement(fsm_minimize(fsm_concat(fsm_concat(fsm_universal(), fsm_intersect(fsm_copy(net1),fsm_copy(UPlus))),fsm_universal())))));
    fsm_destroy(net1);
    fsm_destroy(net2);
    fsm_destroy(UPlus);
    return(ret);
}

struct fsm *fsm_priority_union_upper(struct fsm *net1, struct fsm *net2) {
    /* A .P. B = A | [~[A.u] .o. B] */
    struct fsm *ret;
    ret = fsm_union(fsm_copy(net1),fsm_compose(fsm_complement(fsm_upper(fsm_copy(net1))),net2));
    fsm_destroy(net1);
    return(ret);
}

struct fsm *fsm_priority_union_lower(struct fsm *net1, struct fsm *net2) {
    /* A .p. B = A | B .o. ~[A.l] */
    struct fsm *ret;
    ret = fsm_union(fsm_copy(net1),fsm_compose(net2,fsm_complement(fsm_lower(fsm_copy(net1)))));
    fsm_destroy(net1);
    return(ret);
}

struct fsm *fsm_lenient_compose(struct fsm *net1, struct fsm *net2) {
    /* A .O. B = [A .o. B] .P. B */
    struct fsm *ret;
    ret = fsm_priority_union_upper(fsm_compose(fsm_copy(net1),net2),fsm_copy(net1));
    fsm_destroy(net1);
    return(ret);
}

struct fsm *fsm_term_negation(struct fsm *net1) {
    return(fsm_intersect(fsm_identity(),fsm_complement(net1)));
}

struct fsm *fsm_quotient_interleave(struct fsm *net1, struct fsm *net2) {
    /* A/\/B = The set of strings you can interleave in B and get a string from A */
    /* [B/[x \x* x] & A/x .o. [[[\x]:0]* (x:0 \x* x:0)]*].l */
    struct fsm *Result;
    Result = fsm_lower(fsm_compose(fsm_intersect(fsm_ignore(net2,fsm_concat(fsm_symbol("@>@"),fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol("@>@"))),fsm_symbol("@>@"))),OP_IGNORE_ALL),fsm_ignore(net1,fsm_symbol("@>@"),OP_IGNORE_ALL)),fsm_kleene_star(fsm_concat(fsm_kleene_star(fsm_cross_product(fsm_term_negation(fsm_symbol("@>@")),fsm_empty_string())),fsm_optionality(fsm_concat(fsm_cross_product(fsm_symbol("@>@"),fsm_empty_string()),fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol("@>@"))),fsm_cross_product(fsm_symbol("@>@"),fsm_empty_string()))))))));

    Result->sigma = sigma_remove("@>@",Result->sigma);
    /* Could clean up sigma */
    return(Result);
}

struct fsm *fsm_quotient_left(struct fsm *net1, struct fsm *net2) {
    /* A\\\B = [B .o. A:0 ?*].l; */
    /* A\\\B = the set of suffixes you can add to A to get a string in B */
    struct fsm *Result;
    Result = fsm_lower(fsm_compose(net2,fsm_concat(fsm_cross_product(net1,fsm_empty_string()),fsm_universal())));
    return(Result);
}

struct fsm *fsm_quotient_right(struct fsm *net1, struct fsm *net2) {
    struct fsm *Result;
    
    /* A///B = [A .o. ?* B:0].l; */
    /* A///B = the set of prefixes you can add to B to get strings in A */
    Result = fsm_lower(fsm_compose(net1, fsm_concat(fsm_universal(),fsm_cross_product(net2,fsm_empty_string()))));
    return(Result);
}

struct fsm *fsm_ignore(struct fsm *net1, struct fsm *net2, int operation) {
  struct fsm_state *fsm1, *fsm2, *new_fsm;
  struct fsm *Result;
  short *handled_states1, *handled_states2;
  int i, j, k, state_add_counter = 0, malloc_size, splices = 0, returns, target, splice_size, start_splice, states1, states2, lines1, lines2, *return_state;

  net1 = fsm_minimize(net1);
  net2 = fsm_minimize(net2);

  if (fsm_isempty(net2)) {
      fsm_destroy(net2);
      return(net1);
  }
  fsm_merge_sigma(net1, net2);

  fsm_count(net1);
  fsm_count(net2);

  states1 = net1->statecount;
  states2 = net2->statecount;
  lines1 = net1->linecount;
  lines2 = net2->linecount;
  fsm1 = net1->states;
  fsm2 = net2->states;

  if (operation == OP_IGNORE_INTERNAL) {
    Result = fsm_lower(fsm_compose(fsm_ignore(fsm_copy(net1),fsm_symbol("@i<@"),OP_IGNORE_ALL),fsm_compose(fsm_complement(fsm_union(fsm_concat(fsm_symbol("@i<@"),fsm_universal()),fsm_concat(fsm_universal(),fsm_symbol("@i<@")))),fsm_simple_replace(fsm_symbol("@i<@"),fsm_copy(net2)))));
    Result->sigma = sigma_remove("@i<@",Result->sigma);
    fsm_destroy(net1);
    fsm_destroy(net2);
    return(Result);
  }

  malloc_size = lines1 + (states1 * (lines2 + net2->finalcount + 1));
  new_fsm = xxmalloc(sizeof(struct fsm_state)*(malloc_size+1));

  /* Mark if a state has been handled with ignore */
  handled_states1 = xxmalloc(sizeof(short)*states1);
  handled_states2 = xxmalloc(sizeof(short)*states2);

  /* Mark which ignores return to which state */
  return_state = xxmalloc(sizeof(int)*states1);
  splice_size = states2;
  start_splice = states1;
  for (k=0; k<states1; k++)
    *(handled_states1+k) = 0;
 
  for (i=0, j=0; (fsm1+i)->state_no != -1; i++) {
    if (*(handled_states1+(fsm1+i)->state_no) == 0) {
      target =  start_splice + splices * splice_size;
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, EPSILON, EPSILON, target, (fsm1+i)->final_state, (fsm1+i)->start_state);
      *(return_state+splices) = (fsm1+i)->state_no;
      *(handled_states1+(fsm1+i)->state_no) = 1;
      j++;
      splices++;
      if ((fsm1+i)->in != -1) {
	add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, (fsm1+i)->in, (fsm1+i)->out, (fsm1+i)->target, (fsm1+i)->final_state, (fsm1+i)->start_state);
	j++;
      }
    } else {
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, (fsm1+i)->in, (fsm1+i)->out, (fsm1+i)->target, (fsm1+i)->final_state, (fsm1+i)->start_state);
      j++;
    }
  }

  /* Add a sequence of fsm2s at the end, with arcs back to the appropriate states */

  state_add_counter = start_splice;

  for (returns = 0; splices>0; splices--, returns++) {
    /* Zero handled return arc states */

    for (k=0; k<states2; k++)
	 *(handled_states2+k) = 0;
    
    for (i=0; (fsm2+i)->state_no != -1; i++) {
      if ((fsm2+i)->final_state == 1 && *(handled_states2+(fsm2+i)->state_no) == 0) {
	add_fsm_arc(new_fsm, j, (fsm2+i)->state_no + state_add_counter, EPSILON, EPSILON, *(return_state+returns), 0, 0);
	j++;
	*(handled_states2+(fsm2+i)->state_no) = 1;
	if ((fsm2+i)->target != -1) {
	  add_fsm_arc(new_fsm, j, (fsm2+i)->state_no + state_add_counter, (fsm2+i)->in, (fsm2+i)->out , (fsm2+i)->target + state_add_counter, 0, 0);
	  j++;
	}
      } else {
	add_fsm_arc(new_fsm, j, (fsm2+i)->state_no + state_add_counter, (fsm2+i)->in, (fsm2+i)->out, (fsm2+i)->target + state_add_counter, 0, 0);
	j++;
      }
    }
    state_add_counter = state_add_counter + states2;
  }

  add_fsm_arc(new_fsm, j, -1, -1, -1, -1, -1, -1);
  xxfree(handled_states1);
  xxfree(handled_states2);
  xxfree(return_state);
  xxfree(net1->states);
  fsm_destroy(net2);
  net1->states = new_fsm;
  fsm_update_flags(net1, NO, NO, NO, NO, NO, NO);
  fsm_count(net1);
  return(net1);
}

/* Remove those symbols from sigma that have the same distribution as IDENTITY */

void fsm_compact(struct fsm *net) {
    struct checktable {
        int state_no;
        int target;
    } *checktable;

    struct fsm_state *fsm;
    struct sigma *sig, *sigprev, *sign;
    _Bool *potential;
    int i, j, prevstate, numsymbols, in, out, state, target, removable;

    fsm = net->states;
    numsymbols = sigma_max(net->sigma);
    
    potential = xxmalloc(sizeof(_Bool)*(numsymbols+1));
    checktable = xxmalloc(sizeof(struct checktable)*(numsymbols+1));

    for (i=0; i <= numsymbols; i++) {
        *(potential+i) =  1;
        (checktable+i)->state_no = -1;
        (checktable+i)->target = -1;
    }
    /* For consistency reasons, can't remove symbols longer than 1 */
    /* since @ and ? only match utf8 symbols of length 1           */

    for (sig = net->sigma; sig != NULL && sig->number != -1; sig = sig->next) {
	if (utf8strlen(sig->symbol) > 1) {
	    *(potential+sig->number) = 0;
	}
    }

    prevstate = 0;

    for (i=0;  ; i++) {

        if ((fsm+i)->state_no != prevstate) {
            for (j=3; j<=numsymbols;j++) {
                if ((checktable+j)->state_no != prevstate && (checktable+IDENTITY)->state_no != prevstate) {
                    continue;
                }
                if ((checktable+j)->target == (checktable+IDENTITY)->target && (checktable+j)->state_no == (checktable+IDENTITY)->state_no) {
                    continue;
                }
                *(potential+j) = 0;
            }
        }        

        if ((fsm+i)->state_no == -1)
            break;

        in = (fsm+i)->in;
        out = (fsm+i)->out;
        state = (fsm+i)->state_no;
        target = (fsm+i)->target;

        if (in != -1 && out != -1) {
            if (((in == out && in > 2) || in == IDENTITY)) {
                (checktable+in)->state_no = state;
                (checktable+in)->target = target;
            }
            if (in != out && in > 2) {
                *(potential+in) = 0;
            }
            if (in != out && out > 2) {
                *(potential+out) = 0;
            }
        }
        prevstate = state;
    }
    for (removable = 0, i=3; i <= numsymbols; i++) {
        if (*(potential+i) == 1) {
            removable = 1;
        }

    }
    if (removable == 0) {
        xxfree(potential);
        xxfree(checktable);
        return;
    }
    i = j = 0;
    do {
        in = (fsm+i)->in;

        add_fsm_arc(fsm, j ,(fsm+i)->state_no,(fsm+i)->in,(fsm+i)->out,(fsm+i)->target,(fsm+i)->final_state,(fsm+i)->start_state);
        if (in == -1) {
            i++;
            j++;
        }
        else if (*(potential+in) == 1 && in > 2) {
            i++;    
        } else {
            i++; 
            j++;
        }
    } while ((fsm+i)->state_no != -1);
    add_fsm_arc(fsm, j ,(fsm+i)->state_no,(fsm+i)->in,(fsm+i)->out,(fsm+i)->target,(fsm+i)->final_state,(fsm+i)->start_state);

    sigprev = NULL;
    for (sig = net->sigma; sig != NULL && sig->number != -1; sig = sign) {

        if ((sig->number > 2) && (*(potential+sig->number) == 1)) {
            sigprev->next = sig->next;
            sign = sig->next;
            xxfree(sig->symbol);
            xxfree(sig);
        } else {
            sigprev = sig;
            sign = sig->next;
        }
    }    
    xxfree(potential);
    xxfree(checktable);
    sigma_cleanup(net,0);
}

int fsm_symbol_occurs(struct fsm *net, char *symbol, int side) {
    struct fsm_state *fsm;
    int i, sym;
    sym = sigma_find(symbol, net->sigma);
    if (sym == -1) {
        return 0;
    }
    for (i=0, fsm = net->states; (fsm+i)->state_no != -1; i++) {
        if (side == M_UPPER && (fsm+i)->in == sym) 
            return 1;
        if (side == M_LOWER && (fsm+i)->out == sym)
            return 1;
        if (side == (M_UPPER + M_LOWER) && ( (fsm+i)->in == sym || (fsm+i)->out == sym))
            return 1;
    }
    return 0;
}

struct fsm *fsm_equal_substrings(struct fsm *net, struct fsm *left, struct fsm *right) {

    /* The algorithm extracts from the lower side all and only those strings where   */
    /* every X occurring in different substrings ... left X right ... is identical.  */

    /* Caveat: there is no reliable termination condition for the loop that extracts */
    /* identities.  This means that if run on languages where there are potentially  */ 
    /* infinite-length identical delimited substrings, it will not terminate.        */

    /* For example: _eq(l a* r l a* r, l , r) will not terminate.                    */

    /* However, even if the languages occuring between left and right are infinite   */
    /* the algorithm terminates eventually if the the possible combinations of       */
    /* identical substrings is finite in length.                                     */

    /* For example _eq([l a* b r l a b* r]*, l, r) does terminate even though        */
    /* it contains a potentially infinite number of delimited substrings since       */
    /* the maximum length of the possible identical delimited substrings is finite.  */

    /* In this case the above example evaluates to the language [l a b r l a b r]*   */

    /* The algorithm: */
    /* Input: L, left, right */
    /* Output: the language that preserves only those strings in L */
    /*         where X is the same for all left X right sequences  */

    /* 1. mark all instances of left with ... LB and right with RB ... in L   */

    /* 2. split L into Leq and Lbypass                                        */
    /*    where Lbypass are all those strings where LB RB sequences aren't    */
    /*    properly nested (we must have ... LB ... RB ... LB ... RB ... etc.) */
    /*    Lbypass also includes all those with less than two bracketed        */
    /*    instances, since we don't need to check equality if there's only    */
    /*    one bracketed substring.                                            */
    /*    We also remove the auxiliary LB and RB symbols from Lbypass         */

    /* 3. We extract all the possible symbols occurring between LB and RB     */
    /*    from Leq                                                            */

    /* 4. We create the transducer Move from all symbols in (3)               */
    /*    Move = M(sym_1) | ... | M(sym_n)                                    */
    /*    where M(a) is defined as [\LB* LB:a a:LB]* \LB*                     */
    /*    i.e. it rewrites bracketed strings such as "LB a b RB LB a b RB"    */
    /*    to "a LB b RB a LB b RB" in effect moving brackets to the right     */
    /*    one step for a symbol.                                              */

    /* 5. Leq = Cleanup(Leq)                                                  */
    /*    Cleanup removes LB RB sequences and, at the same time filters out   */
    /*    any strings where we find both LB RB and LB X RB where X is not 0.  */ 
    /*    since we know such sequences could not possibly be identical        */
    /*    Cleanup is implemented by composing Leq with                        */
    /*    \LB* [LB:0 RB:0 \LB*]* | ~$[LB RB]                                  */
    /*    - if the symbol LB does not occur on the lower side of Leq, goto(6) */
    /*    - else Leq = Move(Leq), goto(5)                                     */

    /* 6. Result = L .o. [Leq | Lbypass]                                      */

    int syms;
    struct sigma *sig;
    struct fsm *LB, *RB, *NOLB, *NORB, *InsertBrackets, *RemoveBrackets, *Lbracketed, *NOBR, *BracketFilter, *Lbypass, *Leq, *Labels, *Cleanup, *ThisMove, *ThisSymbol, *Move, *Result, *oldnet;

    oldnet = fsm_copy(net);

    /* LB = "@<eq<@" */
    /* RB = "@>eq>@" */
    
    LB = fsm_symbol("@<eq<@");
    NOLB = fsm_minimize(fsm_term_negation(fsm_copy(LB)));
    RB = fsm_symbol("@>eq>@");
    NORB = fsm_minimize(fsm_term_negation(fsm_copy(RB)));
    /* NOBR = ~$[LB|RB] */
    NOBR = fsm_minimize(fsm_complement(fsm_contains(fsm_union(fsm_copy(LB),fsm_copy(RB)))));

    sigma_add("@<eq<@", net->sigma);
    sigma_add("@>eq>@", net->sigma);
    sigma_sort(net);

    /* Insert our aux markers into the language                */

    /* InsertBrackets = [~$[L|R] [L 0:LB|0:RB R]]* ~$[L|R];    */

    InsertBrackets = fsm_minimize(fsm_concat(fsm_kleene_star(fsm_concat(fsm_complement(fsm_contains(fsm_union(fsm_copy(left),fsm_copy(right)))),fsm_union(fsm_concat(fsm_copy(left),fsm_cross_product(fsm_empty_string(),fsm_copy(LB))),fsm_concat(fsm_cross_product(fsm_empty_string(),fsm_copy(RB)),fsm_copy(right))))),fsm_complement(fsm_contains(fsm_union(fsm_copy(left),fsm_copy(right))))));
    

    /* Lbracketed = L .o. InsertBrackets                       */

    Lbracketed = fsm_compose(fsm_copy(net), InsertBrackets);

    /* Filter out improper nestings, or languages with less than two marker pairs */

    /* BracketFilter = NOBR LB NOBR RB NOBR [LB NOBR RB NOBR]+  */

    BracketFilter = fsm_concat(fsm_copy(NOBR),fsm_concat(fsm_copy(LB),fsm_concat(fsm_copy(NOBR),fsm_concat(fsm_copy(RB),fsm_concat(fsm_copy(NOBR),fsm_kleene_plus(fsm_concat(fsm_copy(LB),fsm_concat(fsm_copy(NOBR),fsm_concat(fsm_copy(RB),fsm_copy(NOBR))))))))));

    /* RemoveBrackets = [LB:0|RB:0|NOBR]*                       */
    /* Lbypass = [Lbracketed .o. ~BracketFilter .o. LB|RB -> 0] */
    /* Leq     = [Lbracketed .o.  BracketFilter]                */

    RemoveBrackets = fsm_kleene_star(fsm_union(fsm_cross_product(fsm_copy(LB),fsm_empty_string()),fsm_union(fsm_cross_product(fsm_copy(RB),fsm_empty_string()),fsm_copy(NOBR))));


    Lbypass = fsm_lower(fsm_compose(fsm_copy(Lbracketed),fsm_compose(fsm_complement(fsm_copy(BracketFilter)),RemoveBrackets)));
    Leq     = fsm_compose(Lbracketed, BracketFilter);

    /* Extract labels from lower side of L */
    /* [Leq .o. [\LB:0* LB:0 \RB* RB:0]* \LB:0*].l */
    
    Labels = fsm_sigma_pairs_net(fsm_lower(fsm_compose(fsm_copy(Leq),fsm_concat(fsm_kleene_star(fsm_concat(fsm_kleene_star(fsm_cross_product(fsm_copy(NOLB),fsm_empty_string())),fsm_concat(fsm_cross_product(fsm_copy(LB),fsm_empty_string()),fsm_concat(fsm_kleene_star(fsm_copy(NORB)),fsm_cross_product(fsm_copy(RB),fsm_empty_string()))))),fsm_kleene_star(fsm_cross_product(fsm_copy(NOLB),fsm_empty_string()))))));

    /* Cleanup = \LB* [LB:0 RB:0 \LB*]* | ~$[LB RB] */

    Cleanup = fsm_minimize(fsm_union(fsm_concat(fsm_kleene_star(fsm_copy(NOLB)),fsm_kleene_star(fsm_concat(fsm_cross_product(fsm_copy(LB),fsm_empty_string()),fsm_concat(fsm_cross_product(fsm_copy(RB),fsm_empty_string()),fsm_kleene_star(fsm_copy(NOLB)))))),fsm_complement(fsm_contains(fsm_concat(fsm_copy(LB),fsm_copy(RB))))));

    /* Construct the move function */
    
    Move = fsm_empty_string();

    syms = 0;
    for (sig = Labels->sigma; sig != NULL; sig = sig->next) {
        /* Unclear which is faster: the first or the second version */
        /* ThisMove = [\LB* LB:X X:LB]* \LB*       */
        /* ThisMove = [\LB* LB:0 X 0:LB]* \LB*     */
        if (sig->number >= 3) {
            ThisSymbol = fsm_symbol(sig->symbol);
            //ThisMove = fsm_concat(fsm_kleene_star(fsm_concat(fsm_kleene_star(fsm_copy(NOLB)),fsm_concat(fsm_cross_product(fsm_copy(LB),fsm_copy(ThisSymbol)),fsm_cross_product(fsm_copy(ThisSymbol),fsm_copy(LB))))), fsm_kleene_star(fsm_copy(NOLB)));
            ThisMove = fsm_concat(fsm_kleene_star(fsm_concat(fsm_kleene_star(fsm_copy(NOLB)),fsm_concat(fsm_cross_product(fsm_copy(LB),fsm_empty_string()), fsm_concat(fsm_copy(ThisSymbol), fsm_cross_product(fsm_empty_string(),fsm_copy(LB)))))), fsm_kleene_star(fsm_copy(NOLB)));
            
            Move = fsm_union(Move, ThisMove);
            syms++;
        }
    }
    Move = fsm_minimize(Move);
    if (syms == 0) {
        //printf("no syms");
        fsm_destroy(net);
        return(oldnet);
    }

    /* Move until no bracket symbols remain */
    for (;;) {
        //printf("Zapping\n");
        Leq = fsm_compose(Leq, fsm_copy(Cleanup));
        if (!fsm_symbol_occurs(Leq, "@<eq<@", M_LOWER))
            break;
        Leq = fsm_compose(Leq, fsm_copy(Move));
        //Leq = fsm_minimize(fsm_compose(Leq, fsm_copy(Move)));
        //        printf("size: %i\n",Leq->statecount);
    }
    
    /* Result = L .o. [Leq | Lbypass] */
    Result = fsm_minimize(fsm_compose(net, fsm_union(fsm_lower(Leq), Lbypass)));
    sigma_remove("@<eq<@", Result->sigma);
    sigma_remove("@>eq>@", Result->sigma);
    fsm_compact(Result);
    sigma_sort(Result);
    fsm_destroy(oldnet);
    return(Result);
}

struct fsm *fsm_invert(struct fsm *net) {
  struct fsm_state *fsm;
  int i, temp;

  fsm = net->states;
  for (i = 0; (fsm+i)->state_no != -1; i++) {
    temp = (fsm+i)->in;
    (fsm+i)->in = (fsm+i)->out;
    (fsm+i)->out = temp;
  }
  i = net->arcs_sorted_in;
  net->arcs_sorted_in = net->arcs_sorted_out;
  net->arcs_sorted_out = i;
  return (net);
}

struct fsm *fsm_sequentialize(struct fsm *net) {
  printf("Implementation pending\n");
  return(net);
}


struct fsm *fsm_bimachine(struct fsm *net) {
    printf("implementation pending\n");
    return(net);
}

/* _leftrewr(L, a:b) does a -> b || .#. L _    */
/* _leftrewr(?* L, a:b) does a -> b || L _     */
/* works only with single symbols, but is fast */

struct fsm *fsm_left_rewr(struct fsm *net, struct fsm *rewr) {
    struct fsm_construct_handle *outh;
    struct fsm_read_handle *inh;
    struct fsm *newnet;
    int i, maxsigma, *sigmatable, currstate, sinkstate, seensource, innum, outnum, relabelin, relabelout, addedsink;

    fsm_merge_sigma(net, rewr);
    relabelin = rewr->states->in;
    relabelout = rewr->states->out;

    inh = fsm_read_init(net);
    sinkstate = fsm_get_num_states(inh);
    outh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(outh, net->sigma);
    maxsigma = sigma_max(net->sigma);
    maxsigma++;
    sigmatable = xxmalloc(maxsigma * sizeof(int));
    for (i = 0; i < maxsigma; i++) {
	*(sigmatable+i) = -1;
    }
    addedsink = 0;
    while ((currstate = fsm_get_next_state(inh)) != -1) {
	seensource = 0;
	fsm_construct_set_final(outh, currstate);

	while (fsm_get_next_state_arc(inh)) {
	    innum = fsm_get_arc_num_in(inh);
	    outnum = fsm_get_arc_num_out(inh);
	    *(sigmatable+innum) = currstate;
	    if (innum == relabelin) {
		    seensource = 1;
		    if (fsm_read_is_final(inh, currstate)) {
			outnum = relabelout;		    
		    }
	    }
	    fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), fsm_get_arc_target(inh), innum, outnum);
	}
	for (i = 2; i < maxsigma; i++) {
	    if (*(sigmatable+i) != currstate && i != relabelin) {
		fsm_construct_add_arc_nums(outh, currstate, sinkstate, i, i);
		addedsink = 1;
	    }
	}
	if (seensource == 0) {
	    addedsink = 1;
	    if (fsm_read_is_final(inh, currstate)) {
		fsm_construct_add_arc_nums(outh, currstate, sinkstate, relabelin, relabelout);
	    } else {
		fsm_construct_add_arc_nums(outh, currstate, sinkstate, relabelin, relabelin);
	    }
	}
    }
    if (addedsink) {
	for (i = 2; i < maxsigma; i++) {
	    fsm_construct_add_arc_nums(outh, sinkstate, sinkstate, i, i);
	}
	fsm_construct_set_final(outh, sinkstate);
    }
    fsm_construct_set_initial(outh, 0);
    fsm_read_done(inh);
    newnet = fsm_construct_done(outh);
    xxfree(sigmatable);
    fsm_destroy(net);
    fsm_destroy(rewr);
    return(newnet);
}

struct fsm *fsm_add_sink(struct fsm *net, int final) {
    struct fsm_construct_handle *outh;
    struct fsm_read_handle *inh;
    struct fsm *newnet;
    int i, maxsigma, *sigmatable, currstate, sinkstate;

    inh = fsm_read_init(net);
    sinkstate = fsm_get_num_states(inh);
    outh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(outh, net->sigma);
    maxsigma = sigma_max(net->sigma);
    maxsigma++;
    sigmatable = xxmalloc(maxsigma * sizeof(int));
    for (i = 0; i < maxsigma; i++) {
	*(sigmatable+i) = -1;
    }
    while ((currstate = fsm_get_next_state(inh)) != -1) {
	while (fsm_get_next_state_arc(inh)) {
	    fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), fsm_get_arc_target(inh), fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
	    *(sigmatable+fsm_get_arc_num_in(inh)) = currstate;
	}
	for (i = 2; i < maxsigma; i++) {
	    if (*(sigmatable+i) != currstate) {
		fsm_construct_add_arc_nums(outh, currstate, sinkstate, i, i);
	    }
	}
    }
    for (i = 2; i < maxsigma; i++) {
	fsm_construct_add_arc_nums(outh, sinkstate, sinkstate, i, i);
    }

    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(outh, i);
    }
    if (final == 1) {
	fsm_construct_set_final(outh, sinkstate);
    }
    fsm_construct_set_initial(outh, 0);
    fsm_read_done(inh);
    newnet = fsm_construct_done(outh);
    fsm_destroy(net);
    return(newnet);
}

/* _addfinalloop(L, "#":0) adds "#":0 at all final states */
/* _addnonfinalloop(L, "#":0) adds "#":0 at all nonfinal states */
/* _addloop(L, "#":0) adds "#":0 at all states */

/* Adds loops at finals = 0 nonfinals, finals = 1 finals, finals = 2, all */

struct fsm *fsm_add_loop(struct fsm *net, struct fsm *marker, int finals) {
    struct fsm *newnet;
    struct fsm_construct_handle *outh;
    struct fsm_read_handle *inh, *minh;
    int i;

    inh = fsm_read_init(net);
    minh = fsm_read_init(marker);

    outh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(outh, net->sigma);
    
    while (fsm_get_next_arc(inh)) {
	fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), fsm_get_arc_target(inh), fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
    }
    /* Where to put the loops */
    if (finals == 1) {
	while ((i = fsm_get_next_final(inh)) != -1) {
	    fsm_construct_set_final(outh, i);
	    fsm_read_reset(minh);
	    while (fsm_get_next_arc(minh)) {
		fsm_construct_add_arc(outh, i, i, fsm_get_arc_in(minh), fsm_get_arc_out(minh));
	    }
	}
    } else if (finals == 0 || finals == 2) {
	for (i=0; i < net->statecount; i++) {
	    if (finals == 2 || !fsm_read_is_final(inh, i)) {
		fsm_read_reset(minh);
		while (fsm_get_next_arc(minh)) {
		    fsm_construct_add_arc(outh, i, i, fsm_get_arc_in(minh), fsm_get_arc_out(minh));
		}
	    }
	}
    }
    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(outh, i);
    }
    fsm_construct_set_initial(outh, 0);
    fsm_read_done(inh);
    fsm_read_done(minh);
    newnet = fsm_construct_done(outh);
    fsm_destroy(net);
    return(newnet);
}

/* _marktail(?* L, 0:x) does ~$x .o. [..] -> x || L _ ;   */
/* _marktail(?* R.r, 0:x).r does ~$x .o. [..] -> x || _ R */

struct fsm *fsm_mark_fsm_tail(struct fsm *net, struct fsm *marker) {
    struct fsm *newnet;
    struct fsm_construct_handle *outh;
    struct fsm_read_handle *inh, *minh;
    int i, *mappings, maxstate, target, newtarget;

    inh = fsm_read_init(net);
    minh = fsm_read_init(marker);

    outh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(outh, net->sigma);

    mappings = xxcalloc(net->statecount, sizeof(int));
    maxstate = net->statecount;
    
    while (fsm_get_next_arc(inh)) {
	target = fsm_get_arc_target(inh);
	if (fsm_read_is_final(inh, target)) {
	    if (!*(mappings+target)) {
		newtarget = maxstate;
		*(mappings+target) = newtarget;
		fsm_read_reset(minh);
		while (fsm_get_next_arc(minh)) {
		    fsm_construct_add_arc(outh, newtarget, target, fsm_get_arc_in(minh), fsm_get_arc_out(minh));
		}
		maxstate++;
	    } else {
		newtarget = *(mappings+target);
	    }
	    fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), newtarget, fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
	} else {
	    fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), target, fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
	}
    }
    for (i=0; i < net->statecount; i++) {
	fsm_construct_set_final(outh,i);
    }

    fsm_construct_set_initial(outh, 0);
    fsm_read_done(inh);
    fsm_read_done(minh);
    newnet = fsm_construct_done(outh);
    fsm_destroy(net);
    xxfree(mappings);
    return(newnet);
}

struct fsm *fsm_flatten(struct fsm *net, struct fsm *epsilon) {
    struct fsm *newnet;
    struct fsm_construct_handle *outh;
    struct fsm_read_handle *inh, *eps;
    int i, maxstate, in, out, target;
    char *epssym, *instring, *outstring;
    
    inh = fsm_read_init(net);
    eps = fsm_read_init(epsilon);
    if (fsm_get_next_arc(eps) == -1) {
	fsm_destroy(net);
	fsm_destroy(epsilon);
	return NULL;
    }
    epssym = strdup(fsm_get_arc_in(eps));
    fsm_read_done(eps);

    outh = fsm_construct_init(net->name);
    maxstate = net->statecount;

    fsm_construct_copy_sigma(outh, net->sigma);

    while (fsm_get_next_arc(inh)) {
	target = fsm_get_arc_target(inh);	
	in = fsm_get_arc_num_in(inh);
	out = fsm_get_arc_num_out(inh);
	if (in == EPSILON || out == EPSILON)  { 
	    instring = fsm_get_arc_in(inh);
	    outstring = fsm_get_arc_out(inh);
	    if (in == EPSILON)  { instring = epssym; }
	    if (out == EPSILON) { outstring = epssym; }	    

	    fsm_construct_add_arc(outh, fsm_get_arc_source(inh), maxstate, instring, instring);
	    fsm_construct_add_arc(outh, maxstate, target, outstring, outstring);
	} else {
	    fsm_construct_add_arc_nums(outh, fsm_get_arc_source(inh), maxstate, in, in);
	    fsm_construct_add_arc_nums(outh, maxstate, target, out, out);
	}
	maxstate++;
    }
    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(outh, i);
    }
    while ((i = fsm_get_next_initial(inh)) != -1) {
	fsm_construct_set_initial(outh, i);
    }

    fsm_read_done(inh);
    newnet = fsm_construct_done(outh);
    fsm_destroy(net);
    fsm_destroy(epsilon);
    xxfree(epssym);
    return(newnet);
}

/* Removes IDENTITY and UNKNOWN transitions. If mode = 1, only removes UNKNOWNs */
struct fsm *fsm_close_sigma(struct fsm *net, int mode) {
    struct fsm *newnet;
    struct fsm_construct_handle *newh;
    struct fsm_read_handle *inh;
    int i;

    inh = fsm_read_init(net);
    newh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(newh, net->sigma);

    while (fsm_get_next_arc(inh)) {
	if ((fsm_get_arc_num_in(inh) != UNKNOWN && fsm_get_arc_num_in(inh) != IDENTITY && fsm_get_arc_num_out(inh) != UNKNOWN && fsm_get_arc_num_out(inh) != IDENTITY) || (mode == 1 && fsm_get_arc_num_in(inh) != UNKNOWN && fsm_get_arc_num_out(inh) != UNKNOWN))
	    fsm_construct_add_arc_nums(newh, fsm_get_arc_source(inh), fsm_get_arc_target(inh), fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
    }
    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_set_final(newh, i);
    }
    while ((i = fsm_get_next_initial(inh)) != -1) {
	fsm_construct_set_initial(newh, i);
    }
    fsm_read_done(inh);
    newnet = fsm_construct_done(newh);
    fsm_destroy(net);
    return(fsm_minimize(newnet));
}
