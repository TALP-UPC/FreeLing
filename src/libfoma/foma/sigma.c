/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2012 Mans Hulden                                     */

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

#include <string.h>
#include <stdlib.h>
#include "foma.h"

struct sigma *sigma_remove(char *symbol, struct sigma *sigma) {
  struct sigma *sigma_start, *sigma_prev = NULL;
  sigma_prev = NULL;
  sigma_start = sigma;
  for ( ; sigma != NULL && sigma->number != -1; sigma_prev = sigma, sigma=sigma->next) {
    if (strcmp(sigma->symbol,symbol) == 0) {
      if (sigma_prev == NULL) {
	sigma_start = sigma->next;
	xxfree(sigma->symbol);
	xxfree(sigma);
      } else {
	(sigma_prev)->next = sigma->next;
	xxfree(sigma->symbol);
	xxfree(sigma);
      }
      break;
    }
  }
  return(sigma_start);
}

struct sigma *sigma_remove_num(int num, struct sigma *sigma) {
  struct sigma *sigma_start, *sigma_prev = NULL;
  sigma_prev = NULL;
  sigma_start = sigma;
  for ( ; sigma != NULL && sigma->number != -1; sigma_prev = sigma, sigma=sigma->next) {
    if (sigma->number == num) {
      if (sigma_prev == NULL) {
	sigma_start = sigma->next;
	xxfree(sigma->symbol);
	xxfree(sigma);
      } else {
	(sigma_prev)->next = sigma->next;
	xxfree(sigma->symbol);
	xxfree(sigma);
      }
      break;
    }
  }
  return(sigma_start);
}

int sigma_add_special (int symbol, struct sigma *sigma) {
    struct sigma *sigma_previous = NULL, *sigma_splice = NULL;
    char *str = NULL;
    if (symbol == EPSILON)
        str = xxstrdup("@_EPSILON_SYMBOL_@");
    if (symbol == IDENTITY)
        str = xxstrdup("@_IDENTITY_SYMBOL_@");
    if (symbol == UNKNOWN)
        str = xxstrdup("@_UNKNOWN_SYMBOL_@");

    /* Insert special symbols pre-sorted */
    if (sigma->number == -1) {
      sigma->number = symbol;
    } else {
        for (;(sigma != NULL) && (sigma->number < symbol) && (sigma->number!=-1); sigma_previous=sigma,sigma = sigma->next) {
      }
      sigma_splice = xxmalloc(sizeof(struct sigma));
      if (sigma_previous != NULL) {
	(sigma_previous)->next = sigma_splice;
	sigma_splice->number = symbol;
	sigma_splice->symbol = str;
	(sigma_splice)->next = sigma; 
	return(symbol);
      } else {
	sigma_splice->symbol = sigma->symbol;
	sigma_splice->number = sigma->number;
	sigma_splice->next = sigma->next;
	sigma->number = symbol;
	sigma->symbol = str;
	sigma->next = sigma_splice;
	return(symbol);
      }
    }
    sigma->next = NULL;
    sigma->symbol = str;
    return(symbol);
}

/* WARNING: this function will indeed add a symbol to sigma */
/* but it's up to the user to sort the sigma (affecting arc numbers in the network) */
/* before merge_sigma() is ever called */

int sigma_add (char *symbol, struct sigma *sigma) {
  int assert = -1;
  struct sigma *sigma_previous = NULL, *sigma_splice = NULL;

  /* Special characters */
  if (strcmp(symbol, "@_EPSILON_SYMBOL_@") == 0)
    assert = EPSILON;
  if (strcmp(symbol,"@_IDENTITY_SYMBOL_@") == 0)
    assert = IDENTITY;
  if (strcmp(symbol,"@_UNKNOWN_SYMBOL_@") == 0)
    assert = UNKNOWN;

  /* Insert non-special in any order */
  if (assert == -1) {
    if (sigma->number == -1) {
 	sigma->number = 3;
    } else {
      for (; sigma->next != NULL; sigma = sigma->next) {
      }
      sigma->next = xxmalloc(sizeof(struct sigma));
      if ((sigma->number)+1 < 3) {
	(sigma->next)->number = 3;
      } else {
	(sigma->next)->number = (sigma->number)+1;
      }
      sigma = sigma->next;
    }
    sigma->next = NULL;  
    sigma->symbol = xxstrdup(symbol);
    return(sigma->number);
  } else {
    /* Insert special symbols pre-sorted */
    if (sigma->number == -1) {
      sigma->number = assert;
    } else {
      for (;(sigma != NULL) && (sigma->number < assert) && (sigma->number!=-1); sigma_previous=sigma,sigma = sigma->next) {
      }
      sigma_splice = xxmalloc(sizeof(struct sigma));
      if (sigma_previous != NULL) {
	(sigma_previous)->next = sigma_splice;
	sigma_splice->number = assert;
	sigma_splice->symbol = xxmalloc(sizeof(char)*(strlen(symbol)+1));
	strcpy(sigma_splice->symbol, symbol);
	(sigma_splice)->next = sigma; 
	return(assert);
      } else {
	sigma_splice->symbol = sigma->symbol;
	sigma_splice->number = sigma->number;
	sigma_splice->next = sigma->next;
	sigma->number = assert;
	sigma->symbol = xxmalloc(sizeof(char)*(strlen(symbol)+1));
	strcpy(sigma->symbol, symbol);
	sigma->next = sigma_splice;
	return(assert);
      }
    }
    sigma->next = NULL;
    sigma->symbol = xxstrdup(symbol);
    return(assert);
  }
}

