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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "foma.h"

struct invtable {
  int state;
  struct invtable *next;
};

struct fsm *fsm_coaccessible(struct fsm *net) {

    struct invtable *inverses, *temp_i, *temp_i_prev, *current_ptr;
  int i, j, s, t, *coacc, current_state, markcount, *mapping, terminate, new_linecount, new_arccount, *added, old_statecount;
  

  struct fsm_state *fsm;

  fsm = net->states;
  new_arccount = 0;
  /* printf("statecount %i\n",net->statecount); */
  old_statecount = net->statecount;
  inverses = xxcalloc(net->statecount, sizeof(struct invtable));
  coacc = xxmalloc(sizeof(int)*(net->statecount));
  mapping = xxmalloc(sizeof(int)*(net->statecount));
  added = xxmalloc(sizeof(int)*(net->statecount));

  for (i=0; i < (net->statecount); i++) {
    (inverses+i)->state = -1;
    *(coacc+i) = 0;
    *(added+i) = 0;
  }

  for (i=0; (fsm+i)->state_no != -1; i++) {
    s = (fsm+i)->state_no;
    t = (fsm+i)->target;
    if (t != -1 && s != t) {
      
      if (((inverses+t)->state) == -1) {
	(inverses+t)->state = s;
      } else {
        temp_i = xxmalloc(sizeof(struct invtable));
	temp_i->next = (inverses+t)->next;
	(inverses+t)->next = temp_i;
	temp_i->state = s;
      }
    }
  }

  /* Push & mark finals */

  markcount = 0;
  for (i=0; (fsm+i)->state_no != -1; i++) {
    if ((fsm+i)->final_state && (!*(coacc+((fsm+i)->state_no)))) {
      int_stack_push((fsm+i)->state_no);
      *(coacc+(fsm+i)->state_no) = 1;
      markcount++;
    }
  }

  terminate = 0;
  while(!int_stack_isempty()) {
    current_state = int_stack_pop();
    current_ptr = inverses+current_state;
    while(current_ptr != NULL && current_ptr->state != -1) {
      if (!*(coacc+(current_ptr->state))) {
	*(coacc+(current_ptr->state)) = 1;
	int_stack_push(current_ptr->state);
	markcount++;
      }
      current_ptr = current_ptr->next;
    }
    if (markcount >= net->statecount) {
      /* printf("Already coacc\n");  */
      terminate = 1;
      int_stack_clear();
      break;
    }
  }


  if (terminate == 0) {
    *mapping = 0; /* state 0 always exists */
    new_linecount = 0;
    for (i=1,j=0; i < (net->statecount);i++) {
      if (*(coacc+i) == 1) {
	j++;
	*(mapping+i) = j;
      }
    }
    
    for (i=0,j=0; (fsm+i)->state_no != -1; i++) {
      if (i > 0 && (fsm+i)->state_no != (fsm+i-1)->state_no && (fsm+i-1)->final_state && !*(added+((fsm+i-1)->state_no))) {
	add_fsm_arc(fsm, j++, *(mapping+((fsm+i-1)->state_no)), -1, -1, -1, 1, (fsm+i-1)->start_state);
	new_linecount++;
	*(added+((fsm+i-1)->state_no)) = 1;
	/* printf("addf ad %i\n",i); */
      }
      if (*(coacc+((fsm+i)->state_no)) && (((fsm+i)->target == -1) || *(coacc+((fsm+i)->target)))) {
	(fsm+j)->state_no = *(mapping+((fsm+i)->state_no));
	if ((fsm+i)->target == -1) {
	  (fsm+j)->target = -1;
	} else {
	  (fsm+j)->target = *(mapping+((fsm+i)->target));
	}
	(fsm+j)->final_state = (fsm+i)->final_state;
	(fsm+j)->start_state = (fsm+i)->start_state;
	(fsm+j)->in = (fsm+i)->in;
	(fsm+j)->out = (fsm+i)->out;
	j++;
	new_linecount++;
	*(added+(fsm+i)->state_no) = 1;
	if ((fsm+i)->target != -1) {
	  new_arccount++;
	}	
      }
    }

    if ((i > 1) && ((fsm+i-1)->final_state) && *(added+((fsm+i-1)->state_no)) == 0) {
      /* printf("addf\n"); */
      add_fsm_arc(fsm, j++, *(mapping+((fsm+i-1)->state_no)), -1, -1, -1, 1, (fsm+i-1)->start_state);
      new_linecount++;
    }

    if (new_linecount == 0) {
      add_fsm_arc(fsm, j++, 0, -1, -1, -1, -1, -1);
    }
  
    add_fsm_arc(fsm, j, -1, -1, -1, -1, -1, -1);
    if (markcount == 0) {
      /* We're dealing with the empty language */
      xxfree(fsm);
      net->states = fsm_empty();
      fsm_sigma_destroy(net->sigma);
      net->sigma = sigma_create();
    }
    net->linecount = new_linecount;
    net->arccount = new_arccount;
    net->statecount = markcount;
  }

  /* printf("Markccount %i \n",markcount); */
  
  for (i = 0; i < old_statecount ; i++) {
      for (temp_i = inverses+i; temp_i != NULL ; ) {
          temp_i_prev = temp_i;
          temp_i = temp_i->next;
          if (temp_i_prev != inverses+i)
              xxfree(temp_i_prev);
      }
  }
  xxfree(inverses);

  xxfree(coacc);
  xxfree(added);
  xxfree(mapping);
  net->is_pruned = YES;
  return(net);
}
