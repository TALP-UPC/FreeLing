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
#include "foma.h"

struct fsm *fsm_lower(struct fsm *net) {
    struct fsm_state *fsm;
    int i, prevstate, out;
    fsm = net->states;
    fsm_state_init(sigma_max(net->sigma));
    prevstate = -1;
    for (i = 0; (fsm+i)->state_no != - 1; prevstate = (fsm+i)->state_no, i++) {
        if (prevstate != -1 && prevstate != (fsm+i)->state_no) {
            fsm_state_end_state();
        }
        if (prevstate != (fsm+i)->state_no) {
            fsm_state_set_current_state((fsm+i)->state_no, (fsm+i)->final_state, (fsm+i)->start_state);
        }
        if ((fsm+i)->target != -1) {
            out = ((fsm+i)->out == UNKNOWN) ? IDENTITY : (fsm+i)->out;
            fsm_state_add_arc((fsm+i)->state_no, out, out, (fsm+i)->target, (fsm+i)->final_state, (fsm+i)->start_state);
        }
    }
    fsm_state_end_state();
    xxfree(net->states);
    fsm_state_close(net);
    sigma_cleanup(net,0);
    return(net);
}

struct fsm *fsm_upper(struct fsm *net) {
    struct fsm_state *fsm;
    int i, prevstate, in;
    fsm = net->states;
    fsm_state_init(sigma_max(net->sigma));
    prevstate = -1;
    for (i = 0; (fsm+i)->state_no != - 1; prevstate = (fsm+i)->state_no, i++) {
        if (prevstate != -1 && prevstate != (fsm+i)->state_no) {
            fsm_state_end_state();
        }
        if (prevstate != (fsm+i)->state_no) {
            fsm_state_set_current_state((fsm+i)->state_no, (fsm+i)->final_state, (fsm+i)->start_state);
        }
        if ((fsm+i)->target != -1) {
            in = ((fsm+i)->in == UNKNOWN) ? IDENTITY : (fsm+i)->in;
            fsm_state_add_arc((fsm+i)->state_no, in, in, (fsm+i)->target, (fsm+i)->final_state, (fsm+i)->start_state);
        }
    }
    fsm_state_end_state();
    xxfree(net->states);
    fsm_state_close(net);
    fsm_update_flags(net,NO,NO,NO,UNK,UNK,UNK);
    sigma_cleanup(net,0);
    return(net);
}