/* Remove symbols that are never used from sigma and renumber   */
/* The variable force controls whether to remove even though    */
/* @ or ? is present                                            */
/* If force == 1, unused symbols are always removed regardless  */

void sigma_cleanup (struct fsm *net, int force) {
    int i,j,first,maxsigma,*attested;
    struct fsm_state *fsm;
    struct sigma *sig, *sig_prev, *sign;
    
    if (force == 0) {
        if (sigma_find_number(IDENTITY, net->sigma) != -1)
            return;
        if (sigma_find_number(UNKNOWN, net->sigma) != -1)
            return;
    }

    maxsigma = sigma_max(net->sigma);
    if (maxsigma < 0) { return; }
    attested = xxmalloc(sizeof(int)*(maxsigma+1));
    for (i=0; i<=maxsigma; i++)
        *(attested+i) = 0;
    fsm = net->states;
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->in >=0)
            *(attested+(fsm+i)->in) = 1;
        if ((fsm+i)->out >=0)
            *(attested+(fsm+i)->out) = 1;
    }
    for (i=3,j=3; i<=maxsigma;i++ ) {
        if (*(attested+i)) {
            *(attested+i) = j;
            j++;
        }
    }
    for (i=0; (fsm+i)->state_no != -1; i++) {        
        if ((fsm+i)->in > 2)
            (fsm+i)->in = *(attested+(fsm+i)->in);
        if ((fsm+i)->out > 2)
            (fsm+i)->out = *(attested+(fsm+i)->out);
    }
    sig_prev = NULL;
    for (sig = net->sigma; sig != NULL && sig->number != -1; sig = sign) {
        first = 1;
	sign = sig->next;
        if (!*(attested+(sig->number))) {
	    xxfree(sig->symbol);
	    xxfree(sig);
            if (sig_prev != NULL) {
                sig_prev->next = sign;
                first = 0;
            } else {
                first = 0;
                net->sigma = sign;
            }
        } else {
            sig->number = sig->number >= 3 ? *(attested+(sig->number)) : sig->number;
        }
        if (first)
            sig_prev = sig;
    }
    xxfree(attested);
    return;
}

int sigma_max(struct sigma *sigma) {
  int i;
  for (i=-1; sigma != NULL; sigma = sigma->next)
      i = sigma->number > i ? sigma->number : i;
  return(i);
}

int sigma_size(struct sigma *sigma) {
  int i;
  for(i=0; sigma != NULL; sigma = sigma->next)
    i++;
  return(i);
}

struct fsm_sigma_list *sigma_to_list(struct sigma *sigma) {
    struct fsm_sigma_list *sl;
    struct sigma *s;
    sl = xxcalloc(sigma_max(sigma)+1,sizeof(struct fsm_sigma_list));
    for (s = sigma; s != NULL && s->number != -1; s = s->next) {
        (sl+(s->number))->symbol = s->symbol;
    }
    return sl;
}

int sigma_add_number(struct sigma *sigma, char *symbol, int number) {
    struct sigma *newsigma, *prev_sigma;
    prev_sigma = NULL;
    if (sigma->number == -1) {
        sigma->symbol = xxstrdup(symbol);
        sigma->number = number;
        sigma->next = NULL;
        return(1);
    }
    for (newsigma = sigma; newsigma != NULL; newsigma = newsigma->next) {
        prev_sigma = newsigma;
    }
    newsigma = xxmalloc(sizeof(struct sigma));
    newsigma->symbol = xxstrdup(symbol);
    newsigma->number = number;
    newsigma->next = NULL;
    prev_sigma->next = newsigma;
    return(1);
}

