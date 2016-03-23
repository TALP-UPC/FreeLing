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
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "foma.h"

#define SUBSET_EPSILON_REMOVE 1
#define SUBSET_DETERMINIZE 2
#define SUBSET_TEST_STAR_FREE 3

#define NHASH_LOAD_LIMIT 2 /* load limit for nhash table size */

static int fsm_linecount, num_states, num_symbols, epsilon_symbol, *single_sigma_array, *double_sigma_array, limit, num_start_states, op;

static _Bool *finals, deterministic, numss;

struct e_closure_memo {
    int state;
    int mark;
    struct e_closure_memo *target;
    struct e_closure_memo *next;
};

static unsigned int primes[26] = {61,127,251,509,1021,2039,4093,8191,16381,32749,65521,131071,262139,524287,1048573,2097143,4194301,8388593,16777213,33554393,67108859,134217689,268435399,536870909,1073741789,2147483647};

static struct e_closure_memo *e_closure_memo;

int T_last_unmarked, T_limit;

struct nhash_list {
    int setnum;
    unsigned int size;
    unsigned int set_offset;
    struct nhash_list *next;
};

struct T_memo {
    unsigned char finalstart;
    unsigned int size;
    unsigned int set_offset;
};

struct trans_list {
    int inout;
    int target;
} *trans_list;

struct trans_array {
    struct trans_list *transitions;
    unsigned int size;
    unsigned int tail;
} *trans_array;

static struct T_memo *T_ptr;

static int nhash_tablesize, nhash_load, current_setnum, *e_table, *marktable, *temp_move, mainloop, maxsigma, *set_table, set_table_size, star_free_mark;
static unsigned int set_table_offset;
static struct nhash_list *table;

extern int add_fsm_arc(struct fsm_state *fsm, int offset, int state_no, int in, int out, int target, int final_state, int start_state);

static void init(struct fsm *net);
INLINE static int e_closure(int states);
INLINE static int set_lookup(int *lookup_table, int size);
static int initial_e_closure(struct fsm *network);
static void memoize_e_closure(struct fsm_state *fsm);
static int next_unmarked(void);
static void single_symbol_to_symbol_pair(int symbol, int *symbol_in, int *symbol_out);
static int symbol_pair_to_single_symbol(int in, int out);
static void sigma_to_pairs(struct fsm *net);
static int nhash_find_insert(int *set, int setsize);
INLINE static int hashf(int *set, int setsize);
static int nhash_insert(int hashval, int *set, int setsize);
static void nhash_rebuild_table ();
static void nhash_init (int initial_size);
static void nhash_free(struct nhash_list *nptr, int size);
static void e_closure_free();
static void init_trans_array(struct fsm *net);
static struct fsm *fsm_subset(struct fsm *net, int operation);

struct fsm *fsm_epsilon_remove(struct fsm *net) {
    return(fsm_subset(net, SUBSET_EPSILON_REMOVE));
}

struct fsm *fsm_determinize(struct fsm *net) {
    return(fsm_subset(net, SUBSET_DETERMINIZE));
}

int fsm_isstarfree(struct fsm *net) {
    #define DFS_WHITE 0
    #define DFS_GRAY 1
    #define DFS_BLACK 2

    struct fsm *sfnet;
    struct state_array *state_array;
    struct fsm_state *curr_ptr;
    int v, vp, is_star_free;
    short int in;
    char *dfs_map;

    sfnet = fsm_subset(net, SUBSET_TEST_STAR_FREE);
    is_star_free = 1;

    state_array = map_firstlines(net);
    ptr_stack_clear();
    ptr_stack_push(state_array->transitions);

    dfs_map = xxcalloc(sfnet->statecount, sizeof(char));
    while(!ptr_stack_isempty()) {

        curr_ptr = ptr_stack_pop();
    nopop:
        v = curr_ptr->state_no; /* source state number */
        vp = curr_ptr->target;  /* target state number */

        if (v == -1 || vp == -1) {
            *(dfs_map+v) = DFS_BLACK;
            continue;
        }
        *(dfs_map+v) = DFS_GRAY;

        in = curr_ptr->in;
        if (*(dfs_map+vp) == DFS_GRAY && in == maxsigma) {
            /* Not star-free */
            is_star_free = 0;
            break;
        }
        if (v == (curr_ptr+1)->state_no) {
            ptr_stack_push(curr_ptr+1);
        }
        if (*(dfs_map+vp) == DFS_WHITE) { 
            curr_ptr = (state_array+vp)->transitions;
            goto nopop;
        }
    }
    ptr_stack_clear();
    xxfree(dfs_map);
    xxfree(state_array);
    //stack_add(sfnet);
    return(is_star_free);
}

