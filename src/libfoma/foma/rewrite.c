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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foma.h"

/*
 * Short version:

 * 1. We construct a language with centers interspersed with arbitrary single-symbol
 *    identity relations (Insert). Centers are A:B cross products (for a rule A -> B)
 * 2. We constrain the centers to occur only in the proper contexts. (Context)
 * 3. We constrain the centers to have to occur if there is a proper context (Coerce)
 * 4. We add a few Additional constraints if the rules is longest-match, dotted, etc.
 * 5. We create [Insert & Context & Coerce & Additional] which is the collection
      of our rules.
 * 6. Of course, if we have multiple parallel rules and contexts, we intersect
      all those in the above Context, Coerce, Additional...
 * 7. Voilá

 * Longer version (how it actually works):

 * The actual encoding of rewrite rules is as follows:
 * 1. An automaton is interpreted as a transducer with sections marked inside brackets
 *  [...<...>...] as a transduction, and outside sections as identity relations.
 *  For example the string: "ab[a>b<a>Z]ab"
 *  represents a transducer a:a b:b a:b a:0 a:a b:b
 *  where Z is a "hard zero"
 *
 *  So [ signifies  : start of rewrite rule center
 *     > signifies  : the following symbol is on the lower side only
 *     < signifies  : the following symbol is on the upper side only
 *     ] signifies  : end of rewrite rule center
 *      Any symbol not preceded by [,<,> is an Id symbol
 *
 * 2. A rewriting transducer is constructed as follows:
 *   a. call *rewrite_cp() to build a center A:B string
 *      e.g. A = abc , B = de
 *      becomes CP = a>d<b>e<c>0
 *      if the relation is not equal length, the shorter string
 *      is padded with a special hard zero symbol
 *
 *   b. construct the language Insert = @#@ NoSpecial* /[ %[ CP %] ] @#@
*       NoSpecial being any single non-auxiliary symbol
 *      i.e. identity relations interspersed with cross-products
 *      and where every string begins and ends with the special symbol @#@      
 *      Now we have strings such as aaa[a>0]a[a>b]va
 *      Where the bracketed sequences are rewrite centers
 *      Naturally, for many parallel rules, we take the union of *all* rule
 *      cross-products in CP.   
 *      Two additions:
 *      i) if the rule is dotted, e.g. [..] -> b
 *         we get stuff like "xxx[[0>b]]xxx" ... 
 *         with two opening and closing brackets
 *      ii) if the rule is a chunking rule A -> B ...C 
 *          we get stuff like "xxx[0>BAAA0>C]xxx" where the A's are Id(a)
 *          
 *   c. constrain the bracketed sequences as per the type of rewrite rule
 *      Replace = [Insert & Context & Coerce]
 *      Context is a context restriction statement on the legitimacy of the [ ] sequence
 *      Coerce comes in two varieties depending on the rewrite arrow:
 *      CoerceU forces every proper context to have a rewrite ->
 *      CoerceL is the <- equivalent (CoerceU&CoerceL encodes <->)
 *      - LR is a left-to-right constraint, and LM is a longest-match constraint
 *      - SM is shortest match.  These are added as intersections in Replace
 *        depending on the rule type.
 *      - Optional rules have no "Coerce"
 *      - Dotted rules [.A.] -> B are separated into the empty part
 *        A&0 and the non empty A-0 and are treated as separate parallel rules
 *        the empty part has a separate Coerce & Context statement
 *        restricting and coercing the rule center to occur in the proper
 *        context if also flanked by i) and Id or ii) some symbol on the 
 *        upper side (i.e. not [[ which indicates dotted centers 
 *          
 *   d. Construct a transducer from the bracketed language by calling
 *      rewrite_cp_to_fst() which converts single-tape languages with strings
 *      such as a[c>d]e to a:a c:d e:e and removes the auxiliaries.
 *      We now remove the @#@ symbols as well
 *
 *
 * More detail:

 *  Insert = @#@ NoSpecial/[ %[ UnionCP %] ] @#@
 *  i.e. as described above.  UnionCP = C1|C2|C3... 
 *  cross products where C1 = [A:B] for a rule A op B

 *  Context = [ => Dir(L) _ C1 Dir(R) , Dir(L2) _ C1 Dir(R2) , ...
 *  i.e. the opening bracket for a rule center [ is only allowed
 *  if there is a legitimate center followed by a legitimate right context
 *  to the right, and a legimate left context to the left.
 *  Since contexts may be one-sided || or \/ etc. we use the function Dir(L)
 *  to ignore either the upper or the lower side of the context, depending
 *  on the context pair's directionality

 *  Coerce =  ~[[?* -[?* %>]] Dir(L) A [Dir(R) ?* & OSR ?*] ?*]
 *  OSR  = NoSpecial* %[ | @#@
 *  i.e. Coerce is the language that does not contain a upper-side center A
 *  occurring outside brackets (that's what OSR is for) if the contexts
 *  are correct.  If the rule arrow is <-, replace A with B, yielding
 *  a coercion on the lower part of the A -> B.
 *  the [?* -[?* %>]] part could be replaced with ?* but at a huge
 *  cost of nondeterminism, so it's really an efficiency tweak.
 *  For coerce, we take the intersection of every center
 *  and context pair (to which the center applies).

 *  For the empty center part of dotted rules, we use: CoerceD = 
 *  ~[[EndOutside & [?* [Id|Sigl]] & [?* Dir(L)] ] [ [Dir(R) ?*] & [Id|Sigr] ?*]]
 *  where Sigl = \%] %];
 *  and   Sigr = %[ \%[;
 *  meaning the language that never contains a L R sequence
 *  if both L and R are identity symbols or regular [ rules
 *  ruling out sequences such as x [a>0]
 *                                ^  
 *  if x is the left context and a the right one.

 *  LR = ~[[[EndOutside] & [?* Dir(L)]] [[Dir(C) - [%[ ?*]] & $%[  ] Dir(R) ?*]
 *  This is our left-to-right restriction.  EndOutside = ~[?* %[ \%]*] - [?* %>]
 *  (this again is a tweak: endoutside could be defined without the subtraction).
 *  The logic of LR is simple: we shouldn't gave a Center inside C ... [ ] sequence 
 *  where C extends inside the bracket: meaning, we could have started rewriting
 *  earlier.

 *  LM = ~$[Dir(L) [ %[ [ [ Upper(C) - [%[ ?* | ?* %] ] ] & $[ %] Upper(NoSpecial) ] ] Dir(R)];
 *  Longest-matched is simply the language where we don't find a L [ C R sequence
 *  where the C contains inside it a ], indicating we could have chosen a longer
 *  rewrite.

 *  SM = ~$[Dir(L) %[ C/Low1  [NoSpecial/Low1 ?* & Dir(R) ?*]]
 *  Here Low1 is the language we use to ignore the lower side inside [ ] brackets
 *  The logic is this: We don't allow a L [ C R sequence where the C doesn't end
 *  in ], i.e. is followed by at least one symbol which is not ] 
 *  (indicating we could have chosen a longer rewrite).

 *  Additional notes: we have a special function to perform the context restriction
 *  involved in constructing Context for two reasons:
 *  1) We want to maintain the @#@ symbols and not treat them as special which
 *     the generic context_restrict doesn't do.
 *  2) We need some efficiency tweaks, such as are done in Coerce to 
 *     minimize nondeterminism.
 
 *  If we have dotted rules we extend Context with [ => [ _ also.
 */


