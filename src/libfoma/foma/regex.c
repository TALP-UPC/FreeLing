/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 18 "regex.y" /* yacc.c:339  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foma.h"
#define MAX_F_RECURSION 100
extern int yyerror();
extern int yylex();
extern int my_yyparse(char *my_string, int lineno, struct defined_networks *defined_nets, struct defined_functions *defined_funcs);
struct fsm *current_parse;
int rewrite, rule_direction;
int substituting = 0;
static char *subval1, *subval2;
struct fsmcontexts *contexts;
struct fsmrules *rules;
struct rewrite_set *rewrite_rules;
static struct fsm *fargs[100][MAX_F_RECURSION];  /* Function arguments [number][frec] */
static int frec = -1;                            /* Current depth of function recursion */
static char *fname[MAX_F_RECURSION];             /* Function names */
static int fargptr[MAX_F_RECURSION];             /* Current argument no. */
/* Variable to produce internal symbols */
unsigned int g_internal_sym = 23482342;

void add_function_argument(struct fsm *net) {
    fargs[fargptr[frec]][frec] = net;
    fargptr[frec]++;
}

void declare_function_name(char *s) {
    if (frec > MAX_F_RECURSION) {
        printf("Function stack depth exceeded. Aborting.\n");
        exit(1);
    }
    fname[frec] = xxstrdup(s);
    xxfree(s);
}

struct fsm *function_apply(struct defined_networks *defined_nets, struct defined_functions *defined_funcs) {
    int i, mygsym, myfargptr;
    char *regex;
    char repstr[13], oldstr[13];
    if ((regex = find_defined_function(defined_funcs, fname[frec],fargptr[frec])) == NULL) {
        fprintf(stderr, "***Error: function %s@%i) not defined!\n",fname[frec], fargptr[frec]);
        return NULL;
    }
    regex = xxstrdup(regex);
    mygsym = g_internal_sym;
    myfargptr = fargptr[frec];
    /* Create new regular expression from function def. */
    /* and parse that */
    for (i = 0; i < fargptr[frec]; i++) {
        sprintf(repstr,"%012X",g_internal_sym);
        sprintf(oldstr, "@ARGUMENT%02i@", (i+1));
        streqrep(regex, oldstr, repstr);
        /* We temporarily define a network and save argument there */
        /* The name is a running counter g_internal_sym */
        add_defined(defined_nets, fargs[i][frec], repstr);
        g_internal_sym++;
    }
    my_yyparse(regex,1,defined_nets, defined_funcs);
    for (i = 0; i < myfargptr; i++) {
        sprintf(repstr,"%012X",mygsym);
        /* Remove the temporarily defined network */
        remove_defined(defined_nets, repstr);
        mygsym++;
    }
    xxfree(fname[frec]);
    frec--;
    xxfree(regex);
    return(current_parse);
}

void add_context_pair(struct fsm *L, struct fsm *R) {
    struct fsmcontexts *newcontext;
    newcontext = xxcalloc(1,sizeof(struct fsmcontexts));
    newcontext->left = L;
    newcontext->right = R;
    newcontext->next = contexts;
    contexts = newcontext;
}

void clear_rewrite_ruleset(struct rewrite_set *rewrite_rules) {
    struct rewrite_set *rule, *rulep;
    struct fsmcontexts *contexts, *contextsp;
    struct fsmrules *r, *rp;
    for (rule = rewrite_rules; rule != NULL; rule = rulep) {

	for (r = rule->rewrite_rules ; r != NULL; r = rp) {
	    fsm_destroy(r->left);
	    fsm_destroy(r->right);
	    fsm_destroy(r->right2);
	    rp = r->next;
	    xxfree(r);
	}
	
	for (contexts = rule->rewrite_contexts; contexts != NULL ; contexts = contextsp) {

	    contextsp = contexts->next;
	    fsm_destroy(contexts->left);
	    fsm_destroy(contexts->right);
	    fsm_destroy(contexts->cpleft);
	    fsm_destroy(contexts->cpright);
	    xxfree(contexts);
	}
       	rulep = rule->next;
	//fsm_destroy(rules->cpunion);
	xxfree(rule);
    }
}

void add_rewrite_rule() {
    struct rewrite_set *new_rewrite_rule;
    if (rules != NULL) {
        new_rewrite_rule = xxmalloc(sizeof(struct rewrite_set));
        new_rewrite_rule->rewrite_rules = rules;
        new_rewrite_rule->rewrite_contexts = contexts;
        new_rewrite_rule->next = rewrite_rules;
        new_rewrite_rule->rule_direction = rule_direction;
        new_rewrite_rule->cpunion = NULL;

        rewrite_rules = new_rewrite_rule;
        rules = NULL;
        contexts = NULL;
        rule_direction = 0;
    }
}

void add_rule(struct fsm *L, struct fsm *R, struct fsm *R2, int type) {
    struct fsm *test;
    struct fsmrules *newrule;
    rewrite = 1;
    newrule = xxmalloc(sizeof(struct fsmrules));

    if ((type & ARROW_DOTTED) != 0) {
        newrule->left = fsm_minus(fsm_copy(L), fsm_empty_string());
    } else {
        newrule->left = L;
    }
    newrule->right = R;
    newrule->right2 = R2;
    newrule->next = rules;
    newrule->arrow_type = type;
    if ((type & ARROW_DOTTED) != 0) {
        newrule->arrow_type = type - ARROW_DOTTED;
    }

    rules = newrule;

    if ((type & ARROW_DOTTED) != 0) {
        /* Add empty [..] -> B for dotted rules (only if LHS contains the empty string) */
        test = fsm_intersect(L,fsm_empty_string());
        if (!fsm_isempty(test)) {
            newrule = xxmalloc(sizeof(struct fsmrules));
            newrule->left = test;
            newrule->right = fsm_copy(R);
            newrule->right2 = fsm_copy(R2);
            newrule->next = rules;
            newrule->arrow_type = type;
            rules = newrule;
        } else {
	    //fsm_destroy(test);
	}
    }
}



