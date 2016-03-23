#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "foma.h"

/* Find a defined symbol from the symbol table */
/* Return the corresponding FSM                */
struct fsm *find_defined(struct defined_networks *def, char *string) {
    struct defined_networks *d;
    for (d = def; d != NULL; d = d->next) {
	if (d->name != NULL && strcmp(string, d->name) == 0) {
	    return(d->net);
	}
    }
    return NULL;
}

struct defined_networks *defined_networks_init(void) {
    struct defined_networks *def;
    def = calloc(1, sizeof(struct defined_networks)); /* Dummy first entry, so we can maintain the ptr */
    return def;
}

struct defined_functions *defined_functions_init(void) {
    struct defined_functions *deff;
    deff = calloc(1, sizeof(struct defined_functions)); /* Dummy first entry */
    return deff;
}

/* Removes a defined network from the list                 */
/* Returns 0 on success, 1 if the definition did not exist */
/* Undefines all if NULL is passed as the string argument  */

int remove_defined(struct defined_networks *def, char *string) {
    struct defined_networks *d, *d_prev, *d_next;
    int exists = 0;
    /* Undefine all */
    if (string == NULL) {
	for (d = def; d != NULL; d = d_next) {
	    d_next = d->next;	    
	    fsm_destroy(d->net);
	    xxfree(d->name);
	}
	return 0;
    }
    d_prev = NULL; 
    for (d = def; d != NULL; d_prev = d, d = d->next) {
	if (d->name != NULL && strcmp(d->name, string) == 0) {
	    exists = 1;
	    break;
	}
    }
    if (exists == 0) {
	return 1;
    }
    if (d == def) {
	if (d->next != NULL) {
	    fsm_destroy(d->net);
	    xxfree(d->name);
	    d->name = d->next->name;
	    d->net = d->next->net;
	    d_next = d->next->next;
	    xxfree(d->next);
	    d->next = d_next;
	} else {
	    fsm_destroy(d->net);
	    xxfree(d->name);
	    d->next = NULL;
	    d->name = NULL;
	}
    } else {
	fsm_destroy(d->net);
	xxfree(d->name);
	d_prev->next = d->next;
	xxfree(d);
    }
    return 0;
}

/* Finds defined regex "function" based on name, numargs */
/* Returns the corresponding regex                       */
char *find_defined_function(struct defined_functions *deff, char *name, int numargs) {
    struct defined_functions *d;
    for (d = deff ; d != NULL; d = d->next) {
        if (d->name != NULL && strcmp(d->name, name) == 0 && d->numargs == numargs) {
            return(d->regex);
        }
    }
    return NULL;
}

/* Add a function to list of defined functions */
int add_defined_function(struct defined_functions *deff, char *name, char *regex, int numargs) {
    struct defined_functions *d;
    for (d = deff; d != NULL; d = d->next) {
	if (d->name != NULL && strcmp(d->name, name) == 0 && d->numargs == numargs) {
	    xxfree(d->regex);
	    d->regex = xxstrdup(regex);
	    printf("redefined %s@%i)\n", name, numargs);
	    return 1;
	}
    }
    if (deff->name == NULL) {
	d = deff;
    } else {
	d = xxmalloc(sizeof(struct defined_functions));
	d->next = deff->next;
	deff->next = d;
    }
    d->name = xxstrdup(name);
    d->regex = xxstrdup(regex);
    d->numargs = numargs;
    return 0;
}

/* Add a network to list of defined networks */
/* Returns 0 on success or 1 on redefinition */
/* Always maintain head of list at same ptr */

int add_defined(struct defined_networks *def, struct fsm *net, char *string) {
    struct defined_networks *d;
    if (net == NULL)
	return 0;
    fsm_count(net);
    for (d = def; d != NULL; d = d->next) {
	if (d->name != NULL && strcmp(d->name, string) == 0) {
	    xxfree(d->net);
	    xxfree(d->name);
	    d->net = net;
	    d->name = xxstrdup(string);
	    return 1;
	}
    }
    if (def->name == NULL) {
	d = def;
    } else {
	d = xxmalloc(sizeof(struct defined_networks));
	d->next = def->next;
	def->next = d;
    }
    d->name = xxstrdup(string);
    d->net = net;
    return 0;
}