struct fsm *rewrite_nospecial();
struct fsm *rewr_context_restrict(struct fsm *X, struct fsmcontexts *LR);
struct fsm *rewrite_pad(struct fsm *net);

void rewrite_add_special_syms(struct fsm *net) {
    if (net == NULL)
        return;
    sigma_substitute(".#.", "@#@", net->sigma); /* We convert boundaries to our interal rep.      */
                                                /* This is because sigma merging is handled       */
                                                /* in a special way for .#., which we don't want. */
    if (sigma_find("@#@", net->sigma) == -1)
	sigma_add("@#@",net->sigma);

    sigma_add("@Z@",net->sigma);
    sigma_add("@<@",net->sigma);
    sigma_add("@>@",net->sigma);
    sigma_add("@]@",net->sigma);
    sigma_add("@[@",net->sigma);
    sigma_sort(net);
}

struct fsm *rewr_igupp(struct fsm *net) {
    struct fsm *uppsym, *NoSpecial;
    NoSpecial = rewrite_nospecial();
   
    uppsym = fsm_minimize(fsm_kleene_star(fsm_union(fsm_symbol("@Z@"),fsm_union(fsm_symbol("@>@"),fsm_union(fsm_symbol("@]@"),fsm_union(fsm_concat(fsm_symbol("@<@"),fsm_copy(NoSpecial)),fsm_union(fsm_concat(fsm_kleene_plus(fsm_symbol("@[@")),fsm_copy(NoSpecial)),fsm_union(fsm_concat(fsm_symbol("@<@"),fsm_symbol("@Z@")),fsm_concat(fsm_kleene_plus(fsm_symbol("@]@")),fsm_symbol("@Z@"))))))))));
    fsm_destroy(NoSpecial);
    return(fsm_ignore(net,uppsym,OP_IGNORE_ALL));
}

struct fsm *rewr_iglow(struct fsm *net) {
    struct fsm *lowsym, *NoSpecial;
    NoSpecial = rewrite_nospecial();
    
    lowsym = fsm_minimize(fsm_kleene_star(fsm_union(fsm_symbol("@Z@"),fsm_union(fsm_symbol("@<@"),fsm_union(fsm_kleene_star(fsm_symbol("@[@")),fsm_union(fsm_kleene_star(fsm_symbol("@]@")),fsm_union(fsm_concat(fsm_symbol("@>@"),NoSpecial),fsm_concat(fsm_symbol("@>@"),fsm_symbol("@Z@")))))))));
    
    return(fsm_ignore(net,lowsym,OP_IGNORE_ALL));
}

struct fsm *rewr_iglow_inside(struct fsm *net) {
    struct fsm *lowsym, *NoSpecial;
    NoSpecial = rewrite_nospecial();

    lowsym = fsm_minimize(fsm_union(fsm_symbol("@Z@"),fsm_union(fsm_symbol("@<@"),fsm_union(fsm_concat(fsm_symbol("@>@"),NoSpecial),fsm_concat(fsm_symbol("@>@"),fsm_symbol("@Z@"))))));
    
    return(fsm_ignore(net,lowsym,OP_IGNORE_ALL));
}

struct fsm *rewrite_nospecial() {
    struct fsm *net;
    net = fsm_create("");
    net->states = xxmalloc(sizeof(struct fsm_state)*3);
    sigma_add_special(IDENTITY,net->sigma);
    sigma_add("@Z@",net->sigma);
    sigma_add("@#@",net->sigma);
    sigma_add("@<@",net->sigma);
    sigma_add("@>@",net->sigma);
    sigma_add("@]@",net->sigma);
    sigma_add("@[@",net->sigma);
    add_fsm_arc(net->states,0,0,IDENTITY,IDENTITY,1,0,1);
    add_fsm_arc(net->states,1,1,-1,-1,-1,1,0);
    add_fsm_arc(net->states,2,-1,-1,-1,-1,-1,-1);
    sigma_sort(net);
    return(net);
}

