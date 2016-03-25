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

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include "foma.h"

static struct fsm *fsm_minimize_brz(struct fsm *net);
static struct fsm *fsm_minimize_hop(struct fsm *net);
static struct fsm *rebuild_machine(struct fsm *net);

static int *single_sigma_array, *double_sigma_array, *memo_table, *temp_move, *temp_group, maxsigma, epsilon_symbol, num_states, num_symbols, num_finals, mainloop, total_states;

static _Bool *finals;

struct statesym {
    int target;
    unsigned short int symbol;
    struct state_list *states;
    struct statesym *next;
};

struct state_list {
    int state;
    struct state_list *next;
};

struct p {
    struct e *first_e;
    struct e *last_e;
    struct p *current_split;
    struct p *next;
    struct agenda *agenda;
    int count;
    int t_count;
    int inv_count;
    int inv_t_count;
};

struct e {
  struct p *group;
  struct e *left;
  struct e *right;
  int inv_count;
};

struct agenda {
  struct p *p;
  struct agenda *next;
  _Bool index;
};

struct trans_list {
    int inout;
    int source;
} *trans_list;

struct trans_array {
    struct trans_list *transitions;
    unsigned int size;
    unsigned int tail;
} *trans_array;



static struct p *P, *Phead, *Pnext, *current_w;
static struct e *E;
static struct agenda *Agenda_head, *Agenda_top, *Agenda_next, *Agenda;

static INLINE int refine_states(int sym);
static void init_PE();
static void agenda_add(struct p *pptr, int start);
static void sigma_to_pairs(struct fsm *net);
/* static void single_symbol_to_symbol_pair(int symbol, int *symbol_in, int *symbol_out); */
static INLINE int symbol_pair_to_single_symbol(int in, int out);
static void generate_inverse(struct fsm *net);

struct fsm *fsm_minimize(struct fsm *net) {
    extern int g_minimal;
    extern int g_minimize_hopcroft;

    if (net == NULL) { return NULL; }
    /* The network needs to be deterministic and trim before we minimize */
    if (net->is_deterministic != YES)
        net = fsm_determinize(net);
    if (net->is_pruned != YES)
        net = fsm_coaccessible(net);
    if (net->is_minimized != YES && g_minimal == 1) {
        if (g_minimize_hopcroft != 0) {
            net = fsm_minimize_hop(net);
        }
        else 
            net = fsm_minimize_brz(net);        
        fsm_update_flags(net,YES,YES,YES,YES,UNK,UNK);
    }
    return(net);
}

static struct fsm *fsm_minimize_brz(struct fsm *net) {
    return(fsm_determinize(fsm_reverse(fsm_determinize(fsm_reverse(net)))));
}

static struct fsm *fsm_minimize_hop(struct fsm *net) {

    struct e *temp_E;
    struct trans_array *tptr;
    struct trans_list *transitions;
    int i,j,minsym,next_minsym,current_i, stateno, thissize, source;  
    unsigned int tail;

    fsm_count(net);
    if (net->finalcount == 0)  {
	fsm_destroy(net);
	return(fsm_empty_set());
    }

    num_states = net->statecount;
    
    P = NULL;

    /* 
       1. generate the inverse lookup table
       2. generate P and E (partitions, states linked list)
       3. Init Agenda = {Q, Q-F}
       4. Split until Agenda is empty
    */
    
    sigma_to_pairs(net);
    
    init_PE();

    if (total_states == num_states) {
        goto bail;
    }

    generate_inverse(net);


    Agenda_head->index = 0;
    if (Agenda_head->next != NULL)
        Agenda_head->next->index = 0;