static struct fsm *fsm_subset(struct fsm *net, int operation) {

    int T, U;
    
    if (net->is_deterministic == YES && operation != SUBSET_TEST_STAR_FREE) {
        return(net);
    }
    /* Export this var */
    op = operation;
    fsm_count(net);
    num_states = net->statecount;
    deterministic = 1;
    init(net);
    nhash_init((num_states < 12) ? 6 : num_states/2);
    
    T = initial_e_closure(net);

    int_stack_clear();
    
    if (deterministic == 1 && epsilon_symbol == -1 && num_start_states == 1 && numss == 0) {
        net->is_deterministic = YES;
        net->is_epsilon_free = YES;
        nhash_free(table, nhash_tablesize);
        xxfree(T_ptr);
        xxfree(e_table);
        xxfree(trans_list);
        xxfree(trans_array);
        xxfree(double_sigma_array);
        xxfree(single_sigma_array);
        xxfree(finals);
        xxfree(temp_move);
        xxfree(set_table);
        return(net);
    }

    if (operation == SUBSET_EPSILON_REMOVE && epsilon_symbol == -1) {
        net->is_epsilon_free = YES;
        nhash_free(table, nhash_tablesize);
        xxfree(T_ptr);
        xxfree(e_table);
        xxfree(trans_list);
        xxfree(trans_array);
        xxfree(double_sigma_array);
        xxfree(single_sigma_array);
        xxfree(finals);
        xxfree(temp_move);
        xxfree(set_table);
        return(net);
    }

    if (operation == SUBSET_TEST_STAR_FREE) {
        fsm_state_init(sigma_max(net->sigma)+1);
        star_free_mark = 0;
    } else {
        fsm_state_init(sigma_max(net->sigma));
        xxfree(net->states);
    }

    /* init */