struct fsm *fsm_rewrite(struct rewrite_set *all_rules) {
    struct rewrite_set *ruleset;
    struct fsmrules *rules;
    struct fsmcontexts *contexts, *allcontexts, *newcontext;
    struct fsm *UnionCP, *RuleCP, *UnionCPI, *Insert, *NoSpecial, *Context, *ContextD, *Result, *Coerce, *CoerceLR, *CoerceLM, *CoerceSM, *thisCoerce, *SigL, *SigR, *Id, *Outside, *EndOutside, *CoerceCenter = NULL;
    int dir, c, minimal_old, dottedrules, num_parallel_rules;

    extern int g_minimal;

    dottedrules = 0;

    /* Preprocess by adding all special symbols to sigmas */
    num_parallel_rules = 0;
    for (ruleset = all_rules; ruleset != NULL; ruleset = ruleset->next) {
        num_parallel_rules++;
        for (rules = ruleset->rewrite_rules; rules != NULL; rules = rules->next) {
            rewrite_add_special_syms(rules->left);
	    rewrite_add_special_syms(rules->right);
            rewrite_add_special_syms(rules->right2);
        }
        for (contexts = ruleset->rewrite_contexts; contexts != NULL; contexts = contexts->next) {
            rewrite_add_special_syms(contexts->left);
            rewrite_add_special_syms(contexts->right);
        }
    }
    /* Do the CP for all rules and store it in the set */
    /* Do the union of all rules */
    UnionCP = fsm_empty_set();
    //printf("Initial CP and Ignore\n"); fflush(stdout);

    NoSpecial = rewrite_nospecial();
    Id = fsm_minimize(fsm_union(fsm_copy(NoSpecial),fsm_symbol("@#@")));

    for (ruleset = all_rules; ruleset != NULL; ruleset = ruleset->next) {
        for (rules = ruleset->rewrite_rules; rules != NULL; rules = rules->next) {
	    /* insert transducer instead of cross-product */
            if (rules->right == NULL) {
		/* Make rules->left upper side of T */
		/* Convert transducer to single-tape format */
		rules->cross_product = rewrite_pad(fsm_flatten(fsm_copy(rules->left), fsm_symbol("@Z@")));
		rules->right = fsm_minimize(fsm_lower(fsm_copy(rules->left)));
		rules->left = fsm_minimize(fsm_upper(rules->left));
		rewrite_add_special_syms(rules->right);
	    }
            else if (rules->right2 == NULL) {
		/* Regular rewrite rule */
		rules->cross_product = rewrite_cp(fsm_copy(rules->left),fsm_copy(rules->right));
            } else {
                /* A -> B ... C type rule */
                /* 0>BA0>C */
                rules->cross_product = fsm_minimize(fsm_concat(rewrite_cp(fsm_empty_string(),fsm_copy(rules->right)),fsm_concat(fsm_copy(rules->left),rewrite_cp(fsm_empty_string(),fsm_copy(rules->right2)))));
            }
            if ((rules->arrow_type & ARROW_DOTTED) != 0) {
                dottedrules++;                
                //printf("RET\n");
                rules->cross_product = fsm_minimize(fsm_concat(fsm_symbol("@[@"),fsm_concat(rules->cross_product,fsm_symbol("@]@"))));
            }
            UnionCP = fsm_minimize(fsm_union(fsm_copy(rules->cross_product),UnionCP));
        }

        /* Transform every context to ignore Upper/Lower */
        dir = ruleset->rule_direction;
        for (c = 0, contexts = ruleset->rewrite_contexts; contexts != NULL; contexts = contexts->next) {
            c = 1;
            if (dir == OP_UPWARD_REPLACE) {
                contexts->cpleft  = rewr_iglow(fsm_copy(contexts->left));
                contexts->cpright = rewr_iglow(fsm_copy(contexts->right));
            }
            if (dir == OP_DOWNWARD_REPLACE) {
                contexts->cpleft  = rewr_igupp(fsm_copy(contexts->left));
                contexts->cpright = rewr_igupp(fsm_copy(contexts->right));
            }
            if (dir == OP_LEFTWARD_REPLACE) {
                contexts->cpleft  = rewr_iglow(fsm_copy(contexts->left));
                contexts->cpright = rewr_igupp(fsm_copy(contexts->right));
            }
            if (dir == OP_RIGHTWARD_REPLACE) {
                contexts->cpleft  = rewr_igupp(fsm_copy(contexts->left));
                contexts->cpright = rewr_iglow(fsm_copy(contexts->right));
            }
        }
        if (c == 0) {
            ruleset->rewrite_contexts = xxcalloc(1,sizeof(struct fsmcontexts));
            ruleset->rewrite_contexts->cpleft = fsm_empty_string();
            ruleset->rewrite_contexts->cpright = fsm_empty_string();        
            ruleset->rewrite_contexts->next = NULL;        
        }
    }

    /* UnionCP now holds _all_ cross products needed for Insert */
    /* Insert = .#. NoSpecial/[ %[ UnionCP %] ] .#. */

    UnionCPI = fsm_minimize(fsm_concat(fsm_symbol("@[@"),fsm_minimize(fsm_concat(fsm_copy(UnionCP),fsm_symbol("@]@")))));

    Insert = fsm_minimize(fsm_ignore(fsm_kleene_star(fsm_copy(NoSpecial)), UnionCPI, OP_IGNORE_ALL));
    Insert = fsm_minimize(fsm_concat(fsm_symbol("@#@"),fsm_minimize(fsm_concat(Insert,fsm_symbol("@#@")))));
    