    for (Agenda = Agenda_head; Agenda != NULL; ) {
        /* Remove current_w from agenda */
        current_w = Agenda->p;
        current_i = Agenda->index;
        Agenda->p->agenda = NULL;
        Agenda = Agenda->next;

        /* Store current group state number in tmp_group */
        /* And figure out minsym */
        /* If index is 0 we start splitting from the first symbol */
        /* Otherwise we split from where we left off last time */

        thissize = 0;
        minsym = INT_MAX;
        for (temp_E = current_w->first_e; temp_E != NULL; temp_E = temp_E->right) {
            stateno = temp_E - E;
            *(temp_group+thissize) = stateno;
            thissize++;
            tptr = trans_array+stateno;
            /* Clear tails if symloop should start from 0 */
            if (current_i == 0)
                tptr->tail = 0;
            
            tail = tptr->tail;
            transitions = (tptr->transitions)+tail;
            if (tail < tptr->size && transitions->inout < minsym) {
                minsym = transitions->inout;
            }
        }

        for (next_minsym = INT_MAX; minsym != INT_MAX ; minsym = next_minsym, next_minsym = INT_MAX) {

            /* Add states to temp_move */
            for (i = 0, j = 0; i < thissize; i++) {
                tptr = trans_array+*(temp_group+i);
                tail = tptr->tail;
                transitions = (tptr->transitions)+tail;
                while (tail < tptr->size && transitions->inout == minsym) {
                    source = transitions->source;
                    if (*(memo_table+(source)) != mainloop) {
                        *(memo_table+(source)) = mainloop;
                        *(temp_move+j) = source;
                        j++;
                    }
                    tail++;
                    transitions++;
                }
                tptr->tail = tail;
                if (tail < tptr->size && transitions->inout < next_minsym) {
                    next_minsym = transitions->inout;
                }
            }
            if (j == 0) {
                continue;
            }
            mainloop++;
            if (refine_states(j) == 1) {
                break; /* break loop if we split current_w */
            }
        }
        if (total_states == num_states) {
            break;
        }
    }

    net = rebuild_machine(net);

    xxfree(trans_array);
    xxfree(trans_list);

 bail:
    
    xxfree(Agenda_top);
    
    xxfree(memo_table);
    xxfree(temp_move);
    xxfree(temp_group);


    xxfree(finals);
    xxfree(E);
    xxfree(Phead);
    xxfree(single_sigma_array);
    xxfree(double_sigma_array);
    
    return(net);
}

static struct fsm *rebuild_machine(struct fsm *net) {
  int i,j, group_num, source, target, new_linecount = 0, arccount = 0;
  struct fsm_state *fsm;
  struct p *myp;
  struct e *thise;

  if (net->statecount == total_states) {
      return(net);
  }
  fsm = net->states;

  /* We need to make sure state 0 is first in its group */
  /* to get the proper numbering of states */

  if (E->group->first_e != E) {
    E->group->first_e = E;
  }

  /* Recycling t_count for group numbering use here */

  group_num = 1;
  myp = P;
  while (myp != NULL) {
    myp->count = 0;
    myp = myp->next;
  }

  for (i=0; (fsm+i)->state_no != -1; i++) {
    thise = E+((fsm+i)->state_no);
    if (thise->group->first_e == thise) {
      new_linecount++;
      if ((fsm+i)->start_state == 1) {
	thise->group->t_count = 0;
	thise->group->count = 1;
      } else if (thise->group->count == 0) {
	thise->group->t_count = group_num++;
	thise->group->count = 1;
      }
    }
  }

  for (i=0, j=0; (fsm+i)->state_no != -1; i++) {
    thise = E+((fsm+i)->state_no);
    if (thise->group->first_e == thise) {
      source = thise->group->t_count;
      target = ((fsm+i)->target == -1) ? -1 : (E+((fsm+i)->target))->group->t_count;
      add_fsm_arc(fsm, j, source, (fsm+i)->in, (fsm+i)->out, target, finals[(fsm+i)->state_no], (fsm+i)->start_state);
      arccount = ((fsm+i)->target == -1) ? arccount : arccount+1;
      j++;
    }
  }
  
  add_fsm_arc(fsm, j, -1, -1, -1, -1, -1, -1);
  fsm = xxrealloc(fsm,sizeof(struct fsm_state)*(new_linecount+1));
  net->states = fsm;
  net->linecount = j+1;
  net->arccount = arccount;
  net->statecount = total_states;
  return(net);
}