    do {
        int i, j, tail, setsize, *theset, stateno, has_trans, minsym, next_minsym, trgt, symbol_in, symbol_out;
        struct trans_list *transitions;
        struct trans_array *tptr;

        fsm_state_set_current_state(T, (T_ptr+T)->finalstart, T == 0 ? 1 : 0);
        
        /* Prepare set */
        setsize = (T_ptr+T)->size;
        theset = set_table+(T_ptr+T)->set_offset;
        minsym = INT_MAX;
        has_trans = 0;
        for (i = 0; i < setsize; i++) {
            stateno = *(theset+i);
            tptr = trans_array+stateno;
            tptr->tail = 0;
            if (tptr->size == 0)
                continue;
            if ((tptr->transitions)->inout < minsym) {
                minsym = (tptr->transitions)->inout;
                has_trans = 1;
            }
        }
        if (!has_trans) {
            /* close state */
            fsm_state_end_state();
            continue;
        }
        
        /* While set not empty */

        for (next_minsym = INT_MAX; minsym != INT_MAX ; minsym = next_minsym, next_minsym = INT_MAX) {
            theset = set_table+(T_ptr+T)->set_offset;
            
            for (i = 0, j = 0 ; i < setsize; i++) {
                
                stateno = *(theset+i);
                tptr = trans_array+stateno;
                tail = tptr->tail;
                transitions = (tptr->transitions)+tail;
                
                while (tail < tptr->size &&  transitions->inout == minsym) {
                    trgt = transitions->target;
                    if (*(e_table+(trgt)) != mainloop) {
                        *(e_table+(trgt)) = mainloop;
                        *(temp_move+j) = trgt;
                        j++;
                        
                        if (operation == SUBSET_EPSILON_REMOVE) {
                            mainloop++;
                            if ((U = e_closure(j)) != -1) {
                                single_symbol_to_symbol_pair(minsym, &symbol_in, &symbol_out);
                                fsm_state_add_arc(T, symbol_in, symbol_out, U, (T_ptr+T)->finalstart, T == 0 ? 1 : 0);
                                j = 0;
                            }
                        }
                    }
                    tail++;
                    transitions++;
                }
                
                tptr->tail = tail;
                
                if (tail == tptr->size)
                    continue;
                /* Check next minsym */
                if (transitions->inout < next_minsym) {
                    next_minsym = transitions->inout;
                }
            }
            if (operation == SUBSET_DETERMINIZE) {
                mainloop++;
                if ((U = e_closure(j)) != -1) {
                    single_symbol_to_symbol_pair(minsym, &symbol_in, &symbol_out);
                    fsm_state_add_arc(T, symbol_in, symbol_out, U, (T_ptr+T)->finalstart, T == 0 ? 1 : 0);
                }
            }
            if (operation == SUBSET_TEST_STAR_FREE) {
                mainloop++;
                if ((U = e_closure(j)) != -1) {
                    single_symbol_to_symbol_pair(minsym, &symbol_in, &symbol_out);                   
                    fsm_state_add_arc(T, symbol_in, symbol_out, U, (T_ptr+T)->finalstart, T == 0 ? 1 : 0);
                    if (star_free_mark == 1) {
                        //fsm_state_add_arc(T, maxsigma, maxsigma, U, (T_ptr+T)->finalstart, T == 0 ? 1 : 0);
                        star_free_mark = 0;
                    }
                }
            }
        }
        /* end state */
        fsm_state_end_state();
    } while ((T = next_unmarked()) != -1);
    
    /* wrapup() */
    nhash_free(table, nhash_tablesize);
    xxfree(set_table);
    xxfree(T_ptr);
    xxfree(temp_move);
    xxfree(e_table);
    xxfree(trans_list);
    xxfree(trans_array);
    
    if (epsilon_symbol != -1)
        e_closure_free();
    xxfree(double_sigma_array);
    xxfree(single_sigma_array);
    xxfree(finals);
    fsm_state_close(net);
    return(net);
}

static void init(struct fsm *net) {
    /* A temporary table for handling epsilon closure */
    /* to avoid doubles */

    e_table = xxcalloc(net->statecount,sizeof(int));

    /* Counter for our access tables */

    mainloop = 1;

    /* Temporary table for storing sets and */
    /* passing to hash function */
    
    /* Table for listing current results of move & e-closure */
    temp_move = xxmalloc((net->statecount + 1) *sizeof(int));
    
    /* We malloc this much memory to begin with for the new fsm */
    /* Then grow it by the double as needed */

    limit = next_power_of_two(net->linecount);
    fsm_linecount = 0;
    sigma_to_pairs(net);

    /* Optimistically malloc T_ptr array */
    /* We allocate memory for a number of pointers to a set of states */
    /* To handle fast lookup in array */
    /* Optimistically, we choose the initial size to be the number of */
    /* states in the non-deterministic fsm */
    
    T_last_unmarked = 0;
    T_limit = next_power_of_two(num_states);

    T_ptr = xxcalloc(T_limit,sizeof(struct T_memo));

    /* Stores all sets consecutively in one table */
    /* T_ptr->set_offset and size                 */
    /* are used to retrieve the set               */

    set_table_size = next_power_of_two(num_states);
    set_table = xxmalloc(set_table_size*sizeof(int));
    set_table_offset = 0;

    init_trans_array(net);
}