    /* Context = [ => Lower(L) _ C1|...|Cn ] Upper(R) */
    //printf("RuleCP\n"); fflush(stdout);
    for (ruleset = all_rules; ruleset != NULL; ruleset = ruleset->next) {        
        RuleCP = fsm_empty_set();
        for (rules = ruleset->rewrite_rules; rules != NULL; rules = rules->next) {
            RuleCP = fsm_minimize(fsm_union(RuleCP,fsm_copy(rules->cross_product)));
	    fsm_destroy(rules->cross_product);
        }
        ruleset->cpunion = RuleCP; /* Store every rule's individual cross-product */
    }

    allcontexts = NULL;
    /* Do Context */

    //printf("Context\n"); fflush(stdout); 
    for (ruleset = all_rules; ruleset != NULL; ruleset = ruleset->next) {        
        /* For every context pair, add its rule union to list of restricts */        
        for (contexts = ruleset->rewrite_contexts; contexts != NULL; contexts = contexts->next) {
            newcontext = xxcalloc(1,sizeof(struct fsmcontexts));
            /* left = L */
            newcontext->left = fsm_copy(contexts->cpleft);
            /* right = Center ] R */
            newcontext->right = fsm_minimize(fsm_concat(fsm_copy(ruleset->cpunion),fsm_concat(fsm_symbol("@]@"),fsm_copy(contexts->cpright))));
            newcontext->next = allcontexts;
            allcontexts = newcontext;
        }
	fsm_destroy(ruleset->cpunion);
        ruleset->cpunion = RuleCP;
    }
    //printf("Doing context\n");


    /* Suspend minimization for Context */
    minimal_old = g_minimal;
    g_minimal = 0;

    if (allcontexts == NULL) {
        Context = fsm_universal();
    } else {
        if (dottedrules == 0) {        
            Context = rewr_context_restrict(fsm_symbol("@[@"),allcontexts);
        } else {
            //printf("Doing dottedcontext\n");

            newcontext = xxcalloc(1,sizeof(struct fsmcontexts));            
            newcontext->left = fsm_symbol("@[@");
            newcontext->right = fsm_empty_string();
            newcontext->next = allcontexts;
            allcontexts = newcontext;
            Context = rewr_context_restrict(fsm_symbol("@[@"),allcontexts);

            newcontext = xxcalloc(1,sizeof(struct fsmcontexts));
            newcontext->left = fsm_minimize(fsm_union(fsm_copy(Id),fsm_concat(fsm_term_negation(fsm_symbol("@]@")),fsm_symbol("@]@"))));
            newcontext->right = fsm_empty_string();
            newcontext->next = NULL;

            ContextD = fsm_minimize(fsm_context_restrict(fsm_minimize(fsm_concat(fsm_symbol("@[@"),fsm_symbol("@[@"))), newcontext));
            sigma_remove("@#@",ContextD->sigma);
            Context = fsm_intersect(Context,ContextD);
	    fsm_destroy(newcontext->left);
	    fsm_destroy(newcontext->right);
	    xxfree(newcontext);
        }
    }

    g_minimal = minimal_old;

    /* For all L C R combos */
    /* ~[?* - [?* @>@] Dir(L) A [Dir(R) ?* & Outside ?*] ?*] for -> */
    /* ~[?* - [?* @>@] Dir(L) B [Dir(R) ?* & Outside ?*] ?*] for <- */
    
    /* Outside = [NoSpecial* [%[|%#]]; */
    Outside = fsm_minimize(fsm_concat(fsm_kleene_star(fsm_copy(NoSpecial)),fsm_union(fsm_symbol("@[@"),fsm_symbol("@#@"))));
    /* EndOutside = ~[?* %[ \%]*] - [?* %>] */
    EndOutside = fsm_complement(fsm_concat(fsm_universal(),fsm_concat(fsm_symbol("@[@"),fsm_kleene_star(fsm_term_negation(fsm_symbol("@]@"))))));
    EndOutside = fsm_minus(EndOutside, fsm_concat(fsm_universal(),fsm_symbol("@>@")));
    
    Coerce = NULL;
    Coerce = fsm_copy(Insert);

