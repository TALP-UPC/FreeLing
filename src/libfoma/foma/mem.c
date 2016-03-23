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

#include "foma.h"
#include <stdlib.h>
#include <string.h>

/* Global variables */
int g_show_flags = 0;
int g_obey_flags = 1;
int g_flag_is_epsilon = 0;
int g_print_space = 0;
int g_print_pairs = 0;
int g_minimal = 1;
int g_name_nets = 0;
int g_print_sigma = 1;
int g_quit_on_fail = 1;
int g_quote_special = 0;
int g_recursive_define = 0;
int g_sort_arcs = 1;
int g_verbose = 1;
int g_minimize_hopcroft = 1;
int g_compose_tristate = 0;
int g_list_limit = 100;
int g_list_random_limit = 15;
int g_med_limit  = 3;
int g_med_cutoff = 15;
int g_lexc_align = 0;
char *g_att_epsilon = "@0@";

char *xxstrndup(const char *s, size_t n) {
    char *r = NULL;
    const char *p = s;
    while(*p++ && n--);
    n = p - s - 1;
    r = (char *) xxmalloc(n + 1);
    if(r != NULL) {
        memcpy(r, s, n);
        r[n] = 0;
    }
    return r;
}

int next_power_of_two(int v) {
    int i;
    for (i=0; v > 0; i++)
        v = v >> 1;
    return (1 << i);
}

unsigned int round_up_to_power_of_two(unsigned int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return(v);
}

INLINE void *xxmalloc(size_t size) {
    return(malloc(size));
}

INLINE void xxfree(void *ptr) {
    free(ptr);
}

void *xxrealloc(void *ptr, size_t size) {
    return(realloc(ptr, size));
}

INLINE void *xxcalloc(size_t nmemb, size_t size) {
    return(calloc(nmemb,size));
}

INLINE char *xxstrdup(const char *s) {
    return(strdup(s));
}

