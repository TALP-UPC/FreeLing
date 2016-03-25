/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2011 Mans Hulden                                     */

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
#include <locale.h>
#include "foma.h"

#define FAIL    1
#define SUCCEED 2
#define NONE    3

static struct flags *flag_extract (struct fsm *net);
static char *flag_type_to_char (int type);
static void flag_purge (struct fsm *net, char *name);
static struct fsm *flag_create_symbol(int type, char *name, char *value);

struct flags {
    int type;
    char *name;
    char *value;
    struct flags *next;
};

/* We eliminate all flags by creating a list of them and building a regex filter   */
/* that successively removes unwanted paths.  NB: flag_eliminate() called with the */
/* second argument NULL eliminates all flags.                                      */
/* The regexes we build for each flag symbol are of the format:                    */
/* ~[?* FAIL ~$SUCCEED THISFLAG ?*] for U,P,D                                      */
/* or                                                                              */
/* ~[(?* FAIL) ~$SUCCEED THISFLAG ?*] for the R flag                               */
/* The function flag_build() determines, depending on the flag at hand for each    */
/* of the other flags occurring in the network if it belongs in FAIL, SUCCEED,     */
/* or neither.                                                                     */
/* The languages FAIL, SUCCEED is then the union of all symbols that cause         */
/* compatibility or incompatibility.                                               */
/* We intersect all these filters, creating a large filter that we compose both on */ 
/* the upper side of the network and the lower side:                               */
/* RESULT = FILTER .o. ORIGINAL .o. FILTER                                         */
/* We can't simply intersect the language with FILTER because the lower side flags */
/* are independent of the upper side ones, and the network may be a transducer.    */
/* Finally, we replace the affected arcs with EPSILON arcs, and call               */
/* sigma_cleanup() to purge the symbols not occurring on arcs.                     */

/// 
///Eliminate a flag from a network. If called with name = NULL, eliminate all flags.
///

struct fsm *flag_eliminate(struct fsm *net, char *name) {

    struct flags *flags, *f, *ff;
    struct fsm *filter, *succeed_flags, *fail_flags, *self, *newfilter, *newnet;
    int flag, fstatus, found;

    filter = NULL;

    flags = flag_extract(net);
    /* Check that flag actually exists in net */
    if (name != NULL) { 
        for (found = 0, f = flags; f != NULL; f = f->next) {
            if (strcmp(name,f->name) == 0)
                found = 1;
        }
        if (found == 0) {
	    fprintf(stderr,"Flag attribute '%s' does not occur in the network.\n",name);
            return(net);
        }
    }

    flag = 0;

    for (f = flags; f != NULL; f = f->next) {

        if ((name == NULL || strcmp(f->name,name) == 0) &&
            (f->type | FLAG_UNIFY | FLAG_REQUIRE | FLAG_DISALLOW | FLAG_EQUAL)) {
            
            succeed_flags = fsm_empty_set();
            fail_flags = fsm_empty_set();
            self = flag_create_symbol(f->type, f->name, f->value);
            
            for (ff = flags, flag = 0; ff != NULL; ff = ff->next) {
                fstatus = flag_build(f->type, f->name, f->value, ff->type, ff->name, ff->value);
                if (fstatus == FAIL) {
                    fail_flags = fsm_minimize(fsm_union(fail_flags, flag_create_symbol(ff->type, ff->name, ff->value)));
                    flag = 1;
                }
                if (fstatus == SUCCEED) {
                    succeed_flags = fsm_minimize(fsm_union(succeed_flags, flag_create_symbol(ff->type, ff->name, ff->value)));
                    flag = 1;
                }
            }
        }

        if (flag) {
            if (f->type == FLAG_REQUIRE) {
                newfilter = fsm_complement(fsm_concat(fsm_optionality(fsm_concat(fsm_universal(), fail_flags)), fsm_concat(fsm_complement(fsm_contains(succeed_flags)), fsm_concat(self, fsm_universal()))));
                
            } else {
                newfilter = fsm_complement(fsm_contains(fsm_concat(fail_flags,fsm_concat(fsm_complement(fsm_contains(succeed_flags)),self))));
            }

            filter = (filter == NULL) ? newfilter : fsm_intersect(filter, newfilter);
        }
        flag = 0;
    }
    if (filter != NULL) {
        extern int g_flag_is_epsilon;
        int old_g_flag_is_epsilon;
        old_g_flag_is_epsilon = g_flag_is_epsilon;
        g_flag_is_epsilon = 0;
        newnet = fsm_compose(fsm_copy(filter),fsm_compose(net,fsm_copy(filter)));
        g_flag_is_epsilon = old_g_flag_is_epsilon;
    } else {
        newnet = net;
    }
    flag_purge(newnet, name);
    newnet = fsm_minimize(newnet);
    sigma_cleanup(newnet,0);
    xxfree(flags);
    return(fsm_topsort(newnet));
}