    for (ruleset = all_rules; ruleset != NULL; ruleset = ruleset->next) {
        for (rules = ruleset->rewrite_rules; rules != NULL; rules = rules->next) {
            for (contexts = ruleset->rewrite_contexts; contexts != NULL; contexts = contexts->next) {
                                
                if ((rules->arrow_type & (ARROW_LEFT | ARROW_RIGHT)) == (ARROW_LEFT|ARROW_RIGHT) )
                    CoerceCenter = fsm_union(fsm_copy(rules->left),fsm_copy(rules->right));
                else if ((rules->arrow_type & ARROW_RIGHT) != 0)
                    CoerceCenter = fsm_copy(rules->left);
                else if ((rules->arrow_type & ARROW_LEFT) != 0) {
                    CoerceCenter = fsm_copy(rules->right);
                }
                //printf("Coercing\n");
                /* Can't coerce empty string */
                CoerceCenter = fsm_minus(CoerceCenter, fsm_empty_string());
                
                /* ~[  [?* -[?* %>]] Upper(L) C [Upper(L) ?* & OSR ?*] ?*]; */
                /* TODO: Try replacing this with logic version for efficiency */
                if ((rules->arrow_type & ARROW_OPTIONAL) == 0) {

                    if ((rules->arrow_type & ARROW_DOTTED) == 0) {
                        thisCoerce = fsm_complement(fsm_concat(fsm_minus(fsm_universal(), fsm_concat(fsm_universal(), fsm_symbol("@>@"))),fsm_concat(fsm_copy(contexts->cpleft),fsm_concat(fsm_copy(CoerceCenter),fsm_concat(fsm_intersect(fsm_concat(fsm_copy(contexts->cpright),fsm_universal()),fsm_concat(fsm_copy(Outside),fsm_universal())),fsm_universal())))));
                        
                    } else {
                        //printf("Coercing dotted\n");
                        /* It's the empty part of a [..] dotted rule */
                        SigL = fsm_concat(fsm_term_negation(fsm_symbol("@]@")),fsm_symbol("@]@"));
                        SigR = fsm_concat(fsm_symbol("@[@"),fsm_term_negation(fsm_symbol("@[@")));
                        thisCoerce = fsm_complement(fsm_concat(fsm_intersect(fsm_copy(EndOutside),fsm_intersect(fsm_concat(fsm_universal(),fsm_copy(contexts->cpleft)),fsm_concat(fsm_universal(),fsm_union(fsm_copy(Id),SigL)))),fsm_intersect(fsm_concat(fsm_union(fsm_copy(Id),SigR),fsm_universal()),fsm_concat(fsm_copy(contexts->cpright),fsm_universal()))));
                    }
                }
                if ((rules->arrow_type & ARROW_OPTIONAL) != 0) {
                    thisCoerce = fsm_universal();
                }
                if ((rules->arrow_type & ARROW_LEFT_TO_RIGHT) != 0) {
                    /* LR ~[[[EndOutside] & [?* L/Upp2]] [ [A/Low2 - [%[ ?*] ] & $%[  ] R/Upp2 ?*] */
                    //printf("Adding LR\n"); fflush(stdout);
                    CoerceLR = fsm_complement(fsm_concat(fsm_intersect(fsm_copy(EndOutside),fsm_concat(fsm_universal(),fsm_copy(contexts->cpleft))),fsm_concat(fsm_intersect(fsm_minus(rewr_iglow(fsm_copy(CoerceCenter)), fsm_concat(fsm_symbol("@[@"),fsm_universal())),fsm_contains(fsm_concat(fsm_symbol("@[@"),rewr_iglow(fsm_copy(NoSpecial))))),fsm_concat(fsm_copy(contexts->cpright),fsm_universal()))));
                    
                    thisCoerce = fsm_intersect(thisCoerce, CoerceLR);
                }
                /* LM = ~$[ Dir(L) [ %[ [ A/Low2 - [ %[ ?* | ?* %] ] ] & $[ %] NoSpecial/Low2 ] ] Dir(R) ] */
                if ((rules->arrow_type & ARROW_LONGEST_MATCH) != 0) {
                    //printf("Adding LM\n");


                    /* For single rule (don't look at left context) */
                    /* LM = ~$[ Dir(L) [ %[ [ A/Low2 - [ %[ ?* | ?* %] ] ] & $[ %] NoSpecial/Low2 ] ] Dir(R) ] */

                    if (num_parallel_rules == 1)
                        CoerceLM = fsm_complement(fsm_contains(fsm_concat(fsm_empty_string(),fsm_concat(fsm_concat(fsm_symbol("@[@"),fsm_intersect(fsm_minus(rewr_iglow(fsm_copy(CoerceCenter)),fsm_union(fsm_concat(fsm_symbol("@[@"),fsm_universal()),fsm_concat(fsm_universal(),fsm_symbol("@]@")))),fsm_contains(fsm_concat(fsm_symbol("@]@"),rewr_iglow(fsm_copy(NoSpecial)))))),fsm_copy(contexts->cpright)))));
                    else
                        CoerceLM = fsm_complement(fsm_contains(fsm_concat(fsm_copy(contexts->cpleft),fsm_concat(fsm_concat(fsm_symbol("@[@"),fsm_intersect(fsm_minus(rewr_iglow(fsm_copy(CoerceCenter)),fsm_union(fsm_concat(fsm_symbol("@[@"),fsm_universal()),fsm_concat(fsm_universal(),fsm_symbol("@]@")))),fsm_contains(fsm_concat(fsm_symbol("@]@"),rewr_iglow(fsm_copy(NoSpecial)))))),fsm_copy(contexts->cpright)))));
                    
                    thisCoerce = fsm_intersect(thisCoerce, CoerceLM);
                }
                if ((rules->arrow_type & ARROW_SHORTEST_MATCH) != 0) {
                    /* ~$[L/Low2 %[ C/Low1  [NoSpecial/Low1 ?* & R/Low2 ?*]] */
                    
                    CoerceSM = fsm_complement(fsm_contains(fsm_concat(fsm_copy(contexts->cpleft),fsm_concat(fsm_symbol("@[@"),fsm_concat(rewr_iglow_inside(fsm_copy(CoerceCenter)),fsm_intersect(fsm_concat(rewr_iglow_inside(fsm_copy(NoSpecial)),fsm_universal()),fsm_concat(fsm_copy(contexts->cpright),fsm_universal())))))));

                    thisCoerce = fsm_intersect(thisCoerce, CoerceSM);
                }
                if (Coerce != NULL) {
                    Coerce = fsm_intersect(Coerce, thisCoerce);
                } else {
                    Coerce = thisCoerce;
                }           
		fsm_destroy(CoerceCenter);
	    }
        }
    }
    fsm_destroy(NoSpecial);
    fsm_destroy(Id);
    fsm_destroy(Outside);
    fsm_destroy(EndOutside);
    fsm_destroy(Insert);

    //printf("Have result\n"); fflush(stdout); 
    //Result = fsm_intersect(Insert,fsm_intersect(Context,Coerce));
    Result = fsm_intersect(Context,Coerce);
    Result = fsm_substitute_symbol(Result, "@#@", "@_EPSILON_SYMBOL_@");
    Result = fsm_substitute_symbol(Result, "@<@", "@_EPSILON_SYMBOL_@");
    Result = fsm_minimize(Result);
    Result = rewrite_cp_to_fst(fsm_minimize(Result), "@>@", "@Z@");
    Result = fsm_substitute_symbol(Result, "@[@", "@_EPSILON_SYMBOL_@");
    Result = fsm_substitute_symbol(Result, "@]@", "@_EPSILON_SYMBOL_@");
    sigma_remove("@>@", Result->sigma);
    sigma_remove("@Z@", Result->sigma);
    // sigma_sort(Result);
    Result = fsm_minimize(Result);
    fsm_compact(Result);
    sigma_sort(Result);
    fsm_clear_contexts(allcontexts);
    fsm_destroy(UnionCP);
    return(Result);
}

