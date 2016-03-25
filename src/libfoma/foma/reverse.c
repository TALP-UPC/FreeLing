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

struct fsm *fsm_reverse(struct fsm *net) {
    struct fsm *revnet;
    struct fsm_construct_handle *revh;
    struct fsm_read_handle *inh;
    int i;

    inh = fsm_read_init(net);
    revh = fsm_construct_init(net->name);
    fsm_construct_copy_sigma(revh, net->sigma);

    while (fsm_get_next_arc(inh)) {
	fsm_construct_add_arc_nums(revh, fsm_get_arc_target(inh)+1, fsm_get_arc_source(inh)+1, fsm_get_arc_num_in(inh), fsm_get_arc_num_out(inh));
    }

    while ((i = fsm_get_next_final(inh)) != -1) {
	fsm_construct_add_arc_nums(revh, 0, i+1, EPSILON, EPSILON);
    }
    while ((i = fsm_get_next_initial(inh)) != -1) {
	fsm_construct_set_final(revh, i+1);
    }
    fsm_construct_set_initial(revh, 0);
    fsm_read_done(inh);
    revnet = fsm_construct_done(revh);
    revnet->is_deterministic = 0;
    revnet->is_epsilon_free = 0;
    fsm_destroy(net);
    return(revnet);
}