#line 234 "regex.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "regex.h".  */
#ifndef YY_YY_REGEX_H_INCLUDED
# define YY_YY_REGEX_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NET = 258,
    END = 259,
    LBRACKET = 260,
    RBRACKET = 261,
    LPAREN = 262,
    RPAREN = 263,
    ENDM = 264,
    ENDD = 265,
    CRESTRICT = 266,
    CONTAINS = 267,
    CONTAINS_OPT_ONE = 268,
    CONTAINS_ONE = 269,
    XUPPER = 270,
    XLOWER = 271,
    FLAG_ELIMINATE = 272,
    IGNORE_ALL = 273,
    IGNORE_INTERNAL = 274,
    CONTEXT = 275,
    NCONCAT = 276,
    MNCONCAT = 277,
    MORENCONCAT = 278,
    LESSNCONCAT = 279,
    DOUBLE_COMMA = 280,
    COMMA = 281,
    SHUFFLE = 282,
    PRECEDES = 283,
    FOLLOWS = 284,
    RIGHT_QUOTIENT = 285,
    LEFT_QUOTIENT = 286,
    INTERLEAVE_QUOTIENT = 287,
    UQUANT = 288,
    EQUANT = 289,
    VAR = 290,
    IN = 291,
    IMPLIES = 292,
    BICOND = 293,
    EQUALS = 294,
    NEQ = 295,
    SUBSTITUTE = 296,
    SUCCESSOR_OF = 297,
    PRIORITY_UNION_U = 298,
    PRIORITY_UNION_L = 299,
    LENIENT_COMPOSE = 300,
    TRIPLE_DOT = 301,
    LDOT = 302,
    RDOT = 303,
    FUNCTION = 304,
    SUBVAL = 305,
    ISUNAMBIGUOUS = 306,
    ISIDENTITY = 307,
    ISFUNCTIONAL = 308,
    NOTID = 309,
    LOWERUNIQ = 310,
    LOWERUNIQEPS = 311,
    ALLFINAL = 312,
    UNAMBIGUOUSPART = 313,
    AMBIGUOUSPART = 314,
    AMBIGUOUSDOMAIN = 315,
    EQSUBSTRINGS = 316,
    LETTERMACHINE = 317,
    MARKFSMTAIL = 318,
    MARKFSMTAILLOOP = 319,
    MARKFSMMIDLOOP = 320,
    MARKFSMLOOP = 321,
    ADDSINK = 322,
    LEFTREWR = 323,
    FLATTEN = 324,
    SUBLABEL = 325,
    CLOSESIGMA = 326,
    CLOSESIGMAUNK = 327,
    ARROW = 328,
    DIRECTION = 329,
    COMPOSE = 330,
    CROSS_PRODUCT = 331,
    HIGH_CROSS_PRODUCT = 332,
    UNION = 333,
    INTERSECT = 334,
    MINUS = 335,
    COMPLEMENT = 336,
    KLEENE_STAR = 337,
    KLEENE_PLUS = 338,
    REVERSE = 339,
    INVERSE = 340,
    TERM_NEGATION = 341
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 186 "regex.y" /* yacc.c:355  */

     char *string;
     struct fsm *net;
     int  type;

#line 367 "regex.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs);

#endif /* !YY_YY_REGEX_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 397 "regex.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  105
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1180

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  87
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  136
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  291

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   341

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   240,   240,   241,   244,   246,   247,   248,   249,   251,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   279,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   295,   296,   297,
     298,   300,   302,   304,   305,   306,   307,   308,   309,   310,
     311,   313,   314,   315,   317,   318,   319,   320,   322,   323,
     324,   325,   326,   327,   329,   330,   331,   332,   333,   335,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   347,
     348,   349,   350,   352,   353,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   366,   367,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   390,   391,
     393,   395,   398,   399,   401,   403,   405
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NET", "END", "LBRACKET", "RBRACKET",
  "LPAREN", "RPAREN", "ENDM", "ENDD", "CRESTRICT", "CONTAINS",
  "CONTAINS_OPT_ONE", "CONTAINS_ONE", "XUPPER", "XLOWER", "FLAG_ELIMINATE",
  "IGNORE_ALL", "IGNORE_INTERNAL", "CONTEXT", "NCONCAT", "MNCONCAT",
  "MORENCONCAT", "LESSNCONCAT", "DOUBLE_COMMA", "COMMA", "SHUFFLE",
  "PRECEDES", "FOLLOWS", "RIGHT_QUOTIENT", "LEFT_QUOTIENT",
  "INTERLEAVE_QUOTIENT", "UQUANT", "EQUANT", "VAR", "IN", "IMPLIES",
  "BICOND", "EQUALS", "NEQ", "SUBSTITUTE", "SUCCESSOR_OF",
  "PRIORITY_UNION_U", "PRIORITY_UNION_L", "LENIENT_COMPOSE", "TRIPLE_DOT",
  "LDOT", "RDOT", "FUNCTION", "SUBVAL", "ISUNAMBIGUOUS", "ISIDENTITY",
  "ISFUNCTIONAL", "NOTID", "LOWERUNIQ", "LOWERUNIQEPS", "ALLFINAL",
  "UNAMBIGUOUSPART", "AMBIGUOUSPART", "AMBIGUOUSDOMAIN", "EQSUBSTRINGS",
  "LETTERMACHINE", "MARKFSMTAIL", "MARKFSMTAILLOOP", "MARKFSMMIDLOOP",
  "MARKFSMLOOP", "ADDSINK", "LEFTREWR", "FLATTEN", "SUBLABEL",
  "CLOSESIGMA", "CLOSESIGMAUNK", "ARROW", "DIRECTION", "COMPOSE",
  "CROSS_PRODUCT", "HIGH_CROSS_PRODUCT", "UNION", "INTERSECT", "MINUS",
  "COMPLEMENT", "KLEENE_STAR", "KLEENE_PLUS", "REVERSE", "INVERSE",
  "TERM_NEGATION", "$accept", "start", "regex", "network", "networkA",
  "n0", "network1", "network2", "network3", "network4", "network5",
  "network6", "network7", "network8", "network9", "network10", "network11",
  "sub1", "sub2", "network12", "fstart", "fmid", "fend", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341
};
# endif