struct fsm *rewr_context_restrict(struct fsm *X, struct fsmcontexts *LR) {

    struct fsm *Var, *Notvar, *UnionL, *UnionP, *Result;
    struct fsmcontexts *pairs;

    Var = fsm_symbol("@VARX@");
    Notvar = fsm_minimize(fsm_kleene_star(fsm_term_negation(fsm_symbol("@VARX@"))));

    /* We add the variable symbol to all alphabets to avoid ? mathing it */
    /* which would cause extra nondeterminism */

    /* Also, if any L or R is undeclared we add 0 */
    for (pairs = LR; pairs != NULL; pairs = pairs->next) {
        if (pairs->left == NULL) {
            pairs->left = fsm_empty_string();
        } else {            
            sigma_add("@VARX@",pairs->left->sigma);
            sigma_sort(pairs->left);
        }
        if (pairs->right == NULL) {
            pairs->right = fsm_empty_string();
            
        } else {
            sigma_add("@VARX@",pairs->right->sigma);
            sigma_sort(pairs->right);
        }
    }

    UnionP = fsm_empty_set();

    for (pairs = LR; pairs != NULL ; pairs = pairs->next) {
        UnionP = fsm_minimize(fsm_union(fsm_minimize(fsm_concat(fsm_copy(pairs->left),fsm_concat(fsm_copy(Var),fsm_concat(fsm_copy(Notvar),fsm_concat(fsm_copy(Var),fsm_copy(pairs->right)))))), UnionP));
    }
    
    UnionL = fsm_minimize(fsm_concat(fsm_copy(Notvar),fsm_concat(fsm_copy(Var), fsm_concat(fsm_copy(X), fsm_concat(fsm_copy(Var),fsm_copy(Notvar))))));

    Result = fsm_intersect(UnionL,fsm_complement(fsm_concat(fsm_minus(fsm_copy(Notvar),fsm_concat(fsm_universal(),fsm_symbol("@>@"))),fsm_minimize(fsm_concat(fsm_copy(UnionP),fsm_copy(Notvar))))));

    if (sigma_find("@VARX@", Result->sigma) != -1) {
        Result = fsm_complement(fsm_substitute_symbol(Result, "@VARX@","@_EPSILON_SYMBOL_@"));
    } else {    
        Result = fsm_complement(Result);
    }
    fsm_destroy(UnionP);
    fsm_destroy(Var);
    fsm_destroy(Notvar);
    fsm_destroy(X);
    return(Result);
}

struct fsm *fsm_context_restrict(struct fsm *X, struct fsmcontexts *LR) {

    struct fsm *Var, *Notvar, *UnionL, *UnionP, *Result, *Word;
    struct fsmcontexts *pairs;

    /* [.#. \.#.* .#.]-`[[ [\X* X C X \X*]&~[\X* [L1 X \X* X R1|...|Ln X \X* X Rn] \X*]],X,0] */
    /* Where X = variable symbol */
    /* The above only works if we do the subtraction iff the right hand side contains .#. in */
    /* its alphabet */
    /* A more generic formula is the following: */

    /* `[[[(?) \.#.* (?)] - `[[[\X* X C X \X*] - [\X* [L1 X \X* X R1|...|Ln X \X* X Rn] \X*] ],X,0],.#.,0]; */
    /* Here, the LHS is another way of saying ~[?+ .#. ?+] */

    Var = fsm_symbol("@VARX@");
    Notvar = fsm_minimize(fsm_kleene_star(fsm_term_negation(fsm_symbol("@VARX@"))));

    /* We add the variable symbol to all alphabets to avoid ? mathing it */
    /* which would cause extra nondeterminism */
    sigma_add("@VARX@",X->sigma);
    sigma_sort(X);
    
    /* Also, if any L or R is undeclared we add 0 */
    for (pairs = LR; pairs != NULL; pairs = pairs->next) {
        if (pairs->left == NULL) {
            pairs->left = fsm_empty_string();
        } else {
            sigma_add("@VARX@",pairs->left->sigma);
	    sigma_substitute(".#.", "@#@", pairs->left->sigma);
            sigma_sort(pairs->left);
        }
        if (pairs->right == NULL) {
            pairs->right = fsm_empty_string();
        } else {
            sigma_add("@VARX@",pairs->right->sigma);
	    sigma_substitute(".#.", "@#@", pairs->right->sigma);
            sigma_sort(pairs->right);
        }
    }

    UnionP = fsm_empty_set();
    
    for (pairs = LR; pairs != NULL ; pairs = pairs->next) {
        UnionP = fsm_minimize(fsm_union(fsm_minimize(fsm_concat(fsm_copy(pairs->left),fsm_concat(fsm_copy(Var),fsm_concat(fsm_copy(Notvar),fsm_concat(fsm_copy(Var),fsm_copy(pairs->right)))))), UnionP));
    }
    
    UnionL = fsm_minimize(fsm_concat(fsm_copy(Notvar),fsm_concat(fsm_copy(Var), fsm_concat(fsm_copy(X), fsm_concat(fsm_copy(Var),fsm_copy(Notvar))))));