static INLINE int refine_states(int invstates) {
    int i, selfsplit;
    struct e *thise;
    struct p *tP, *newP = NULL;

  /* 
     1. add inverse(P,a) to table of inverses, disallowing duplicates
     2. first pass on S, touch each state once, increasing P->t_count
     3. for each P where counter != count, split and add to agenda
  */

  /* Inverse to table of inverses */
  selfsplit = 0;

  /* touch and increase P->counter */
  for (i=0; i < invstates; i++) {
    ((E+(*(temp_move+i)))->group)->t_count++;
    ((E+(*(temp_move+i)))->group)->inv_t_count += ((E+(*(temp_move+i)))->inv_count);
    assert((E+(*(temp_move+i)))->group->t_count <= (E+(*(temp_move+i)))->group->count);
  }

  /* Split (this is the tricky part) */
  
  for (i=0; i < invstates; i++) {
    
    thise = E+*(temp_move+i);
    tP = thise->group;
    
    /* Do we need to split?
       if we've touched as many states as there are in the partition
       we don't split */

    if (tP->t_count == tP->count) {
      tP->t_count = 0;
      tP->inv_t_count = 0;
      continue;
    }
    
    if ((tP->t_count != tP->count) && (tP->count > 1) && (tP->t_count > 0)) {      
        
        /* Check if we already split this */
        newP = tP->current_split;
        if (newP == NULL) {
            /* printf("tP [%i] newP [%i]\n",tP->inv_count,tP->inv_t_count); */
            /* Create new group newP */
            total_states++;
            if (total_states == num_states)
                return(1); /* Abort now, machine is already minimal */
            tP->current_split = Pnext++;
            newP = tP->current_split;
            newP->first_e = newP->last_e = thise;
            newP->count = 0;
            newP->inv_count = tP->inv_t_count;
            newP->inv_t_count = 0;
            newP->t_count = 0;
            newP->current_split = NULL;
            newP->agenda = NULL;

            /* Add to agenda */
            
            /* If the current block (tP) is on the agenda, we add both back */
            /* to the agenda */
            /* In practice we need only add newP since tP stays where it is */
            /* However, we mark the larger one as not starting the symloop */
            /* from zero */
            if (tP->agenda != NULL) {
                /* Is tP smaller */
                if (tP->inv_count < tP->inv_t_count) {
                    agenda_add(newP, 1);
                    tP->agenda->index = 0;
                }
                else {
                    agenda_add(newP, 0);
                }
                /* In the event that we're splitting the partition we're currently */
                /* splitting with, we can simply add both new partitions to the agenda */
                /* and break out of the entire sym loop after we're */
                /* done with the current sym and move on with the agenda */
                /* We process the larger one for all symbols */
                /* and the smaller one for only the ones remaining in this symloop */

            } else if (tP == current_w) {
                agenda_add(((tP->inv_count < tP->inv_t_count) ? tP : newP),0);
                agenda_add(((tP->inv_count >= tP->inv_t_count) ? tP : newP),1);
                selfsplit = 1;
            } else {
                /* If the block is not on the agenda, we add */
                /* the smaller of tP, newP and start the symloop from 0 */                
                agenda_add((tP->inv_count < tP->inv_t_count ? tP : newP),0);
            }
            /* Add to middle of P-chain */
            newP->next = P->next;
            P->next = newP;
        }
    
        thise->group = newP;
        newP->count++;
        
        /* need to make tP->last_e point to the last untouched e */
        if (thise == tP->last_e)
            tP->last_e = thise->left;
        if (thise == tP->first_e)
            tP->first_e = thise->right;
        
        /* Adjust links */
        if (thise->left != NULL)
            thise->left->right = thise->right;
        if (thise->right != NULL)
            thise->right->left = thise->left;
        
        if (newP->last_e != thise) {
            newP->last_e->right = thise;
            thise->left = newP->last_e;
            newP->last_e = thise;
        }
    
        thise->right = NULL;
        if (newP->first_e == thise)
            thise->left = NULL;
        
        /* Are we done for this block? Adjust counters */
        if (newP->count == tP->t_count) {
            tP->count = tP->count - newP->count;
            tP->inv_count = tP->inv_count - tP->inv_t_count;
            tP->current_split = NULL;
            tP->t_count = 0;
            tP->inv_t_count = 0;
        }
    }
  }
  /* We return 1 if we just split the partition we were working with */
  return (selfsplit);
}

static void agenda_add(struct p *pptr, int start) {

  /* Use FILO strategy here */

  struct agenda *ag;
  //ag = xxmalloc(sizeof(struct agenda));
  ag = Agenda_next++;
  if (Agenda != NULL)
    ag->next = Agenda;
  else
    ag->next = NULL;
  ag->p = pptr;
  ag->index = start;
  Agenda = ag;
  pptr->agenda = ag;
}