#define YYPACT_NINF -42

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-42)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     750,   -42,   750,   750,    68,    68,    68,   330,    -3,   750,
     459,     0,   820,   610,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,    68,  1001,    10,
     750,     8,   -42,    25,    15,   -42,   -42,  1100,    68,   266,
     -42,   -42,   206,   -42,   -42,   -13,   -42,   680,   750,   -42,
      66,   142,   -42,   -42,   -42,   750,   750,    43,   750,   -36,
       3,    14,   960,    17,    20,   750,   669,   160,   -12,  1069,
     171,   203,   263,   291,   361,   370,   431,   440,   501,   510,
     571,   174,   580,   893,  1048,  1053,  1056,   641,  1058,  1060,
    1064,   711,   720,   -42,   -42,   -42,   -42,   -42,   750,   750,
     750,   750,   400,   750,   260,   960,   960,   960,   960,   960,
     960,   960,   960,   960,   960,   266,    68,    68,    68,    68,
      68,   -42,   -42,   -42,   -42,   -42,   -42,   -42,  1001,   -42,
     -42,   -42,   -42,    36,   -42,   -42,   149,   165,   -42,   -42,
      25,    25,   750,   781,   -42,   -42,    68,   -42,   -42,  1073,
     890,    29,   750,    35,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   750,   -42,   750,   750,   750,
     750,   -42,   750,   750,   750,   -42,   -42,   -42,   -42,   -42,
      25,   750,   750,   350,    25,   750,   470,   750,    40,   -42,
     -42,   -42,    68,    68,    68,    68,    68,    68,    68,   -42,
     -42,   -42,   -42,   -42,   -42,    62,   -42,   -42,   -42,   -42,
      25,   -42,   -42,  1133,   790,   110,    74,   750,  1076,   851,
     860,   921,   962,  1002,  1031,  1082,    25,    25,   750,    25,
     750,   750,   181,    25,   750,   540,   750,    64,   -42,   -42,
     -42,   750,   750,   231,   750,   -42,   -42,   -42,   -42,   -42,
     -42,   750,    25,    25,    25,   750,   750,    25,   750,   750,
     348,    25,   -42,    25,    25,   750,   750,  1036,  1040,    25,
      25,    25,    25,   750,   750,    25,    25,   -42,   -42,    25,
      25
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    95,     0,     0,     0,     0,     0,    14,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     5,     9,    10,    47,    51,    52,    53,    61,
      68,    74,    79,    80,    93,     0,    96,     0,     0,   107,
       0,     0,    76,    78,    77,     0,     0,    18,     0,    98,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    94,     1,     3,     4,     0,     0,
       0,     0,    12,     0,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    62,     0,     0,     0,     0,
       0,    85,    86,    87,    89,    92,    90,    91,     0,    81,
      82,    83,    84,     0,   104,   136,     0,     0,   100,    99,
      15,    17,     0,     0,    66,    67,    63,    64,    65,     0,
       0,     0,     0,     0,   130,   110,   108,   109,   111,   112,
     113,   114,   115,   116,   117,     0,   118,     0,     0,     0,
       0,   123,     0,     0,     0,   127,   128,     7,     6,     8,
      21,     0,     0,    11,    34,     0,    35,     0,    22,    48,
      49,    50,    59,    60,    55,    56,    54,    57,    58,    69,
      70,    71,    72,    73,    88,     0,   135,   132,   134,   133,
      19,    97,   105,     0,     0,     0,    25,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,    13,     0,    31,
       0,     0,    36,    33,     0,    37,     0,     0,   101,   102,
     103,     0,     0,    24,     0,   119,   120,   121,   122,   124,
     125,     0,    20,    42,    46,     0,     0,    30,     0,     0,
      38,    32,   106,    27,    29,     0,     0,     0,     0,    41,
      44,    40,    45,     0,     0,    26,    28,   129,   126,    39,
      43
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -42,   103,   -42,    -1,   -32,    -7,   -42,   -20,   -42,   -42,
     -31,   -41,    42,   -42,   -42,   -35,   -42,   -42,   -42,   -42,
     -42,   -42,   -42
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   144,    56,
      57,    58,    59
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      67,    60,    61,   104,    68,    75,    79,   125,    69,   108,
     105,    77,   107,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   111,   143,   154,   109,
     110,   156,   115,   116,   117,   112,    62,    63,    64,   155,
     113,   111,   157,   108,   111,   158,   146,   147,   150,   151,
     112,   162,   215,   112,   225,   113,   244,   153,   113,   152,
     272,     1,   148,     2,   159,     3,   187,   188,   189,   103,
       4,     5,     6,   109,   110,   111,   245,   202,   203,   204,
     205,   206,   207,   208,   112,   199,   200,   201,   114,   113,
     251,     8,     9,   214,   190,   193,   194,   198,   227,    11,
      12,   108,   247,   114,   246,   125,   114,    14,   250,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,   109,   110,   106,     0,   220,     0,   114,   252,    37,
     149,     0,     0,     0,    38,   226,     0,   216,     0,   224,
       0,   125,   125,   125,   125,   125,   125,   125,   209,   210,
     211,   212,   213,   218,   228,   217,   229,   230,   231,   232,
       0,   233,   234,   235,   236,   237,   161,   108,   239,   242,
     243,   219,   111,     0,   108,     0,     0,   164,     0,     0,
     175,   112,     0,     0,     0,   108,   113,   265,     0,     0,
     108,   165,     0,     0,     0,     0,   108,   109,   110,   108,
     253,   131,   132,   133,   109,   110,     0,   134,   135,   136,
     137,   262,     0,   263,   264,   109,   110,   267,   270,   271,
     109,   110,   111,     0,   273,   274,   109,   110,   108,   109,
     110,   112,     0,   277,   114,   266,   113,   275,   279,   280,
     278,   281,   282,     1,     0,     2,     0,     3,   285,   286,
       0,   166,     4,     5,     6,     0,   289,   290,   109,   110,
       7,     0,     0,   138,   126,   127,   195,     0,   139,   140,
     141,   142,     0,     8,     9,    10,   128,   129,   130,   167,
       0,    11,    12,     0,   114,   276,   196,    13,   108,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,   197,     2,   108,     3,   109,   110,
       0,    37,     4,     5,     6,     0,    38,     0,     0,     0,
       7,     0,     0,     0,     0,    65,    66,     0,     0,   111,
       0,   111,     0,     8,     9,    10,   109,   110,   112,   168,
     112,    11,    12,   113,   283,   113,   238,    13,   169,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,   114,   284,   114,     0,   191,   192,     0,     0,     0,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   170,
       0,    11,    12,     0,     0,   109,   110,    13,   171,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,    70,    71,     0,
       7,     0,     0,     0,     0,    72,   240,     0,    73,    74,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   172,
       0,    11,    12,     0,     0,   109,   110,    13,   173,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,   241,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,     0,     0,     0,     0,     0,   268,     0,     0,     0,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   174,
       0,    11,    12,     0,     0,   109,   110,    13,   176,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,   269,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   181,
       0,    11,    12,     0,     0,   109,   110,    13,    78,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,   145,     0,
       0,    37,     4,     5,     6,   160,    38,    70,    71,     0,
       7,     0,     0,     0,     0,    72,     0,     0,    73,    74,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   185,
       0,    11,    12,     0,     0,     0,     0,    13,   186,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,   109,   110,     0,   221,
       0,    11,    12,     0,     0,   109,   110,    13,   249,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    76,   109,   110,     0,   255,
       0,    11,    12,     0,     0,   109,   110,    13,   256,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,     0,     0,
       0,    37,     4,     5,     6,   108,    38,     0,     0,     0,
       7,     0,     0,     0,     0,     0,     0,     0,     0,   177,
       0,     0,     0,     8,     9,   223,   109,   110,     0,   257,
       0,    11,    12,     0,     0,   109,   110,    13,   108,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     1,     0,     2,   108,     3,   109,   110,
     258,    37,     4,     5,     6,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,   109,   110,     0,     0,
       0,    11,    12,     0,     1,     0,     2,   108,     3,    14,
     259,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,     0,     8,     9,     0,   109,   110,   260,
       0,    37,    11,    12,   287,     0,    38,   108,   288,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,   178,     0,   108,   109,   110,   179,
     111,   108,   180,     0,   182,   108,   183,    38,     0,   112,
     184,     0,     0,   108,   113,     0,     0,     0,   108,   222,
       0,   108,   254,   108,     0,   108,   109,   110,   261,   108,
       0,   109,   110,     0,     0,   109,   110,   163,   108,     0,
       0,   108,     0,   109,   110,     0,     0,   108,   109,   110,
       0,   109,   110,   109,   110,   109,   110,   118,   119,   109,
     110,   248,   114,   120,   121,     0,     0,     0,   109,   110,
       0,   109,   110,     0,     0,     0,     0,   109,   110,     0,
       0,    70,    71,     0,     0,     0,     0,     0,     0,    72,
       0,     0,    73,    74,     0,     0,     0,     0,   122,   123,
     124
};