int sigma_find_number(int number, struct sigma *sigma) {
    if (sigma == NULL)
        return -1;
    if (sigma->number == -1) {
        return -1;
    }
    /* for (;(sigma != NULL) && (sigma->number <= number); sigma = sigma->next) { */
    for (;(sigma != NULL) && (sigma->number != -1); sigma = sigma->next) {
        if (number == sigma->number) {
            return (sigma->number);
        }
    }
    return -1;
}
char *sigma_string(int number, struct sigma *sigma) {
    if (sigma == NULL)
        return NULL;
    if (sigma->number == -1) {
        return NULL;
    }
    for (;(sigma != NULL) && (sigma->number != -1); sigma = sigma->next) {
        if (number == sigma->number) {
            return (sigma->symbol);
        }
    }
    return NULL;
}

/* Substitutes string symbol for sub in sigma */
/* no check for duplicates                    */
int sigma_substitute(char *symbol, char *sub, struct sigma *sigma) {
    if (sigma->number == -1) {
        return -1;
    }
    for (; sigma != NULL && sigma->number != -1 ; sigma = sigma->next) {
        if (strcmp(sigma->symbol, symbol) == 0) {
	    xxfree(sigma->symbol);
	    sigma->symbol = strdup(sub);
            return(sigma->number);
        }
    }
    return -1;
}

int sigma_find(char *symbol, struct sigma *sigma) {
    
    if (sigma->number == -1) {
        return -1;
    }
    for (; sigma != NULL && sigma->number != -1 ; sigma = sigma->next) {
        if (strcmp(sigma->symbol, symbol) == 0) {
            return (sigma->number);
        }
    }
    return -1;
}

struct ssort {
  char *symbol;
  int number;
};

int ssortcmp(const void *a, const void *b) {
  return(strcmp( ((struct ssort *)a)->symbol, ((struct ssort *)b)->symbol));
}

struct sigma *sigma_copy(struct sigma *sigma) {
    int f = 0;
    struct sigma *copy_sigma, *copy_sigma_s;

    if (sigma == NULL) { return NULL; }
    copy_sigma_s = xxmalloc(sizeof(struct sigma));

    for (copy_sigma = copy_sigma_s; sigma != NULL; sigma=sigma->next) {
	if (f == 1) {
	    copy_sigma->next = xxmalloc(sizeof(struct sigma));
	    copy_sigma = copy_sigma->next;
	}
	copy_sigma->number = sigma->number;
	if (sigma->symbol != NULL)
	    copy_sigma->symbol = xxstrdup(sigma->symbol);
	else
	    copy_sigma->symbol = NULL;
	copy_sigma->next = NULL;
	f = 1;
    }
    return(copy_sigma_s);
}

/* Assigns a consecutive numbering to symbols in sigma > IDENTITY */
/* and sorts the sigma based on the symbol string contents        */

int sigma_sort(struct fsm *net) {
  //int(*comp)() = ssortcmp;
  int size, i, max, *replacearray;
  struct ssort *ssort;
  struct sigma *sigma;
  struct fsm_state *fsm_state;
  
  size = sigma_max(net->sigma);
  if (size < 0) { return 1; }
  ssort = xxmalloc(sizeof(struct ssort)*size);

  for (i=0, sigma=net->sigma; sigma != NULL; sigma=sigma->next) {
    if (sigma->number > IDENTITY) {
      ssort[i].symbol = (char *)sigma->symbol;
      ssort[i].number = sigma->number;
      i++;
    }
  }
  max = i;
  qsort(ssort, max, sizeof(struct ssort), ssortcmp);
  replacearray = xxmalloc(sizeof(int)*(size+3));
  for (i=0; i<max; i++)
      replacearray[(ssort+i)->number] = i+3;

  /* Replace arcs */
  for(i=0, fsm_state = net->states; (fsm_state+i)->state_no != -1; i++) {
    if ((fsm_state+i)->in > IDENTITY)
      (fsm_state+i)->in = replacearray[(fsm_state+i)->in];
    if ((fsm_state+i)->out > IDENTITY)
      (fsm_state+i)->out = replacearray[(fsm_state+i)->out];
  }
  /* Replace sigma */
  for (i=0, sigma=net->sigma; sigma != NULL; sigma=sigma->next) {
    if (sigma->number > IDENTITY) {
      sigma->number = i+3;
      sigma->symbol = (ssort+i)->symbol;
      i++;
    }
  }
  xxfree(replacearray);
  xxfree(ssort);
  return(1);
}

struct sigma *sigma_create() {
  struct sigma *sigma;
  sigma = xxmalloc(sizeof(struct sigma));
  sigma->number = -1; /*Empty sigma*/
  sigma->next = NULL;
  sigma->symbol = NULL;
  return(sigma);
}