struct fsm *flag_create_symbol(int type, char *name, char *value) {
    char *string;
    if (value == NULL)
        value = "";

    string = xxmalloc(sizeof(char)*strlen(name)+strlen(value)+6);
    *string = '\0';
    strcat(string, "@");
    strcat(string, flag_type_to_char(type));
    strcat(string, ".");
    strcat(string, name);
    if (strcmp(value,"") != 0) {
        strcat(string, ".");    
        strcat(string, value);
    }
    strcat(string, "@");

    return(fsm_symbol(string));

}

char *flag_type_to_char (int type) {
    switch(type) {
    case FLAG_UNIFY:
        return("U");
    case FLAG_CLEAR:
        return("C");
    case FLAG_DISALLOW:
        return("D");
    case FLAG_NEGATIVE:
        return("N");
    case FLAG_POSITIVE:
        return("P");
    case FLAG_REQUIRE:
        return("R");
    case FLAG_EQUAL:
        return("E");
    }
    return NULL;
}

int flag_build(int ftype, char *fname, char *fvalue, int fftype, char *ffname, char *ffvalue) {
    int valeq, selfnull;

    selfnull = 0; /* If current flag has no value, e.g. @R.A@ */
    if (strcmp(fname,ffname) != 0)
        return NONE;
    
    if (fvalue == NULL) {
        fvalue = "";
        selfnull = 1;
    }
    
    if (ffvalue == NULL)
        ffvalue = "";

    valeq = strcmp(fvalue, ffvalue);
    /* U flags */
    if (ftype == FLAG_UNIFY && fftype == FLAG_POSITIVE && valeq == 0)
        return SUCCEED;
    if (ftype == FLAG_UNIFY && fftype == FLAG_CLEAR)
        return SUCCEED;
    if (ftype == FLAG_UNIFY && fftype == FLAG_UNIFY && valeq != 0)
        return FAIL;
    if (ftype == FLAG_UNIFY && fftype == FLAG_POSITIVE && valeq != 0)
        return FAIL;
    if (ftype == FLAG_UNIFY && fftype == FLAG_NEGATIVE && valeq == 0)
        return FAIL;

    /* R flag with value = 0 */
    if (ftype == FLAG_REQUIRE && fftype == FLAG_UNIFY && selfnull)
        return SUCCEED;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_POSITIVE && selfnull)
        return SUCCEED;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_NEGATIVE && selfnull)
        return SUCCEED;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_CLEAR && selfnull)
        return FAIL;

    /* R flag with value */
    if (ftype == FLAG_REQUIRE && fftype == FLAG_POSITIVE && valeq == 0 && !selfnull)
        return SUCCEED;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_UNIFY && valeq == 0 && !selfnull)
        return SUCCEED;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_POSITIVE && valeq != 0 && !selfnull)
        return FAIL;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_UNIFY && valeq != 0 && !selfnull)
        return FAIL;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_NEGATIVE && !selfnull)
        return FAIL;
    if (ftype == FLAG_REQUIRE && fftype == FLAG_CLEAR && !selfnull)
        return FAIL;

    /* D flag with value = 0 */
    if (ftype == FLAG_DISALLOW && fftype == FLAG_CLEAR && selfnull)
        return SUCCEED;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_POSITIVE && selfnull)
        return FAIL;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_UNIFY && selfnull)
        return FAIL;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_NEGATIVE && selfnull)
        return FAIL;

    /* D flag with value */
    if (ftype == FLAG_DISALLOW && fftype == FLAG_POSITIVE && valeq != 0 && !selfnull)
        return SUCCEED;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_CLEAR && !selfnull)
        return SUCCEED;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_NEGATIVE  && valeq == 0 && !selfnull)
        return SUCCEED;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_POSITIVE && valeq == 0 && !selfnull)
        return FAIL;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_UNIFY && valeq == 0 && !selfnull)
        return FAIL;
    if (ftype == FLAG_DISALLOW && fftype == FLAG_NEGATIVE  && valeq != 0 && !selfnull)
        return FAIL;

    return NONE;
}


/* Remove flags that are being eliminated from arcs and sigma */