static const yytype_int16 yycheck[] =
{
       7,     2,     3,    38,     7,     5,    13,    48,     9,    45,
       0,    12,     4,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    11,    50,    35,    75,
      76,    72,    27,    28,    29,    20,     4,     5,     6,    35,
      25,    11,    35,    45,    11,    35,    57,    58,    65,    66,
      20,    73,    26,    20,    35,    25,    26,    68,    25,    26,
       6,     3,     6,     5,    75,     7,   108,   109,   110,    37,
      12,    13,    14,    75,    76,    11,    46,   118,   119,   120,
     121,   122,   123,   124,    20,   115,   116,   117,    73,    25,
      26,    33,    34,   138,   111,   112,   113,   114,    73,    41,
      42,    45,    50,    73,    74,   156,    73,    49,     8,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    75,    76,    40,    -1,   152,    -1,    73,    74,    81,
       8,    -1,    -1,    -1,    86,   162,    -1,     8,    -1,   160,
      -1,   202,   203,   204,   205,   206,   207,   208,   126,   127,
     128,   129,   130,     8,   175,    26,   177,   178,   179,   180,
      -1,   182,   183,   184,   191,   192,    26,    45,   195,   196,
     197,    26,    11,    -1,    45,    -1,    -1,    26,    -1,    -1,
      26,    20,    -1,    -1,    -1,    45,    25,    26,    -1,    -1,
      45,     8,    -1,    -1,    -1,    -1,    45,    75,    76,    45,
     227,    15,    16,    17,    75,    76,    -1,    21,    22,    23,
      24,   238,    -1,   240,   241,    75,    76,   244,   245,   246,
      75,    76,    11,    -1,   251,   252,    75,    76,    45,    75,
      76,    20,    -1,   254,    73,    74,    25,    26,   265,   266,
     261,   268,   269,     3,    -1,     5,    -1,     7,   275,   276,
      -1,     8,    12,    13,    14,    -1,   283,   284,    75,    76,
      20,    -1,    -1,    77,    18,    19,    26,    -1,    82,    83,
      84,    85,    -1,    33,    34,    35,    30,    31,    32,     8,
      -1,    41,    42,    -1,    73,    74,    46,    47,    45,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    74,     5,    45,     7,    75,    76,
      -1,    81,    12,    13,    14,    -1,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    11,
      -1,    11,    -1,    33,    34,    35,    75,    76,    20,     8,
      20,    41,    42,    25,    26,    25,    26,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    73,    74,    73,    -1,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    28,    29,    -1,
      20,    -1,    -1,    -1,    -1,    36,    26,    -1,    39,    40,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    74,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    74,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,     8,    -1,
      -1,    81,    12,    13,    14,    26,    86,    28,    29,    -1,
      20,    -1,    -1,    -1,    -1,    36,    -1,    -1,    39,    40,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    -1,    -1,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,     8,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    -1,    -1,
      -1,    81,    12,    13,    14,    45,    86,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,     8,
      -1,    41,    42,    -1,    -1,    75,    76,    47,    45,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,     3,    -1,     5,    45,     7,    75,    76,
       8,    81,    12,    13,    14,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    75,    76,    -1,    -1,
      -1,    41,    42,    -1,     3,    -1,     5,    45,     7,    49,
       8,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    33,    34,    -1,    75,    76,     8,
      -1,    81,    41,    42,     8,    -1,    86,    45,     8,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    26,    -1,    45,    75,    76,    26,
      11,    45,    26,    -1,    26,    45,    26,    86,    -1,    20,
      26,    -1,    -1,    45,    25,    -1,    -1,    -1,    45,    26,
      -1,    45,    26,    45,    -1,    45,    75,    76,    26,    45,
      -1,    75,    76,    -1,    -1,    75,    76,    48,    45,    -1,
      -1,    45,    -1,    75,    76,    -1,    -1,    45,    75,    76,
      -1,    75,    76,    75,    76,    75,    76,    37,    38,    75,
      76,     8,    73,    43,    44,    -1,    -1,    -1,    75,    76,
      -1,    75,    76,    -1,    -1,    -1,    -1,    75,    76,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    78,    79,
      80
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     5,     7,    12,    13,    14,    20,    33,    34,
      35,    41,    42,    47,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    81,    86,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   106,   107,   108,   109,
      90,    90,    99,    99,    99,    25,    26,    92,     7,    90,
      28,    29,    36,    39,    40,     5,    35,    90,    48,    92,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    99,   102,     0,    88,     4,    45,    75,
      76,    11,    20,    25,    73,    27,    28,    29,    37,    38,
      43,    44,    78,    79,    80,    98,    18,    19,    30,    31,
      32,    15,    16,    17,    21,    22,    23,    24,    77,    82,
      83,    84,    85,    50,   105,     8,    90,    90,     6,     8,
      92,    92,    26,    90,    35,    35,    97,    35,    35,    90,
      26,    26,    73,    48,    26,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,    26,     8,    26,    26,    26,
      26,     8,    26,    26,    26,     8,     8,    91,    91,    91,
      92,    25,    26,    92,    92,    26,    46,    74,    92,    94,
      94,    94,    97,    97,    97,    97,    97,    97,    97,    99,
      99,    99,    99,    99,   102,    26,     8,    26,     8,    26,
      92,     8,    26,    35,    90,    35,    92,    73,    90,    90,
      90,    90,    90,    90,    90,    90,    92,    92,    26,    92,
      26,    74,    92,    92,    26,    46,    74,    50,     8,     8,
       8,    26,    74,    92,    26,     8,     8,     8,     8,     8,
       8,    26,    92,    92,    92,    26,    74,    92,    26,    74,
      92,    92,     6,    92,    92,    26,    74,    90,    90,    92,
      92,    92,    92,    26,    74,    92,    92,     8,     8,    92,
      92
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    87,    88,    88,    89,    90,    90,    90,    90,    91,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    93,    93,    93,
      93,    94,    95,    96,    96,    96,    96,    96,    96,    96,
      96,    97,    97,    97,    97,    97,    97,    97,    98,    98,
      98,    98,    98,    98,    99,    99,    99,    99,    99,   100,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   102,   102,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   104,   105,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   108,   108,   109,   109,   109
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     1,     3,     3,     3,     1,
       1,     3,     2,     4,     1,     3,     4,     3,     2,     4,
       5,     3,     3,     2,     5,     4,     7,     6,     7,     6,
       5,     4,     5,     4,     3,     3,     4,     4,     5,     7,
       6,     6,     5,     7,     6,     6,     5,     1,     3,     3,
       3,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     1,     2,     3,     3,     3,     3,     3,     1,     3,
       3,     3,     3,     3,     1,     2,     2,     2,     2,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     2,     2,     1,     2,     1,     1,     4,     2,     3,
       3,     5,     5,     5,     2,     4,     4,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       5,     5,     5,     3,     5,     5,     7,     3,     3,     7,
       3,     2,     3,     3,     3,     3,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, scanner, defined_nets, defined_funcs, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, scanner, defined_nets, defined_funcs); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (defined_nets);
  YYUSE (defined_funcs);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, defined_nets, defined_funcs);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , scanner, defined_nets, defined_funcs);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, defined_nets, defined_funcs); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (defined_nets);
  YYUSE (defined_funcs);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, struct defined_networks *defined_nets, struct defined_functions *defined_funcs)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

/* User initialization code.  */
#line 199 "regex.y" /* yacc.c:1429  */
{
    clear_quantifiers();
    rewrite = 0;
    contexts = NULL;
    rules = NULL;
    rewrite_rules = NULL;
    rule_direction = 0;
    substituting = 0;
}

