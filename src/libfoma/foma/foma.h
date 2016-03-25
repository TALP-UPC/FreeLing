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

#include "fomalib.h"

#define AP_D 1 /* Apply down */
#define AP_U 2 /* Apply up   */
#define AP_M 3 /* Apply minimum edit distance */

#define PROMPT_MAIN 0 /* Regular prompt */
#define PROMPT_A 1    /* Apply prompt   */

struct defined_networks   *g_defines;
struct defined_functions  *g_defines_f;

/** User stack */
struct stack_entry {
  int number;
  struct apply_handle *ah;
  struct apply_med_handle *amedh;
  struct fsm *fsm;
  struct stack_entry *next;
  struct stack_entry *previous;    
};

/* Quantifier & Logic-related */
char *find_quantifier(char *string);
void add_quantifier (char *string);
void purge_quantifier (char *string);
struct fsm *union_quantifiers();
int count_quantifiers();
void clear_quantifiers();

/* Main Stack functions */
int stack_add(struct fsm *fsm);
int stack_size();
int stack_init();
struct fsm *stack_pop();
int stack_isempty();
int stack_turn();
struct stack_entry *stack_find_top();
struct stack_entry *stack_find_second();
struct stack_entry *stack_find_bottom();
int stack_clear();
int stack_rotate();
int stack_print();
struct apply_handle *stack_get_ah();
struct apply_med_handle *stack_get_med_ah();

/* Iface */
void iface_ambiguous_upper(void);
void iface_apply_down(char *word);
int iface_apply_file(char *infilename, char *outfilename, int direction);
void iface_apply_med(char *word);
void iface_apply_set_params(struct apply_handle *h);
void iface_apply_up(char *word);
void iface_apropos(char *s);
void iface_close(void);
void iface_compact(void);
void iface_complete(void);
void iface_compose(void);
void iface_conc(void);
void iface_crossproduct(void);
void iface_determinize(void);
void iface_eliminate_flags(void);
void iface_eliminate_flag(char *name);
int  iface_extract_number(char *s);
void iface_extract_ambiguous(void);
void iface_extract_unambiguous(void);
void iface_factorize(void);
void iface_help_search(char *s);
void iface_help(void);
void iface_ignore(void);
void iface_intersect(void);
void iface_invert(void);
void iface_load_defined(char *filename);
void iface_load_stack(char *filename);
void iface_lower_side(void);
void iface_minimize(void);
void iface_one_plus(void);
void iface_pop(void);
void iface_label_net(void);
void iface_letter_machine(void);
void iface_lower_words(int limit);
void iface_name_net(char *name);
void iface_negate(void);
void iface_print_cmatrix(void);
void iface_print_cmatrix_att(char *filename);
void iface_print_net(char *netname, char *filename);
void iface_print_defined(void);
void iface_print_dot(char *filename);
void iface_print_shortest_string();
void iface_print_shortest_string_size();
void iface_print_name(void);
void iface_quit(void);
void iface_apply_random(char *(*applyer)(), int limit);
void iface_random_lower(int limit);
void iface_random_upper(int limit);
void iface_random_words(int limit);
void iface_pairs(int limit);
void iface_pairs_file(char *filename);
void iface_random_pairs(int limit);
void iface_print_sigma(void);
void iface_print_stats(void);
void iface_shuffle(void);
void iface_sort(void);
void iface_sort_input(void);
void iface_sort_output(void);
int  iface_stack_check(int size);
void iface_upper_words(int limit);
void iface_prune(void);
int  iface_read_att(char *filename);
int  iface_read_prolog(char *filename);
int  iface_read_spaced_text(char *filename);
int  iface_read_text(char *filename);
void iface_reverse(void);
void iface_rotate(void);
void iface_save_defined(char *filename);
void iface_save_stack(char *filename);
void iface_sequentialize(void);
void iface_set_variable(char *name, char *value);
void iface_show_variables(void);
void iface_show_variable(char *name);
void iface_sigma_net();
void iface_substitute_defined (char *original, char *substitute);
void iface_substitute_symbol (char *original, char *substitute);
void iface_test_equivalent(void);
void iface_test_functional(void);
void iface_test_identity(void);
void iface_test_lower_universal(void);
void iface_test_sequential(void);
void iface_test_unambiguous(void);
void iface_test_upper_universal(void);
void iface_test_nonnull(void);
void iface_test_null(void);
void iface_turn(void);
void iface_twosided_flags(void);
void iface_union(void);
void iface_upper_side(void);
void iface_view(void);
void iface_warranty(void);
void iface_words(int limit);
void iface_words_file(char *filename, int type);
int iface_write_att(char *filename);
void iface_write_prolog(char *filename);
void iface_zero_plus(void);
int  print_stats(struct fsm *net);