void flag_purge (struct fsm *net, char *name) {
    struct fsm_state *fsm;
    struct sigma *sigma;
    int i, *ftable, sigmasize;
    char *csym;
    sigmasize = sigma_max(net->sigma)+1;
    ftable = xxmalloc(sizeof(int) * sigmasize);
    fsm = net->states;
    for (i=0; i<sigmasize; i++)
        *(ftable+i)=0;
    
    for (sigma = net->sigma; sigma != NULL && sigma->number != -1; sigma = sigma->next) {
        
        if (flag_check(sigma->symbol)) {
            if (name == NULL) {
                *(ftable+(sigma->number)) = 1;
            } else {
                csym = (sigma->symbol) + 3;
                if (strncmp(csym,name,strlen(name)) == 0 && (strlen(csym)>strlen(name)) && (strncmp(csym+strlen(name),".",1) == 0 || strncmp(csym+strlen(name),"@",1) == 0)) {
                    *(ftable+(sigma->number)) = 1;
                }
            }
        }
    }
    for (i = 0; i < sigmasize; i++) {
	if (*(ftable+i)) {
	    net->sigma = sigma_remove_num(i, net->sigma);
	}
    }

    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->in >= 0 && (fsm+i)->out >= 0) {
            if (*(ftable+(fsm+i)->in))
                (fsm+i)->in = EPSILON;
            if (*(ftable+(fsm+i)->out))
                (fsm+i)->out = EPSILON;
        }
    }

    xxfree(ftable);
    net->is_deterministic = net->is_minimized = net->is_epsilon_free = NO;
    return;
}

/* Extract all flags from network and place them in struct flag linked list */

struct flags *flag_extract (struct fsm *net) {
    struct sigma *sigma;
    struct flags *flags, *flagst;

    flags = NULL;
    for (sigma = net->sigma ; sigma != NULL; sigma = sigma->next) {
        if (flag_check(sigma->symbol)) {
            flagst = xxmalloc(sizeof(struct flags));
            flagst->next = flags;
            flags = flagst;
            
            flags->type  = flag_get_type(sigma->symbol);
            flags->name  = flag_get_name(sigma->symbol);
            flags->value = flag_get_value(sigma->symbol);
        }        
    }    
    return(flags);
}

int flag_check(char *s) {
    
    /* We simply simulate this regex (where ND is not dot) */
    /* "@" [U|P|N|R|E|D] "." ND+ "." ND+ "@" | "@" [D|R|C] "." ND+ "@" */
    /* and return 1 if it matches */

    int i;
    i = 0;
    
    if (*(s+i) == '@') { i++; goto s1; } return 0;
 s1:
    if (*(s+i) == 'C') { i++; goto s4; }
    if (*(s+i) == 'N' || *(s+i) == 'E' || *(s+i) == 'U' || *(s+i) == 'P') { i++; goto s2; } 
    if (*(s+i) == 'R' || *(s+i) == 'D') { i++; goto s3; } return 0;
 s2:
    if (*(s+i) == '.') { i++; goto s5; } return 0;
 s3:
    if (*(s+i) == '.') { i++; goto s6; } return 0;
 s4:
    if (*(s+i) == '.') { i++; goto s7; } return 0;
 s5:
    if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s8; } return 0;   
 s6:
    if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s9; } return 0;   
 s7:
    if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s10; } return 0;
 s8:
   if (*(s+i) == '.') { i++; goto s7; } 
   if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s8; } return 0; 
 s9:
    if (*(s+i) == '@') { i++; goto s11; }
    if (*(s+i) == '.') { i++; goto s7; }
    if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s9; } return 0;

 s10:
    if (*(s+i) == '@') {i++; goto s11;} 
    if (*(s+i) != '.' && *(s+i) != '\0') { i++; goto s10; } return 0;
 s11:
    if (*(s+i) == '\0') {return 1;} return 0;
}

int flag_get_type(char *string) {
    if (strncmp(string+1,"U.",2) == 0) {
	return FLAG_UNIFY;
    }    
    if (strncmp(string+1,"C.",2) == 0) {
	return FLAG_CLEAR;
    }    
    if (strncmp(string+1,"D.",2) == 0) {
	return FLAG_DISALLOW;
    }    
    if (strncmp(string+1,"N.",2) == 0) {
	return FLAG_NEGATIVE;
    }    
    if (strncmp(string+1,"P.",2) == 0) {
	return FLAG_POSITIVE;
    }    
    if (strncmp(string+1,"R.",2) == 0) {
	return FLAG_REQUIRE;
    }    
    if (strncmp(string+1,"E.",2) == 0) {
	return FLAG_EQUAL;
    }    
    return 0;
}