#line 1796 "regex.c" /* yacc.c:1429  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 244 "regex.y" /* yacc.c:1646  */
    { current_parse = (yyvsp[-1].net);              }
#line 1985 "regex.c" /* yacc.c:1646  */
    break;

  case 5:
#line 246 "regex.y" /* yacc.c:1646  */
    { }
#line 1991 "regex.c" /* yacc.c:1646  */
    break;

  case 6:
#line 247 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_compose((yyvsp[-2].net),(yyvsp[0].net));         }
#line 1997 "regex.c" /* yacc.c:1646  */
    break;

  case 7:
#line 248 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_lenient_compose((yyvsp[-2].net),(yyvsp[0].net)); }
#line 2003 "regex.c" /* yacc.c:1646  */
    break;

  case 8:
#line 249 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_cross_product((yyvsp[-2].net),(yyvsp[0].net));   }
#line 2009 "regex.c" /* yacc.c:1646  */
    break;

  case 9:
#line 251 "regex.y" /* yacc.c:1646  */
    { if (rewrite) { add_rewrite_rule(); (yyval.net) = fsm_rewrite(rewrite_rules); clear_rewrite_ruleset(rewrite_rules); } rewrite = 0; contexts = NULL; rules = NULL; rewrite_rules = NULL; }
#line 2015 "regex.c" /* yacc.c:1646  */
    break;

  case 10:
#line 253 "regex.y" /* yacc.c:1646  */
    { }
#line 2021 "regex.c" /* yacc.c:1646  */
    break;

  case 11:
#line 254 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = NULL; add_context_pair((yyvsp[-2].net),(yyvsp[0].net));}
#line 2027 "regex.c" /* yacc.c:1646  */
    break;

  case 12:
#line 255 "regex.y" /* yacc.c:1646  */
    { add_context_pair((yyvsp[-1].net),fsm_empty_string()); }
#line 2033 "regex.c" /* yacc.c:1646  */
    break;

  case 13:
#line 256 "regex.y" /* yacc.c:1646  */
    { add_context_pair((yyvsp[-3].net),fsm_empty_string()); }
#line 2039 "regex.c" /* yacc.c:1646  */
    break;

  case 14:
#line 257 "regex.y" /* yacc.c:1646  */
    { add_context_pair(fsm_empty_string(),fsm_empty_string());}
#line 2045 "regex.c" /* yacc.c:1646  */
    break;

  case 15:
#line 258 "regex.y" /* yacc.c:1646  */
    { add_rewrite_rule(); add_context_pair(fsm_empty_string(),fsm_empty_string());}
#line 2051 "regex.c" /* yacc.c:1646  */
    break;

  case 16:
#line 259 "regex.y" /* yacc.c:1646  */
    { add_rewrite_rule(); add_context_pair((yyvsp[-3].net),fsm_empty_string());}
#line 2057 "regex.c" /* yacc.c:1646  */
    break;

  case 17:
#line 260 "regex.y" /* yacc.c:1646  */
    { add_context_pair(fsm_empty_string(),fsm_empty_string());}
#line 2063 "regex.c" /* yacc.c:1646  */
    break;

  case 18:
#line 261 "regex.y" /* yacc.c:1646  */
    { add_context_pair(fsm_empty_string(),(yyvsp[0].net)); }
#line 2069 "regex.c" /* yacc.c:1646  */
    break;

  case 19:
#line 262 "regex.y" /* yacc.c:1646  */
    { add_context_pair(fsm_empty_string(),(yyvsp[-2].net)); }
#line 2075 "regex.c" /* yacc.c:1646  */
    break;

  case 20:
#line 263 "regex.y" /* yacc.c:1646  */
    { add_context_pair((yyvsp[-4].net),(yyvsp[-2].net)); }
#line 2081 "regex.c" /* yacc.c:1646  */
    break;

  case 21:
#line 264 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_context_restrict((yyvsp[-2].net),contexts); fsm_clear_contexts(contexts);}
#line 2087 "regex.c" /* yacc.c:1646  */
    break;

  case 22:
#line 265 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-2].net),(yyvsp[0].net),NULL,(yyvsp[-1].type)); if ((yyvsp[-2].net)->arity == 2) { printf("Error: LHS is transducer\n"); YYERROR;}}
#line 2093 "regex.c" /* yacc.c:1646  */
    break;

  case 23:
#line 266 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-1].net),NULL,NULL,(yyvsp[0].type)); }
#line 2099 "regex.c" /* yacc.c:1646  */
    break;

  case 24:
#line 268 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-3].net),(yyvsp[0].net),NULL,(yyvsp[-1].type)|ARROW_DOTTED); if ((yyvsp[0].net) == NULL) { YYERROR;}}
#line 2105 "regex.c" /* yacc.c:1646  */
    break;

  case 25:
#line 269 "regex.y" /* yacc.c:1646  */
    { add_rule(fsm_empty_string(),(yyvsp[0].net),NULL,(yyvsp[-1].type)|ARROW_DOTTED); if ((yyvsp[0].net) == NULL) { YYERROR;}}
#line 2111 "regex.c" /* yacc.c:1646  */
    break;

  case 26:
#line 270 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),(yyvsp[-2].net),NULL,(yyvsp[-3].type)|ARROW_DOTTED);}
#line 2117 "regex.c" /* yacc.c:1646  */
    break;

  case 27:
#line 271 "regex.y" /* yacc.c:1646  */
    { add_rule(fsm_empty_string(),(yyvsp[-2].net),NULL,(yyvsp[-3].type)|ARROW_DOTTED);}
#line 2123 "regex.c" /* yacc.c:1646  */
    break;

  case 28:
#line 272 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),(yyvsp[-2].net),NULL,(yyvsp[-3].type)|ARROW_DOTTED); rule_direction = (yyvsp[-1].type);}
#line 2129 "regex.c" /* yacc.c:1646  */
    break;

  case 29:
#line 273 "regex.y" /* yacc.c:1646  */
    { add_rule(fsm_empty_string(),(yyvsp[-2].net),NULL,(yyvsp[-3].type)|ARROW_DOTTED); rule_direction = (yyvsp[-1].type);}
#line 2135 "regex.c" /* yacc.c:1646  */
    break;

  case 30:
#line 274 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-4].net),(yyvsp[-2].net),NULL,(yyvsp[-3].type));}
#line 2141 "regex.c" /* yacc.c:1646  */
    break;

  case 31:
#line 275 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-3].net),NULL,NULL,(yyvsp[-2].type));}
#line 2147 "regex.c" /* yacc.c:1646  */
    break;

  case 32:
#line 276 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-4].net),(yyvsp[-2].net),NULL,(yyvsp[-3].type)); rule_direction = (yyvsp[-1].type);}
#line 2153 "regex.c" /* yacc.c:1646  */
    break;

  case 33:
#line 277 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-3].net),NULL,NULL,(yyvsp[-2].type)); rule_direction = (yyvsp[-1].type);}
#line 2159 "regex.c" /* yacc.c:1646  */
    break;

  case 34:
#line 279 "regex.y" /* yacc.c:1646  */
    { add_rewrite_rule();}
#line 2165 "regex.c" /* yacc.c:1646  */
    break;

  case 35:
#line 281 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-2].net),fsm_empty_string(),fsm_empty_string(),(yyvsp[-1].type));}
#line 2171 "regex.c" /* yacc.c:1646  */
    break;

  case 36:
#line 282 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-3].net),fsm_empty_string(),(yyvsp[0].net),(yyvsp[-2].type));}
#line 2177 "regex.c" /* yacc.c:1646  */
    break;

  case 37:
#line 283 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-3].net),(yyvsp[-1].net),fsm_empty_string(),(yyvsp[-2].type));}
#line 2183 "regex.c" /* yacc.c:1646  */
    break;

  case 38:
#line 284 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-4].net),(yyvsp[-2].net),(yyvsp[0].net),(yyvsp[-3].type));}
#line 2189 "regex.c" /* yacc.c:1646  */
    break;

  case 39:
#line 285 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-6].net),(yyvsp[-4].net),(yyvsp[-2].net),(yyvsp[-5].type));}
#line 2195 "regex.c" /* yacc.c:1646  */
    break;

  case 40:
#line 286 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),(yyvsp[-3].net),fsm_empty_string(),(yyvsp[-4].type));}
#line 2201 "regex.c" /* yacc.c:1646  */
    break;

  case 41:
#line 287 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),fsm_empty_string(),(yyvsp[-2].net),(yyvsp[-4].type));}
#line 2207 "regex.c" /* yacc.c:1646  */
    break;

  case 42:
#line 288 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-4].net),fsm_empty_string(),fsm_empty_string(),(yyvsp[-3].type));}
#line 2213 "regex.c" /* yacc.c:1646  */
    break;

  case 43:
#line 289 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-6].net),(yyvsp[-4].net),(yyvsp[-2].net),(yyvsp[-5].type)); rule_direction = (yyvsp[-1].type);}
#line 2219 "regex.c" /* yacc.c:1646  */
    break;

  case 44:
#line 290 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),fsm_empty_string(),(yyvsp[-2].net),(yyvsp[-4].type)); rule_direction = (yyvsp[-1].type);}
#line 2225 "regex.c" /* yacc.c:1646  */
    break;

  case 45:
#line 291 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-5].net),(yyvsp[-3].net),fsm_empty_string(),(yyvsp[-4].type)); rule_direction = (yyvsp[-1].type);}
#line 2231 "regex.c" /* yacc.c:1646  */
    break;

  case 46:
#line 292 "regex.y" /* yacc.c:1646  */
    { add_rule((yyvsp[-4].net),fsm_empty_string(),fsm_empty_string(),(yyvsp[-3].type)); rule_direction = (yyvsp[-1].type);}
#line 2237 "regex.c" /* yacc.c:1646  */
    break;

  case 47:
#line 295 "regex.y" /* yacc.c:1646  */
    { }
#line 2243 "regex.c" /* yacc.c:1646  */
    break;

  case 48:
#line 296 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_shuffle((yyvsp[-2].net),(yyvsp[0].net));  }
#line 2249 "regex.c" /* yacc.c:1646  */
    break;

  case 49:
#line 297 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_precedes((yyvsp[-2].net),(yyvsp[0].net)); }
#line 2255 "regex.c" /* yacc.c:1646  */
    break;

  case 50:
#line 298 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_follows((yyvsp[-2].net),(yyvsp[0].net));  }
#line 2261 "regex.c" /* yacc.c:1646  */
    break;

  case 51:
#line 300 "regex.y" /* yacc.c:1646  */
    { }
#line 2267 "regex.c" /* yacc.c:1646  */
    break;

  case 52:
#line 302 "regex.y" /* yacc.c:1646  */
    { }
#line 2273 "regex.c" /* yacc.c:1646  */
    break;

  case 53:
#line 304 "regex.y" /* yacc.c:1646  */
    { }
#line 2279 "regex.c" /* yacc.c:1646  */
    break;

  case 54:
#line 305 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_union((yyvsp[-2].net),(yyvsp[0].net));                     }
#line 2285 "regex.c" /* yacc.c:1646  */
    break;

  case 55:
#line 306 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_priority_union_upper((yyvsp[-2].net),(yyvsp[0].net));      }
#line 2291 "regex.c" /* yacc.c:1646  */
    break;

  case 56:
#line 307 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_priority_union_lower((yyvsp[-2].net),(yyvsp[0].net));      }
#line 2297 "regex.c" /* yacc.c:1646  */
    break;

  case 57:
#line 308 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_intersect((yyvsp[-2].net),(yyvsp[0].net));                 }
#line 2303 "regex.c" /* yacc.c:1646  */
    break;

  case 58:
#line 309 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_minus((yyvsp[-2].net),(yyvsp[0].net));                     }
#line 2309 "regex.c" /* yacc.c:1646  */
    break;

  case 59:
#line 310 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_union(fsm_complement((yyvsp[-2].net)),(yyvsp[0].net));     }
#line 2315 "regex.c" /* yacc.c:1646  */
    break;

  case 60:
#line 311 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_intersect(fsm_union(fsm_complement(fsm_copy((yyvsp[-2].net))),fsm_copy((yyvsp[0].net))), fsm_union(fsm_complement(fsm_copy((yyvsp[0].net))),fsm_copy((yyvsp[-2].net)))); fsm_destroy((yyvsp[-2].net)); fsm_destroy((yyvsp[0].net));}
#line 2321 "regex.c" /* yacc.c:1646  */
    break;

  case 61:
#line 313 "regex.y" /* yacc.c:1646  */
    { }
#line 2327 "regex.c" /* yacc.c:1646  */
    break;

  case 62:
#line 314 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_concat((yyvsp[-1].net),(yyvsp[0].net)); }
#line 2333 "regex.c" /* yacc.c:1646  */
    break;

  case 63:
#line 315 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_ignore(fsm_contains(fsm_concat(fsm_symbol((yyvsp[-2].string)),fsm_concat((yyvsp[0].net),fsm_symbol((yyvsp[-2].string))))),union_quantifiers(),OP_IGNORE_ALL); }
#line 2339 "regex.c" /* yacc.c:1646  */
    break;

  case 64:
#line 317 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_logical_eq((yyvsp[-2].string),(yyvsp[0].string)); }
#line 2345 "regex.c" /* yacc.c:1646  */
    break;

  case 65:
#line 318 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_complement(fsm_logical_eq((yyvsp[-2].string),(yyvsp[0].string))); }
#line 2351 "regex.c" /* yacc.c:1646  */
    break;

  case 66:
#line 319 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_logical_precedence((yyvsp[-2].string),(yyvsp[0].string)); }
#line 2357 "regex.c" /* yacc.c:1646  */
    break;

  case 67:
#line 320 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_logical_precedence((yyvsp[0].string),(yyvsp[-2].string)); }
#line 2363 "regex.c" /* yacc.c:1646  */
    break;

  case 68:
#line 322 "regex.y" /* yacc.c:1646  */
    { }
#line 2369 "regex.c" /* yacc.c:1646  */
    break;

  case 69:
#line 323 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_ignore((yyvsp[-2].net),(yyvsp[0].net), OP_IGNORE_ALL);          }
#line 2375 "regex.c" /* yacc.c:1646  */
    break;

  case 70:
#line 324 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_ignore((yyvsp[-2].net),(yyvsp[0].net), OP_IGNORE_INTERNAL);     }
#line 2381 "regex.c" /* yacc.c:1646  */
    break;

  case 71:
#line 325 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_quotient_right((yyvsp[-2].net),(yyvsp[0].net));                 }
#line 2387 "regex.c" /* yacc.c:1646  */
    break;

  case 72:
#line 326 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_quotient_left((yyvsp[-2].net),(yyvsp[0].net));                  }
#line 2393 "regex.c" /* yacc.c:1646  */
    break;

  case 73:
#line 327 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_quotient_interleave((yyvsp[-2].net),(yyvsp[0].net));            }
#line 2399 "regex.c" /* yacc.c:1646  */
    break;

  case 74:
#line 329 "regex.y" /* yacc.c:1646  */
    { }
#line 2405 "regex.c" /* yacc.c:1646  */
    break;

  case 75:
#line 330 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_complement((yyvsp[0].net));       }
#line 2411 "regex.c" /* yacc.c:1646  */
    break;

  case 76:
#line 331 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_contains((yyvsp[0].net));         }
#line 2417 "regex.c" /* yacc.c:1646  */
    break;

  case 77:
#line 332 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_contains_one((yyvsp[0].net));     }
#line 2423 "regex.c" /* yacc.c:1646  */
    break;

  case 78:
#line 333 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_contains_opt_one((yyvsp[0].net)); }
#line 2429 "regex.c" /* yacc.c:1646  */
    break;

  case 79:
#line 335 "regex.y" /* yacc.c:1646  */
    { }
#line 2435 "regex.c" /* yacc.c:1646  */
    break;

  case 80:
#line 337 "regex.y" /* yacc.c:1646  */
    { }
#line 2441 "regex.c" /* yacc.c:1646  */
    break;

  case 81:
#line 338 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_kleene_star(fsm_minimize((yyvsp[-1].net))); }
#line 2447 "regex.c" /* yacc.c:1646  */
    break;

  case 82:
#line 339 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_kleene_plus((yyvsp[-1].net)); }
#line 2453 "regex.c" /* yacc.c:1646  */
    break;

  case 83:
#line 340 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_determinize(fsm_reverse((yyvsp[-1].net))); }
#line 2459 "regex.c" /* yacc.c:1646  */
    break;

  case 84:
#line 341 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_invert((yyvsp[-1].net)); }
#line 2465 "regex.c" /* yacc.c:1646  */
    break;

  case 85:
#line 342 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_upper((yyvsp[-1].net)); }
#line 2471 "regex.c" /* yacc.c:1646  */
    break;

  case 86:
#line 343 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_lower((yyvsp[-1].net)); }
#line 2477 "regex.c" /* yacc.c:1646  */
    break;

  case 87:
#line 344 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = flag_eliminate((yyvsp[-1].net), NULL); }
#line 2483 "regex.c" /* yacc.c:1646  */
    break;

  case 88:
#line 345 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_cross_product((yyvsp[-2].net),(yyvsp[0].net)); }
#line 2489 "regex.c" /* yacc.c:1646  */
    break;

  case 89:
#line 347 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_concat_n((yyvsp[-1].net),atoi((yyvsp[0].string))); }
#line 2495 "regex.c" /* yacc.c:1646  */
    break;

  case 90:
#line 348 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_concat(fsm_concat_n(fsm_copy((yyvsp[-1].net)), atoi((yyvsp[0].string))),fsm_kleene_plus(fsm_copy((yyvsp[-1].net)))); fsm_destroy((yyvsp[-1].net)); }
#line 2501 "regex.c" /* yacc.c:1646  */
    break;

  case 91:
#line 349 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_concat_m_n((yyvsp[-1].net),0,atoi((yyvsp[0].string))-1); }
#line 2507 "regex.c" /* yacc.c:1646  */
    break;

  case 92:
#line 350 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_concat_m_n((yyvsp[-1].net),atoi((yyvsp[0].string)),atoi(strstr((yyvsp[0].string),",")+1)); }
#line 2513 "regex.c" /* yacc.c:1646  */
    break;

  case 93:
#line 352 "regex.y" /* yacc.c:1646  */
    { }
#line 2519 "regex.c" /* yacc.c:1646  */
    break;

  case 94:
#line 353 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_term_negation((yyvsp[0].net)); }
#line 2525 "regex.c" /* yacc.c:1646  */
    break;

  case 95:
#line 355 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = (yyvsp[0].net);}
#line 2531 "regex.c" /* yacc.c:1646  */
    break;

  case 96:
#line 356 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = (yyvsp[0].net); }
#line 2537 "regex.c" /* yacc.c:1646  */
    break;

  case 97:
#line 357 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_complement(fsm_substitute_symbol(fsm_intersect(fsm_quantifier((yyvsp[-3].string)),fsm_complement((yyvsp[-1].net))),(yyvsp[-3].string),"@_EPSILON_SYMBOL_@")); purge_quantifier((yyvsp[-3].string)); }
#line 2543 "regex.c" /* yacc.c:1646  */
    break;

  case 98:
#line 358 "regex.y" /* yacc.c:1646  */
    {  (yyval.net) = fsm_substitute_symbol(fsm_intersect(fsm_quantifier((yyvsp[-1].string)),(yyvsp[0].net)),(yyvsp[-1].string),"@_EPSILON_SYMBOL_@"); purge_quantifier((yyvsp[-1].string)); }
#line 2549 "regex.c" /* yacc.c:1646  */
    break;

  case 99:
#line 359 "regex.y" /* yacc.c:1646  */
    { if (count_quantifiers()) (yyval.net) = (yyvsp[-1].net); else {(yyval.net) = fsm_optionality((yyvsp[-1].net));} }
#line 2555 "regex.c" /* yacc.c:1646  */
    break;

  case 100:
#line 360 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = (yyvsp[-1].net); }
#line 2561 "regex.c" /* yacc.c:1646  */
    break;

  case 101:
#line 361 "regex.y" /* yacc.c:1646  */
    {(yyval.net) = fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-3].string)),fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-3].string)),fsm_concat(union_quantifiers(),fsm_concat(fsm_symbol((yyvsp[-1].string)),fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-1].string)),fsm_universal())))))))); }
#line 2567 "regex.c" /* yacc.c:1646  */
    break;

  case 102:
#line 362 "regex.y" /* yacc.c:1646  */
    {(yyval.net) = fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-3].string)),fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-3].string)),fsm_concat(fsm_ignore((yyvsp[-1].net),union_quantifiers(),OP_IGNORE_ALL),fsm_universal()))))); }
#line 2573 "regex.c" /* yacc.c:1646  */
    break;

  case 103:
#line 363 "regex.y" /* yacc.c:1646  */
    {(yyval.net) = fsm_concat(fsm_universal(),fsm_concat(fsm_ignore((yyvsp[-3].net),union_quantifiers(),OP_IGNORE_ALL),fsm_concat(fsm_symbol((yyvsp[-1].string)),fsm_concat(fsm_universal(),fsm_concat(fsm_symbol((yyvsp[-1].string)),fsm_universal()))))); }
#line 2579 "regex.c" /* yacc.c:1646  */
    break;

  case 104:
#line 364 "regex.y" /* yacc.c:1646  */
    {(yyval.net) = fsm_substitute_symbol((yyvsp[-1].net),subval1,subval2); substituting = 0; xxfree(subval1); xxfree(subval2); subval1 = subval2 = NULL;}
#line 2585 "regex.c" /* yacc.c:1646  */
    break;

  case 105:
#line 366 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = (yyvsp[-1].net); substituting = 1;                      }
#line 2591 "regex.c" /* yacc.c:1646  */
    break;

  case 106:
#line 367 "regex.y" /* yacc.c:1646  */
    { subval1 = (yyvsp[-2].string); subval2 = (yyvsp[0].string); }
#line 2597 "regex.c" /* yacc.c:1646  */
    break;

  case 107:
#line 369 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = (yyvsp[0].net); }
#line 2603 "regex.c" /* yacc.c:1646  */
    break;

  case 108:
#line 370 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_boolean(fsm_isidentity((yyvsp[-1].net)));   }
#line 2609 "regex.c" /* yacc.c:1646  */
    break;

  case 109:
#line 371 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_boolean(fsm_isfunctional((yyvsp[-1].net))); }
#line 2615 "regex.c" /* yacc.c:1646  */
    break;

  case 110:
#line 372 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_boolean(fsm_isunambiguous((yyvsp[-1].net))); }
#line 2621 "regex.c" /* yacc.c:1646  */
    break;

  case 111:
#line 373 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_extract_nonidentity(fsm_copy((yyvsp[-1].net))); }
#line 2627 "regex.c" /* yacc.c:1646  */
    break;

  case 112:
#line 374 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_lowerdet(fsm_copy((yyvsp[-1].net))); }
#line 2633 "regex.c" /* yacc.c:1646  */
    break;

  case 113:
#line 375 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_lowerdeteps(fsm_copy((yyvsp[-1].net))); }
#line 2639 "regex.c" /* yacc.c:1646  */
    break;

  case 114:
#line 376 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_markallfinal(fsm_copy((yyvsp[-1].net))); }
#line 2645 "regex.c" /* yacc.c:1646  */
    break;

  case 115:
#line 377 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_extract_unambiguous(fsm_copy((yyvsp[-1].net)));      }
#line 2651 "regex.c" /* yacc.c:1646  */
    break;

  case 116:
#line 378 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_extract_ambiguous(fsm_copy((yyvsp[-1].net)));        }
#line 2657 "regex.c" /* yacc.c:1646  */
    break;

  case 117:
#line 379 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_extract_ambiguous_domain(fsm_copy((yyvsp[-1].net))); }
#line 2663 "regex.c" /* yacc.c:1646  */
    break;

  case 118:
#line 380 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_letter_machine(fsm_copy((yyvsp[-1].net))); }
#line 2669 "regex.c" /* yacc.c:1646  */
    break;

  case 119:
#line 381 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_mark_fsm_tail((yyvsp[-3].net),(yyvsp[-1].net)); }
#line 2675 "regex.c" /* yacc.c:1646  */
    break;

  case 120:
#line 382 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_add_loop((yyvsp[-3].net),(yyvsp[-1].net),1); }
#line 2681 "regex.c" /* yacc.c:1646  */
    break;

  case 121:
#line 383 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_add_loop((yyvsp[-3].net),(yyvsp[-1].net),0); }
#line 2687 "regex.c" /* yacc.c:1646  */
    break;

  case 122:
#line 384 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_add_loop((yyvsp[-3].net),(yyvsp[-1].net),2); }
#line 2693 "regex.c" /* yacc.c:1646  */
    break;

  case 123:
#line 385 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_add_sink((yyvsp[-1].net),1); }
#line 2699 "regex.c" /* yacc.c:1646  */
    break;

  case 124:
#line 386 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_left_rewr((yyvsp[-3].net),(yyvsp[-1].net)); }
#line 2705 "regex.c" /* yacc.c:1646  */
    break;

  case 125:
#line 387 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_flatten((yyvsp[-3].net),(yyvsp[-1].net)); }
#line 2711 "regex.c" /* yacc.c:1646  */
    break;

  case 126:
#line 388 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_substitute_label((yyvsp[-5].net), fsm_network_to_char((yyvsp[-3].net)), (yyvsp[-1].net)); }
#line 2717 "regex.c" /* yacc.c:1646  */
    break;

  case 127:
#line 389 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_close_sigma(fsm_copy((yyvsp[-1].net)), 0); }
#line 2723 "regex.c" /* yacc.c:1646  */
    break;

  case 128:
#line 390 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_close_sigma(fsm_copy((yyvsp[-1].net)), 1); }
#line 2729 "regex.c" /* yacc.c:1646  */
    break;

  case 129:
#line 391 "regex.y" /* yacc.c:1646  */
    { (yyval.net) = fsm_equal_substrings((yyvsp[-5].net),(yyvsp[-3].net),(yyvsp[-1].net)); }
#line 2735 "regex.c" /* yacc.c:1646  */
    break;

  case 130:
#line 394 "regex.y" /* yacc.c:1646  */
    { frec++; fargptr[frec] = 0 ;declare_function_name((yyvsp[-2].string)) ; add_function_argument((yyvsp[-1].net)); }
#line 2741 "regex.c" /* yacc.c:1646  */
    break;

  case 131:
#line 396 "regex.y" /* yacc.c:1646  */
    { frec++; fargptr[frec] = 0 ;declare_function_name((yyvsp[-1].string)) ; add_function_argument((yyvsp[0].net)); }
#line 2747 "regex.c" /* yacc.c:1646  */
    break;

  case 132:
#line 398 "regex.y" /* yacc.c:1646  */
    { add_function_argument((yyvsp[-1].net)); }
#line 2753 "regex.c" /* yacc.c:1646  */
    break;

  case 133:
#line 399 "regex.y" /* yacc.c:1646  */
    { add_function_argument((yyvsp[-1].net)); }
#line 2759 "regex.c" /* yacc.c:1646  */
    break;

  case 134:
#line 402 "regex.y" /* yacc.c:1646  */
    { add_function_argument((yyvsp[-1].net)); if (((yyval.net) = function_apply(defined_nets, defined_funcs)) == NULL) YYERROR; }
#line 2765 "regex.c" /* yacc.c:1646  */
    break;

  case 135:
#line 404 "regex.y" /* yacc.c:1646  */
    { add_function_argument((yyvsp[-1].net)); if (((yyval.net) = function_apply(defined_nets, defined_funcs)) == NULL) YYERROR; }
#line 2771 "regex.c" /* yacc.c:1646  */
    break;

  case 136:
#line 406 "regex.y" /* yacc.c:1646  */
    { if (((yyval.net) = function_apply(defined_nets, defined_funcs)) == NULL) YYERROR;}
#line 2777 "regex.c" /* yacc.c:1646  */
    break;


#line 2781 "regex.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, scanner, defined_nets, defined_funcs, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, scanner, defined_nets, defined_funcs, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, scanner, defined_nets, defined_funcs);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, scanner, defined_nets, defined_funcs);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, scanner, defined_nets, defined_funcs, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, scanner, defined_nets, defined_funcs);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, scanner, defined_nets, defined_funcs);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 408 "regex.y" /* yacc.c:1906  */