static int trans_sort_cmp(const void *a, const void *b) {
  return (((const struct trans_list *)a)->inout - ((const struct trans_list *)b)->inout);
}

static void init_trans_array(struct fsm *net) {
    struct trans_list *arrptr;
    struct fsm_state *fsm;
    int i, j, laststate, lastsym, inout, size, state;

    arrptr = trans_list = xxmalloc(net->linecount * sizeof(struct trans_list));
    trans_array = xxcalloc(net->statecount, sizeof(struct trans_array));
    
    laststate = -1;
    fsm = net->states;

    for (i=0, size = 0; (fsm+i)->state_no != -1; i++) {
        state = (fsm+i)->state_no;
        if (state != laststate) {
            if (laststate != -1) {
                (trans_array+laststate)->size = size;
            }
            (trans_array+state)->transitions = arrptr;
            size = 0;
        }
        laststate = state;

        if ((fsm+i)->target == -1)
            continue;
        inout = symbol_pair_to_single_symbol((fsm+i)->in, (fsm+i)->out);
        if (inout == epsilon_symbol)
            continue;
        
        arrptr->inout = inout;
        arrptr->target = (fsm+i)->target;
        arrptr++;
        size++;
    }

    if (laststate != -1) {
        (trans_array+laststate)->size = size;
    }

    for (i=0; i < net->statecount; i++) {
        arrptr = (trans_array+i)->transitions;
        size = (trans_array+i)->size;
        if (size > 1) {
            qsort(arrptr, size, sizeof(struct trans_list), trans_sort_cmp);
            lastsym = -1;
            /* Figure out if we're already deterministic */
            for (j=0; j < size; j++) {
                if ((arrptr+j)->inout == lastsym)
                    deterministic = 0;
                lastsym = (arrptr+j)->inout;
            }
        }
    }
}

INLINE static int e_closure(int states) {

    int i, set_size;
    struct e_closure_memo *ptr;

    /* e_closure extends the list of states which are reachable */
    /* and appends these to e_table                             */

    if (epsilon_symbol == -1)
        return(set_lookup(temp_move, states));

    if (states == 0)
        return -1;

    mainloop--;
    
    set_size = states;

    for (i = 0; i < states; i++) {

        /* State number we want to do e-closure on */
        ptr = e_closure_memo + *(temp_move+i);
        if (ptr->target == NULL)
            continue;
        ptr_stack_push(ptr);

        while (!(ptr_stack_isempty())) {
            ptr = ptr_stack_pop();
            /* Don't follow if already seen */
            if (*(marktable+ptr->state) == mainloop)
                continue;
            
            ptr->mark = mainloop;
            *(marktable+ptr->state) = mainloop;
            /* Add to tail of list */
            if (*(e_table+(ptr->state)) != mainloop) {
                *(temp_move+set_size) = ptr->state;
                *(e_table+(ptr->state)) = mainloop;
                set_size++;
            }
            
            if (ptr->target == NULL)
                continue;
            /* Traverse chain */

            for (; ptr != NULL ; ptr = ptr->next) {
                if (ptr->target->mark != mainloop) {
                    /* Push */
                    ptr->target->mark = mainloop;
                    ptr_stack_push(ptr->target);
                }
            }
        }
    }

    mainloop++;
    return(set_lookup(temp_move, set_size));
}

INLINE static int set_lookup (int *lookup_table, int size) {

  /* Look up a set and its corresponding state number */
  /* if it doesn't exist from before, assign a state number */
  
    return(nhash_find_insert(lookup_table, size));
  
}

void add_T_ptr(int setnum, int setsize, unsigned int theset, int fs) {

  int i;
  if (setnum >= T_limit) {
    T_limit *= 2;
    T_ptr = xxrealloc(T_ptr, sizeof(struct T_memo)*T_limit);
    for (i=setnum; i < T_limit; i++) {
        (T_ptr+i)->size = 0;
    }
  }
  
  (T_ptr + setnum)->size = setsize;
  (T_ptr + setnum)->set_offset = theset;
  (T_ptr + setnum)->finalstart = fs;
  int_stack_push(setnum);

}