char *flag_get_name(char *string) {
    int i, start, end, len;
    start = end = 0;
    len = strlen(string);

    for (i=0; i < len; i += (utf8skip(string+i) + 1)) {
	if (*(string+i) == '.' && start == 0) {
	    start = i+1;
	    continue;
	}
	if ((*(string+i) == '.' || *(string+i) == '@')  && start != 0) {
	    end = i;
	    break;
	}
    }
    if (start > 0 && end > 0) {
	return(xxstrndup(string+start,end-start));
    }
    return NULL;
}

char *flag_get_value(char *string) {
    int i, first, start, end, len;
    first = start = end = 0;
    len = strlen(string);

    for (i=0; i < len; i += (utf8skip(string+i) + 1)) {
	if (*(string+i) == '.' && first == 0) {
	    first = i+1;
	    continue;
	}
	if (*(string+i) == '@' && start != 0) {
	    end = i;
	    break;
	}
	if (*(string+i) == '.' && first != 0) {
	    start = i+1;
	    continue;
	}
    }
    if (start > 0 && end > 0) {
	return(xxstrndup(string+start,end-start));
    }
    return NULL;
}

struct fsm *flag_twosided(struct fsm *net) {
  struct fsm_state *fsm;
  struct sigma *sigma;
  int i, j, tail, *isflag, maxsigma, maxstate, newarcs, change;
 
  /* Enforces twosided flag diacritics */
  
  /* Mark flag symbols */
  maxsigma = sigma_max(net->sigma);
  isflag = xxcalloc(maxsigma+1, sizeof(int));
  fsm = net->states;
  for (sigma = net->sigma ; sigma != NULL; sigma = sigma->next) {
    if (flag_check(sigma->symbol)) {
      *(isflag+sigma->number) = 1;
    } else {
      *(isflag+sigma->number) = 0;
    }
  }
  maxstate = 0;
  change = 0;
  for (i = 0, newarcs = 0; (fsm+i)->state_no != -1 ; i++) {
    maxstate = (fsm+i)->state_no > maxstate ? (fsm+i)->state_no : maxstate;
    if ((fsm+i)->target == -1)
      continue;
    if (*(isflag+(fsm+i)->in) && (fsm+i)->out == EPSILON) {
      change = 1;
      (fsm+i)->out = (fsm+i)->in;
    }
    else if (*(isflag+(fsm+i)->out) && (fsm+i)->in == EPSILON) {
      change = 1;
      (fsm+i)->in = (fsm+i)->out;
    }
    if ((*(isflag+(fsm+i)->in) || *(isflag+(fsm+i)->out)) && (fsm+i)->in != (fsm+i)->out) {
      newarcs++;
    }
  }

  if (newarcs == 0) {
    if (change == 1) {
      net->is_deterministic = UNK;
      net->is_minimized = UNK;
      net->is_pruned = UNK;
      return fsm_topsort(fsm_minimize(net));
    }
    return net;
  }
  net->states = xxrealloc(net->states, sizeof(struct fsm)*(i+newarcs));
  fsm = net->states;
  tail = j = i;
  maxstate++;
  for (i = 0; i < tail; i++) {

    if ((fsm+i)->target == -1) 
      continue;
    if ((*(isflag+(fsm+i)->in) || *(isflag+(fsm+i)->out)) && (fsm+i)->in != (fsm+i)->out) {
      if (*(isflag+(fsm+i)->in) && !*(isflag+(fsm+i)->out)) {
	j = add_fsm_arc(fsm, j, maxstate, EPSILON, (fsm+i)->out, (fsm+i)->target, 0, 0);
	(fsm+i)->out = (fsm+i)->in;
	(fsm+i)->target = maxstate;
	maxstate++;
      }
      else if (*(isflag+(fsm+i)->out) && !*(isflag+(fsm+i)->in)) {
	j = add_fsm_arc(fsm, j, maxstate, (fsm+i)->out, (fsm+i)->out, (fsm+i)->target, 0, 0);
	(fsm+i)->out = EPSILON;
	(fsm+i)->target = maxstate;
	maxstate++;
      }
      else if (*(isflag+(fsm+i)->in) && *(isflag+(fsm+i)->out)) {
	j = add_fsm_arc(fsm, j, maxstate, (fsm+i)->out, (fsm+i)->out, (fsm+i)->target, 0, 0);
	(fsm+i)->out = (fsm+i)->in;
	(fsm+i)->target = maxstate;
	maxstate++;
      }
    }
  }
  /* Add sentinel */
  add_fsm_arc(fsm, j, -1, -1, -1, -1, -1, -1);
  net->is_deterministic = UNK;
  net->is_minimized = UNK;
  return fsm_topsort(fsm_minimize(net));
}
