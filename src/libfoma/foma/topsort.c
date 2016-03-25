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
#include "foma.h"

struct fsm *fsm_topsort (struct fsm *net) {

    /* We topologically sort the network by looking for a state          */
    /* with inverse count 0. We then examine all the arcs from that      */
    /* state, and decrease the target invcounts. If we find a new        */
    /* state with invcount 0, we push that on the stack to be treated    */
    /* If the graph is cyclic, one of two things will happen:            */

    /* (1) We fail to find a state with invcount 0 before we've treated  */
    /*     all states                                                    */
    /* (2) A state under treatment has an arc to a state already treated */
    /*     or itself (we mark a state as treated as soon as we start     */
    /*     working on it).                                               */
    /* Of course we also count the number of paths in the network.       */

    int i, j, curr_state, *statemap, treatcount, *order, lc, *newnum, newtarget, newstate;
    unsigned short int *invcount;
    unsigned char *treated, overflow;
    long long grand_pathcount, *pathcount;
    struct fsm_state *fsm, *curr_fsm, *new_fsm;

    fsm_count(net);

    fsm = net->states;
    
    statemap = xxmalloc(sizeof(int)*net->statecount);
    order = xxmalloc(sizeof(int)*net->statecount);
    pathcount = xxmalloc(sizeof(long long)*net->statecount);
    newnum = xxmalloc(sizeof(int)*net->statecount);
    invcount = xxmalloc(sizeof(unsigned short int)*net->statecount);
    treated =  xxmalloc(sizeof(unsigned char)*net->statecount);
   
    for (i=0; i < net->statecount; i++) {
	*(statemap+i) = -1;
	*(invcount+i) = 0;
	*(treated+i) = 0;
	*(order+i) = 0;
	*(pathcount+i) = 0;
    }

    for (i=0, lc=0; (fsm+i)->state_no != -1; i++) {        
        lc++;
        if ((fsm+i)->target != -1) {
            (*(invcount+(fsm+i)->target))++;
            /* Do a fast check here to see if we have a selfloop */
            if ((fsm+i)->state_no == (fsm+i)->target) {
                net->pathcount = PATHCOUNT_CYCLIC;
                net->is_loop_free = 0;
                goto cyclic;
            }
        }
	if (*(statemap+(fsm+i)->state_no) == -1) {
	    *(statemap+(fsm+i)->state_no) = i;
	}
    }

    treatcount = net->statecount;
    int_stack_clear();
    int_stack_push(0);
    grand_pathcount = 0;
    
    *(pathcount+0) = 1;

    overflow = 0;
    for (i=0 ; !int_stack_isempty(); i++) {
        /* Treat a state */
        curr_state = int_stack_pop();
        *(treated+curr_state) = 1;
        *(order+i) = curr_state;
        *(newnum+curr_state) = i;

        treatcount--;
        curr_fsm = fsm+*(statemap+curr_state);
        while (curr_fsm->state_no == curr_state) {
            if (curr_fsm->target != -1 ) {
                (*(invcount+(curr_fsm->target)))--;
                
                /* Check if we overflow the path counter */

                if (!overflow) {
                    *(pathcount+(curr_fsm->target)) += *(pathcount+curr_state);
                    if ((*(pathcount+(curr_fsm->target)) < 0)) {
                        overflow = 1;
                    }
                }
                
                /* Case (1) for cyclic */
                if (*(treated+(curr_fsm)->target) == 1) {
                    net->pathcount = PATHCOUNT_CYCLIC;
                    net->is_loop_free = 0;
                    goto cyclic;
                }
                if ( *(invcount+(curr_fsm->target)) == 0) {
                    int_stack_push(curr_fsm->target);
                }
            }
            curr_fsm++;
        }       
    }

    /* Case (2) */
    if (treatcount > 0) {
        net->pathcount = PATHCOUNT_CYCLIC;
        net->is_loop_free = 0;
        goto cyclic;
    }

    new_fsm = xxmalloc(sizeof(struct fsm_state) * (lc+1));
    for (i=0, j=0 ; i < net->statecount; i++) {

        curr_state = *(order+i);
        curr_fsm = fsm+*(statemap+curr_state);
        
        if (curr_fsm->final_state == 1 && !overflow) {
            grand_pathcount += *(pathcount + curr_state);
            if (grand_pathcount < 0)
                overflow = 1;           
        }
            
        for (; curr_fsm->state_no == curr_state; curr_fsm++) {
                        
            newstate = curr_fsm->state_no  == -1 ? -1 : *(newnum+(curr_fsm->state_no));
            newtarget = curr_fsm->target == -1 ? -1 : *(newnum+(curr_fsm->target));
            add_fsm_arc(new_fsm, j, newstate, curr_fsm->in, curr_fsm->out, newtarget, curr_fsm->final_state, curr_fsm->start_state);
            j++;
        }
    }
    
    add_fsm_arc(new_fsm, j, -1, -1, -1, -1, -1, -1);
    net->states = new_fsm;
    net->pathcount = grand_pathcount;
    net->is_loop_free = 1;
    if (overflow == 1) {
        net->pathcount = PATHCOUNT_OVERFLOW;
    }
    xxfree(fsm);

 cyclic:

    xxfree(statemap);
    xxfree(order);
    xxfree(pathcount);
    xxfree(newnum);
    xxfree(invcount);
    xxfree(treated);
    int_stack_clear();
    return(net);
}