static int initial_e_closure(struct fsm *net) {

    struct fsm_state *fsm;
    int i,j;

    finals = xxcalloc(num_states, sizeof(_Bool));

    num_start_states = 0;
    fsm = net->states;
    
    /* Create lookups for each state */
    for (i=0,j=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->final_state) {
            finals[(fsm+i)->state_no] = 1;
        }
        /* Add the start states as the initial set */
        if ((op == SUBSET_TEST_STAR_FREE) || ((fsm+i)->start_state)) {
            if (*(e_table+((fsm+i)->state_no)) != mainloop) {
                num_start_states++;
                numss = (fsm+i)->state_no;
                *(e_table+((fsm+i)->state_no)) = mainloop;
                *(temp_move+j) = (fsm+i)->state_no;
                j++;
            }
        }
    }
    mainloop++;
    /* Memoize e-closure(u) */
    if (epsilon_symbol != -1) {
        memoize_e_closure(fsm);
    }
    return(e_closure(j));
}
 
static void memoize_e_closure(struct fsm_state *fsm) {
    
    int i, state, laststate, *redcheck;
    struct e_closure_memo *ptr;
    
    e_closure_memo = xxcalloc(num_states,sizeof(struct e_closure_memo));
    marktable = xxcalloc(num_states,sizeof(int));
    /* Table for avoiding redundant epsilon arcs in closure */
    redcheck = xxmalloc(num_states*sizeof(int));

    for (i=0; i < num_states; i++) {
        ptr = e_closure_memo+i;
        ptr->state = i;
        ptr->target = NULL;
        *(redcheck+i) = -1;
    }

    laststate = -1;

    for (i=0; ;i++) {
        
        state = (fsm+i)->state_no;
        
        if (state != laststate) {
            if (!int_stack_isempty()) {                
                deterministic = 0;
                ptr = e_closure_memo+laststate;
                ptr->target = e_closure_memo+int_stack_pop();
                while (!int_stack_isempty()) {
                    ptr->next = xxmalloc(sizeof(struct e_closure_memo));
                    ptr->next->state = laststate;
                    ptr->next->target = e_closure_memo+int_stack_pop();
                    ptr->next->next = NULL;
                    ptr = ptr->next;
                }
            }
        }
        if (state == -1) {
            break;
        }
        if ((fsm+i)->target == -1) {
            continue;
        }
        /* Check if we have a redundant epsilon arc */
        if ((fsm+i)->in == EPSILON && (fsm+i)->out == EPSILON) {
            if (*(redcheck+((fsm+i)->target)) != (fsm+i)->state_no) {
                if ((fsm+i)->target != (fsm+i)->state_no) {
                    int_stack_push((fsm+i)->target);
                    *(redcheck+((fsm+i)->target)) = (fsm+i)->state_no;
                }
            }
            laststate = state;
        }
    }
    xxfree(redcheck);
}
 
static int next_unmarked(void) {
    if ((int_stack_isempty()))
        return -1;
    return(int_stack_pop());

    if ((T_limit <= T_last_unmarked + 1) || (T_ptr+T_last_unmarked+1)->size == 0) {
        return -1;
    } else {
        T_last_unmarked++;
        return(T_last_unmarked);
    }
}

static void single_symbol_to_symbol_pair(int symbol, int *symbol_in, int *symbol_out) {

  *symbol_in = *(single_sigma_array+(symbol*2));
  *symbol_out = *(single_sigma_array+(symbol*2+1));
  
}

static int symbol_pair_to_single_symbol(int in, int out) {
  return(*(double_sigma_array+maxsigma*in+out));
}