    Result = fsm_intersect(UnionL, fsm_complement(fsm_concat(fsm_copy(Notvar),fsm_minimize(fsm_concat(fsm_copy(UnionP),fsm_copy(Notvar))))));
    if (sigma_find("@VARX@", Result->sigma) != -1) {
        Result = fsm_complement(fsm_substitute_symbol(Result, "@VARX@","@_EPSILON_SYMBOL_@"));
    } else {
	Result = fsm_complement(Result);
    }

    if (sigma_find("@#@", Result->sigma) != -1) {
	Word = fsm_minimize(fsm_concat(fsm_symbol("@#@"),fsm_concat(fsm_kleene_star(fsm_term_negation(fsm_symbol("@#@"))),fsm_symbol("@#@"))));
        Result = fsm_intersect(Word, Result);
        Result = fsm_substitute_symbol(Result, "@#@", "@_EPSILON_SYMBOL_@");
    }
    fsm_destroy(UnionP);
    fsm_destroy(Var);
    fsm_destroy(Notvar);
    fsm_destroy(X);
    fsm_clear_contexts(pairs);
    return(Result);
}

void fsm_clear_contexts(struct fsmcontexts *contexts) {
    struct fsmcontexts *c, *cp;
    for (c = contexts; c != NULL; c = cp) {
	fsm_destroy(c->left);
	fsm_destroy(c->right);
	fsm_destroy(c->cpleft);
	fsm_destroy(c->cpright);
	cp = c->next;
	xxfree(c);
    }
}

struct fsm *rewrite_pad(struct fsm *net) {
    struct fsm *pad;
    /* [? 0:"@>@" ? 0:"@<@"]* [? 0:"@>@" ?] */
    pad = fsm_minimize(fsm_parse_regex("[? 0:\"@>@\" ? 0:\"@<@\"]* [? 0:\"@>@\" ?]", NULL, NULL));
    return(fsm_minimize(fsm_lower(fsm_compose(net,pad))));
}

/** Takes two languages and produces the cross-product on one tape with marker symbols */
/** e.g. CP(a b c, d e) -> a@>@d@<@b@>@e@<@c@>@@Z@, where @Z@ is a "hard zero" symbol. */

struct fsm *rewrite_cp(struct fsm *U, struct fsm *L) {

    struct fsm *PadU, *PadL, *One, *Two, *Three, *Result;

    PadU = fsm_create("");
    PadL = fsm_create("");

    PadU->states = xxmalloc(sizeof(struct fsm_state)*5);
    PadL->states = xxmalloc(sizeof(struct fsm_state)*5);
    
    sigma_add("@<@",PadU->sigma); /* 3 */
    sigma_add("@>@",PadU->sigma); /* 4 */
    sigma_add("@<@",PadL->sigma); /* 3 */
    sigma_add("@>@",PadL->sigma); /* 4 */

    sigma_add_special(IDENTITY,PadL->sigma);
    sigma_add_special(EPSILON,PadL->sigma);
    sigma_add_special(UNKNOWN,PadL->sigma);

    sigma_add_special(IDENTITY,PadU->sigma); 
    sigma_add_special(EPSILON,PadU->sigma);
    sigma_add_special(UNKNOWN,PadU->sigma);

    /* define PadU [NN 0:%> 0:NN 0:%<]* [NN 0:%> 0:NN]; */
    add_fsm_arc(PadU->states,0, 0,IDENTITY,IDENTITY,1,1,1);
    add_fsm_arc(PadU->states,1, 1,EPSILON,4,2,0,0);
    add_fsm_arc(PadU->states,2, 2,EPSILON,UNKNOWN,3,0,0);
    add_fsm_arc(PadU->states,3, 3,EPSILON,3,0,1,0);
    add_fsm_arc(PadU->states,4, -1,-1,-1,-1,-1,-1);

    /* define PadL [0:NN 0:%> NN 0:%<]* [0:NN 0:%> NN] */
    add_fsm_arc(PadL->states,0, 0,EPSILON,UNKNOWN,1,1,1);
    add_fsm_arc(PadL->states,1, 1,EPSILON,4,2,0,0);
    add_fsm_arc(PadL->states,2, 2,IDENTITY,IDENTITY,3,0,0);
    add_fsm_arc(PadL->states,3, 3,EPSILON,3,0,1,0);
    add_fsm_arc(PadL->states,4, -1,-1,-1,-1,-1,-1);

    sigma_sort(PadL);
    sigma_sort(PadU);

    PadL->statecount = PadU->statecount = 4;
    PadL->pathcount  = PadU->pathcount = -1;
    PadL->linecount  = PadU->linecount = 5;

    fsm_update_flags(PadU, YES, YES, YES, YES, NO, NO);
    fsm_update_flags(PadL, YES, YES, YES, YES, NO, NO);

    /* Result = [U "@Z@"* .o. PadU].l & [L "@Z@"* .o. PadL].l & ~$["@Z@" "@>@" "@Z@"] */;
    One = fsm_lower(fsm_compose(fsm_concat(fsm_copy(U),fsm_kleene_star(fsm_symbol("@Z@"))),PadU));
    Two = fsm_lower(fsm_compose(fsm_concat(fsm_copy(L),fsm_kleene_star(fsm_symbol("@Z@"))),PadL));

    Three = fsm_complement(fsm_contains(fsm_concat(fsm_symbol("@Z@"),fsm_concat(fsm_symbol("@>@"),fsm_symbol("@Z@")))));

    Result = fsm_minimize(fsm_intersect(fsm_intersect(One,Two),Three));
    Result = fsm_intersect(Result, fsm_complement(fsm_concat(fsm_universal(),fsm_symbol("@<@"))));
    fsm_destroy(U);
    fsm_destroy(L);
    return(Result);
}