static void init_PE() {
  /* Create two members of P
     (nonfinals,finals)
     and put both of them on the agenda 
  */

  int i;
  struct e *last_f, *last_nonf;
  struct p *nonFP, *FP;
  struct agenda *ag;

  mainloop = 1;
  memo_table = xxcalloc(num_states,sizeof(int));
  temp_move = xxcalloc(num_states,sizeof(int));
  temp_group = xxcalloc(num_states,sizeof(int));
  Phead = P = Pnext = xxcalloc(num_states+1, sizeof(struct p));
  nonFP = Pnext++;
  FP = Pnext++;
  nonFP->next = FP;
  nonFP->count = num_states-num_finals;
  FP->next = NULL;
  FP->count = num_finals;
  FP->t_count = 0;
  nonFP->t_count = 0;
  FP->current_split = NULL;
  nonFP->current_split = NULL;
  FP->inv_count = nonFP->inv_count = FP->inv_t_count = nonFP->inv_t_count = 0;
  
  /* How many groups can we put on the agenda? */
  Agenda_top = Agenda_next = xxcalloc(num_states*2, sizeof(struct agenda));
  Agenda_head = NULL;

  P = NULL;
  total_states = 0;

  if (num_finals > 0) {
      ag = Agenda_next++;
      FP->agenda = ag;
      P = FP;
      P->next = NULL;
      ag->p = FP;
      Agenda_head = ag;
      ag->next = NULL;
      total_states++;
  }
  if (num_states - num_finals > 0) {
      ag = Agenda_next++;
      nonFP->agenda = ag;
      ag->p = nonFP;
      ag->next = NULL;
      total_states++;
      if (Agenda_head != NULL) {
          Agenda_head->next = ag;
          P->next = nonFP;
          P->next->next = NULL;
      } else {
          P = nonFP;
          P->next = NULL;
          Agenda_head = ag;
      }
  }
  
  /* Initialize doubly linked list E */
  E = xxcalloc(num_states,sizeof(struct e));

  last_f = NULL;
  last_nonf = NULL;
  
  for (i=0; i < num_states; i++) {
    if (finals[i]) {
      (E+i)->group = FP;
      (E+i)->left = last_f;
      if (i > 0 && last_f != NULL)
	last_f->right = (E+i);
      if (last_f == NULL)
	FP->first_e = (E+i);
      last_f = (E+i);
      FP->last_e = (E+i);
    } else {
      (E+i)->group = nonFP;
      (E+i)->left = last_nonf;
      if (i > 0 && last_nonf != NULL)
	last_nonf->right = (E+i);
      if (last_nonf == NULL)
	nonFP->first_e = (E+i);
      last_nonf = (E+i);
      nonFP->last_e = (E+i);
    }
    (E+i)->inv_count = 0;
  }

  if (last_f != NULL)
    last_f->right = NULL;
  if (last_nonf != NULL)
    last_nonf->right = NULL;
}

static int trans_sort_cmp(const void *a, const void *b) {
  return (((const struct trans_list *)a)->inout - ((const struct trans_list *)b)->inout);
}

static void generate_inverse(struct fsm *net) {
    
    struct fsm_state *fsm;
    struct trans_array *tptr;
    struct trans_list *listptr;

    int i, source, target, offsetcount, symbol, size;
    fsm = net->states;
    trans_array = xxcalloc(net->statecount, sizeof(struct trans_array));
    trans_list = xxcalloc(net->arccount, sizeof(struct trans_list));

    /* Figure out the number of transitions each one has */
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target == -1) {
            continue;
        }
        target = (fsm+i)->target;
        (E+target)->inv_count++;
        (E+target)->group->inv_count++;
        (trans_array+target)->size++;
    }
    offsetcount = 0;
    for (i=0; i < net->statecount; i++) {
        (trans_array+i)->transitions = trans_list + offsetcount;
        offsetcount += (trans_array+i)->size;
    }
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target == -1) {
            continue;
        }
        symbol = symbol_pair_to_single_symbol((fsm+i)->in,(fsm+i)->out);        
        source = (fsm+i)->state_no;
        target = (fsm+i)->target;
        tptr = trans_array + target;
        ((tptr->transitions)+(tptr->tail))->inout = symbol;
        ((tptr->transitions)+(tptr->tail))->source = source;
        tptr->tail++;
    }
    /* Sort arcs */
    for (i=0; i < net->statecount; i++) {
        listptr = (trans_array+i)->transitions;
        size = (trans_array+i)->size;
        if (size > 1)
            qsort(listptr, size, sizeof(struct trans_list), trans_sort_cmp);
    }
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

  finals = xxcalloc(num_states, sizeof(_Bool));
  x = 0; num_finals = 0;
  net->arity = 1;
  for (i=0; (fsm+i)->state_no != -1; i++) {
    if ((fsm+i)->final_state == 1 && finals[(fsm+i)->state_no] != 1) {
      num_finals++;
      finals[(fsm+i)->state_no] = 1;
    }
    y = (fsm+i)->in;
    z = (fsm+i)->out;
    if (y != z || y == UNKNOWN || z == UNKNOWN)
        net->arity = 2;
    if ((y == -1) || (z == -1))
      continue;
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

static INLINE int symbol_pair_to_single_symbol(int in, int out) {
  return(*(double_sigma_array+maxsigma*in+out));
}