static void sigma_to_pairs(struct fsm *net) {
  
  int i, j, x, y, z, next_x = 0;
  struct fsm_state *fsm;

  fsm = net->states;

  epsilon_symbol = -1; 
  maxsigma = sigma_max(net->sigma);
  maxsigma++;

  single_sigma_array = xxmalloc(2*maxsigma*maxsigma*sizeof(int));
  double_sigma_array = xxmalloc(maxsigma*maxsigma*sizeof(int));
  
  for (i=0; i < maxsigma; i++) {
    for (j=0; j< maxsigma; j++) {
      *(double_sigma_array+maxsigma*i+j) = -1;
    }
  }
  
  /* f(x) -> y,z sigma pair */
  /* f(y,z) -> x simple entry */
  /* if exists f(n) <-> EPSILON, EPSILON, save n */
  /* symbol(x) x>=1 */

  /* Forward mapping: */
  /* *(double_sigma_array+maxsigma*in+out) */

  /* Backmapping: */
  /* *(single_sigma_array+(symbol*2) = in(symbol) */
  /* *(single_sigma_array+(symbol*2+1) = out(symbol) */

  /* Table for checking whether a state is final */

  x = 0;
  net->arity = 1;
  for (i=0; (fsm+i)->state_no != -1; i++) {
    y = (fsm+i)->in;
    z = (fsm+i)->out;
    if ((y == -1) || (z == -1))
      continue;
    if (y != z || y == UNKNOWN || z == UNKNOWN)
        net->arity = 2;
    if (*(double_sigma_array+maxsigma*y+z) == -1) {
      *(double_sigma_array+maxsigma*y+z) = x;
      *(single_sigma_array+next_x) = y;
      next_x++;
      *(single_sigma_array+next_x) = z;
      next_x++;
      if (y == EPSILON && z == EPSILON) {
	epsilon_symbol = x;
      }
      x++;
    }
  }
  num_symbols = x;
}


/* Functions for hashing n integers */
/* with permutations hashing to the same value */
/* necessary for subset construction */

static int nhash_find_insert(int *set, int setsize) {
    int j, found, *currlist;
    struct nhash_list *tableptr;
    unsigned int hashval;
    
    hashval = hashf(set, setsize);
    if ((table+hashval)->size == 0) {
        return(nhash_insert(hashval, set, setsize));
    } else {
        for (tableptr=(table+hashval); tableptr != NULL; tableptr = tableptr->next) {
            if ((tableptr)->size != setsize) {
                continue;
            } else {
                /* Compare the list at hashval position */
                /* to the current set by looking at etable */
                /* entries */
                for (j=0, found = 1, currlist= set_table+tableptr->set_offset; j < setsize; j++) {
                    if (*(e_table+(*(currlist+j))) != (mainloop-1)) {
                        found = 0;
                        break;
                    }
                }
                if (op == SUBSET_TEST_STAR_FREE && found == 1) {
                    for (j=0, currlist= set_table+tableptr->set_offset; j < setsize; j++) {
                        if (*(set+j) != *(currlist+j)) {
                            /* Set mark */
                            star_free_mark = 1;
                        }
                    }
                }
                if (found == 1) {
                    return(tableptr->setnum);
                }
            }
        }
        
        if (nhash_load / NHASH_LOAD_LIMIT > nhash_tablesize) {
            nhash_rebuild_table();
            hashval = hashf(set, setsize);
        }
        return(nhash_insert(hashval, set, setsize));
    }
}

INLINE static int hashf(int *set, int setsize) {
  int i;
  unsigned int hashval, sum = 0;
  hashval = 6703271;
  for (i = 0; i < setsize; i++) {
      hashval = (unsigned int) (*(set+i) + 1103 * setsize) * hashval; 
      sum += *(set+i) + i;
  }
  hashval = hashval + sum * 31;
  hashval = (hashval % nhash_tablesize);
  return hashval;
}