/* Take a simple FSM and convert it to a transducer where any */
/* symbol followed by the lower_symbol is part of a a:b arc */
/* e.g. a>ba>c0>d -> a:b a:c 0:d , where 0 in the FSM is the hard zero */

struct fsm *rewrite_cp_to_fst(struct fsm *net, char *lower_symbol, char *zero_symbol) {
    struct state_link {
        int in;
        int out; 
        struct fsm_state *outptr; /* This points to where the target state is */
        _Bool crowded;
    } *state_link;
    
    int i, j, t, lower, zero, *targets, linecount, in, out, has_unk;
    _Bool *useless_states;
    struct fsm_state *fsm, *fsm2, *tempfsm;

    has_unk = 0;

    if ((lower = sigma_find(lower_symbol, net->sigma)) == -1)
        return(net);
    if ((zero = sigma_find(zero_symbol, net->sigma)) == -1)
        zero = 0;
    state_link = xxmalloc(sizeof(struct state_link) * net->statecount);
    useless_states = xxcalloc(net->statecount, sizeof(_Bool));
    targets = xxmalloc(net->statecount * sizeof(int));
    fsm = net->states;
    for (i=0; i < net->statecount ; i++) {
        (state_link+i)->in = -1;
        (state_link+i)->out = -1;
        (state_link+i)->crowded = 0;
        (state_link+i)->outptr = NULL;
        *(targets+i) = -1;
    }
    
    /* Mark states that have > symbol outgoing (U states) */
    for (i=0, linecount = 0; (fsm+i)->state_no != -1; i++) {
        linecount++;
        if ((fsm+i)->in == lower) {
            (state_link+(fsm+i)->state_no)->out = (fsm+i)->target;
            /* Mark both source and target useless (to be deleted) */
            *(targets+(fsm+i)->target) = (fsm+i)->state_no;
            *(useless_states+(fsm+i)->state_no) = 1;
            *(useless_states+(fsm+i)->target) = 1;
        }
    }
    /* Mark the states that target the U states */
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1 && (state_link+(fsm+i)->target)->out != -1) {
            (state_link+(fsm+i)->state_no)->in = (fsm+i)->state_no;
        }
    }
    /* Store a ptr to the first line of the target states in the lookup table */
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((state_link+(fsm+i)->state_no)->out != -1 && (fsm+i)->in != lower) {
            printf("**Warning, mediator state is crowded!\n");
            (state_link+(fsm+i)->state_no)->crowded = 1;
            *(useless_states+(fsm+i)->state_no) = 0;
        }
        /* Is (fsm+i)->state_no a target? */
        if ((t = *(targets+(fsm+i)->state_no)) != -1) {
        /* Yes, whose target */

            if ((state_link+t)->outptr == NULL) {
                (state_link+t)->outptr = (fsm+i);            
            }
        }
    }

    /* If there is an arc to a U state, do the cross-product */
    /* in-out pairings for all arcs outgoing from the associated out state */

    fsm2 = xxmalloc(sizeof(struct fsm)*linecount);

    for (i=0,j=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1 && (state_link+(fsm+i)->target)->out != -1) {
            for (tempfsm=(state_link+(fsm+i)->target)->outptr,t=tempfsm->state_no;tempfsm->state_no == t ;tempfsm++) {               
                in = (fsm+i)->in;
                out = tempfsm->out;
                if (in == IDENTITY || out == IDENTITY) {
                    has_unk = 1;
                }

                if (in == IDENTITY && out == IDENTITY) {
                    add_fsm_arc(fsm2,j, (fsm+i)->state_no, in, out, tempfsm->target, (fsm+i)->final_state, (fsm+i)->start_state);
                    j++;
                }
                in = in == IDENTITY ? UNKNOWN : in;
                out = out == IDENTITY ? UNKNOWN : out;
                add_fsm_arc(fsm2,j, (fsm+i)->state_no, in, out, tempfsm->target, (fsm+i)->final_state, (fsm+i)->start_state);
                j++;
            }
            if ((state_link+(fsm+i)->target)->crowded) {
                add_fsm_arc(fsm2,j,(fsm+i)->state_no,(fsm+i)->in,(fsm+i)->out,(fsm+i)->target,(fsm+i)->final_state,(fsm+i)->start_state);
                j++;
            }
        } else if (!*(useless_states+(fsm+i)->state_no) && (fsm+i)->in != lower) {
            add_fsm_arc(fsm2,j,(fsm+i)->state_no,(fsm+i)->in,(fsm+i)->out,(fsm+i)->target,(fsm+i)->final_state,(fsm+i)->start_state);
            j++;
        }
    }

    add_fsm_arc(fsm2, j, -1, -1, -1, -1, -1, -1);

    /* Renumber states, store new number in *(targets+x) */
    /* Also, replace epsilons */

    for (i=0, j=0; i<net->statecount;i++) {
        *(targets+i) = j;
        if (*(useless_states+i) != 1) {
            j++;
        }
    }
    for (i=0; (fsm2+i)->state_no != -1 ; i++) {
        (fsm2+i)->state_no = *(targets+(fsm2+i)->state_no);
        if ((fsm2+i)->target != -1) {
            (fsm2+i)->target = *(targets+(fsm2+i)->target);
        }
        if ((fsm2+i)->in == zero)
            (fsm2+i)->in = EPSILON;
        if ((fsm2+i)->out == zero)
            (fsm2+i)->out = EPSILON;
        
    }

    if (has_unk == 1 && sigma_find_number(UNKNOWN, net->sigma) == -1) {
        sigma_add_special(UNKNOWN, net->sigma);
    }
    
    net->states = fsm2;
    xxfree(fsm);
    xxfree(useless_states);
    xxfree(targets);
    xxfree(state_link);
    return(net);
}