static unsigned int move_set(int *set, int setsize) {
    unsigned int old_offset;
    if (set_table_offset + setsize >= set_table_size) {
        while (set_table_offset + setsize >= set_table_size) {
            set_table_size *= 2;
        }
        set_table = xxrealloc(set_table, set_table_size * sizeof(int));
    }
    memcpy(set_table+set_table_offset, set, setsize * sizeof(int));
    old_offset = set_table_offset;
    set_table_offset += setsize;
    return(old_offset);
}

static int nhash_insert(int hashval, int *set, int setsize) { 
  struct nhash_list *tableptr;  
  int i, fs = 0;

  current_setnum++;
  tableptr = table+hashval;

  nhash_load++;
  for (i = 0; i < setsize; i++) {
      if (finals[*(set+i)])
          fs = 1;
  }
  if (tableptr->size == 0) {
    
      tableptr->set_offset = move_set(set, setsize);
      tableptr->size = setsize;
      tableptr->setnum = current_setnum;
      
      add_T_ptr(current_setnum, setsize, tableptr->set_offset, fs);
      return(current_setnum);
  }
  
  tableptr = xxmalloc(sizeof(struct nhash_list));
  tableptr->next = (table+hashval)->next;
  (table+hashval)->next = tableptr;
  tableptr->setnum = current_setnum;
  tableptr->size = setsize;
  tableptr->set_offset = move_set(set, setsize);
  
  add_T_ptr(current_setnum, setsize, tableptr->set_offset, fs);
  return(current_setnum);
}

static void nhash_rebuild_table () {
    int i, oldsize;
    struct nhash_list *oldtable, *tableptr, *ntableptr, *newptr;
    unsigned int hashval;
    
    oldtable = table;
    oldsize = nhash_tablesize;

    nhash_load = 0;
    for (i=0; primes[i] < nhash_tablesize; i++) { }
    nhash_tablesize = primes[(i+1)];
    
    table = xxcalloc(nhash_tablesize,sizeof(struct nhash_list));
    for (i=0; i < oldsize;i++) {
        if ((oldtable+i)->size == 0) {
            continue;
        }
        tableptr = oldtable+i;
        for ( ; tableptr != NULL; (tableptr = tableptr->next)) {
            /* rehash */
            hashval = hashf(set_table+tableptr->set_offset,tableptr->size);
            ntableptr = table+hashval;
            if (ntableptr->size == 0) {
                nhash_load++;
                ntableptr->size = tableptr->size;
                ntableptr->set_offset = tableptr->set_offset;
                ntableptr->setnum = tableptr->setnum;
                ntableptr->next = NULL;
            } else {
                newptr = xxmalloc(sizeof(struct nhash_list));
                newptr->next = ntableptr->next;
                ntableptr->next = newptr;
                newptr->setnum = tableptr->setnum;
                newptr->size = tableptr->size;
                newptr->set_offset = tableptr->set_offset;
            }
        }
    }
    nhash_free(oldtable, oldsize);
}

static void nhash_init (int initial_size) {

  int i;

  for (i=0; primes[i] < initial_size; i++) { }
  nhash_load = 0;
  nhash_tablesize = primes[i];
  table = xxcalloc(nhash_tablesize , sizeof(struct nhash_list));
  current_setnum = -1;
}
static void e_closure_free() {
    int i;
    struct e_closure_memo *eptr, *eprev;
    xxfree(marktable);
    for (i=0;i < num_states; i++) {
        eptr = (e_closure_memo+i)->next;
        for (eprev = NULL; eptr != NULL; ) {
            eprev = eptr;
            eptr = eptr->next;
            xxfree(eprev);
        }
        
    }
    xxfree(e_closure_memo);
}

static void nhash_free(struct nhash_list *nptr, int size) {
    struct nhash_list *nptr2, *nnext;
    int i;
    for (i=0; i < size; i++) {
        for (nptr2 = (nptr+i)->next; nptr2 != NULL; nptr2 = nnext) {
            nnext = nptr2->next;
            xxfree(nptr2);
        }
    }
    xxfree(nptr);
}
