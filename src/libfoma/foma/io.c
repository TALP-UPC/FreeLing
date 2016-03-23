/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2014 Mans Hulden                                     */

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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "foma.h"
#include "zlib.h"

#define TYPE_TRANSITION 1
#define TYPE_SYMBOL 2
#define TYPE_FINAL 3
#define TYPE_PROPERTY 4
#define TYPE_END 5
#define TYPE_ERROR 6

#define READ_BUF_SIZE 4096

struct binaryline {
    int type;
    int state;
    int in;
    int target;
    int out;
    int symbol;
    char *name;
    char *value;
};

extern char *g_att_epsilon;

extern struct defined_networks   *g_defines;
extern struct defined_functions  *g_defines_f;

struct io_buf_handle {
    char *io_buf;
    char *io_buf_ptr;
};

/* deactivate tty interaction, for FreeLing we */
/* need only library mode, and this function is */
/* unknown for MSVC */
int isatty(int x) {return 0;}


struct io_buf_handle *io_init();
void io_free(struct io_buf_handle *iobh);
static int io_gets(struct io_buf_handle *iobh, char *target);
static size_t io_get_gz_file_size(char *filename);
static size_t io_get_file_size(char *filename);
static size_t io_get_regular_file_size(char *filename);
size_t io_gz_file_to_mem (struct io_buf_handle *iobh, char *filename);
int foma_net_print(struct fsm *net, gzFile outfile);
struct fsm *io_net_read(struct io_buf_handle *iobh, char **net_name);
static INLINE int explode_line (char *buf, int *values);


void escape_print(FILE *stream, char* string) {
    int i;
    if (strchr(string, '"') != NULL) {
	for (i = 0; *(string+i) != '\0'; i++) {
	    if (*(string+i) == '"') {
		fprintf(stream, "\\\""); 
	    } else {
		fputc(*(string+i), stream);
	    }
	}
    } else {
	fprintf(stream, "%s", string);
    }
}

int foma_write_prolog (struct fsm *net, char *filename) {
  struct fsm_state *stateptr;
  int i, *finals, *used_symbols, maxsigma;
  FILE *out;
  char *outstring, *instring, identifier[100];
  
  if (filename == NULL) {
    out = stdout;
  } else {
    if ((out = fopen(filename, "w")) == NULL) {
      printf("Error writing to file '%s'. Using stdout.\n", filename);
      out = stdout;
    }
    printf("Writing prolog to file '%s'.\n", filename);
  }
  fsm_count(net);
  maxsigma = sigma_max(net->sigma);
  used_symbols = xxcalloc(maxsigma+1,sizeof(int));
  finals = xxmalloc(sizeof(int)*(net->statecount));
  stateptr = net->states;
  identifier[0] = '\0';

  strcpy(identifier, net->name);

  /* Print identifier */
  fprintf(out, "%s%s%s", "network(",identifier,").\n");

  for (i=0; (stateptr+i)->state_no != -1; i++) {
    if ((stateptr+i)->final_state == 1) {
      *(finals+((stateptr+i)->state_no)) = 1;
    } else {
      *(finals+((stateptr+i)->state_no)) = 0;
    }
    if ((stateptr+i)->in != -1) {
      *(used_symbols+((stateptr+i)->in)) = 1;
    }
    if ((stateptr+i)->out != -1) {
      *(used_symbols+((stateptr+i)->out)) = 1;
    }

  }

  for (i = 3; i <= maxsigma; i++) {
    if (*(used_symbols+i) == 0) {
      instring = sigma_string(i, net->sigma);
      if (strcmp(instring,"0") == 0) {
	  instring = "%0";
      } 
      fprintf(out, "symbol(%s, \"", identifier);
      escape_print(out, instring);
      fprintf(out, "\").\n"); 

    }
  }
  
  for (; stateptr->state_no != -1; stateptr++) {
    if (stateptr->target == -1)
      continue;
    fprintf(out, "arc(%s, %i, %i, ", identifier, stateptr->state_no, stateptr->target);
    if      (stateptr->in == 0) instring = "0";
    else if (stateptr->in == 1) instring = "?";
    else if (stateptr->in == 2) instring = "?";
    else instring = sigma_string(stateptr->in, net->sigma);
    if      (stateptr->out == 0) outstring = "0";
    else if (stateptr->out == 1) outstring = "?";
    else if (stateptr->out == 2) outstring = "?";
    else outstring = sigma_string(stateptr->out, net->sigma);

    if (strcmp(instring,"0") == 0 && stateptr->in != 0) instring = "%0";
    if (strcmp(outstring,"0") == 0 && stateptr->out != 0) outstring = "%0";
    if (strcmp(instring,"?") == 0 && stateptr->in > 2) instring = "%?";
    if (strcmp(outstring,"?") == 0 && stateptr->in > 2) outstring = "%?";
    /* Escape quotes */
    
    if (net->arity == 2 && stateptr->in == IDENTITY && stateptr->out == IDENTITY) {
      fprintf(out, "\"?\").\n");
    }
    else if (net->arity == 2 && stateptr->in == stateptr->out && stateptr->in != UNKNOWN) {
	fprintf(out, "\"");
	escape_print(out, instring);
	fprintf(out, "\").\n");
    }
    else if (net->arity == 2) {	
      fprintf(out, "\"");
      escape_print(out, instring);
      fprintf(out, "\":\"");
      escape_print(out, outstring); 
      fprintf(out, "\").\n");
    }
    else if (net->arity == 1) {
	fprintf(out, "\"");
	escape_print(out, instring);
	fprintf(out, "\").\n");
    }
  }

  for (i = 0; i < net->statecount; i++) {
    if (*(finals+i)) {
      fprintf(out, "final(%s, %i).\n", identifier, i);
    }
  }
  if (filename != NULL) {
      fclose(out);
  }
  xxfree(finals);
  xxfree(used_symbols);
  return 1;
}

struct fsm *read_att(char *filename) {

    struct fsm_construct_handle *h;
    struct fsm *net;
    int i;
    char inword[1024], delimiters[] = "\t", *tokens[6];
    FILE *INFILE;

    INFILE = fopen(filename, "r");
    if (INFILE == NULL) {
        return(NULL);
    }

    h = fsm_construct_init(filename);
    while (fgets(inword, 1024, INFILE) != NULL) {
        if (inword[strlen(inword)-1] == '\n') {
            inword[strlen(inword)-1] = '\0';
        }
        tokens[0] = strtok(inword, delimiters);
        i = 0;
        if (tokens[0] != NULL) {
            i = 1;
            for ( ; ; ) {
                tokens[i] = strtok(NULL, delimiters);
                if (tokens[i] == NULL) {
                    break;
                }
                i++;
                if (i == 6)
                    break;
            }
        }
        if (i == 0) { continue; }
        if (i >= 4) {
            if (strcmp(tokens[2],g_att_epsilon) == 0)
                tokens[2] = "@_EPSILON_SYMBOL_@";
            if (strcmp(tokens[3],g_att_epsilon) == 0)
                tokens[3] = "@_EPSILON_SYMBOL_@";

            fsm_construct_add_arc(h, atoi(tokens[0]), atoi(tokens[1]), tokens[2], tokens[3]);
        }
        else if (i <= 3 && i > 0) {
            fsm_construct_set_final(h,atoi(tokens[0]));
        }
    }
    fsm_construct_set_initial(h,0);
    fclose(INFILE);
    net = fsm_construct_done(h);
    fsm_count(net);
    net = fsm_topsort(net);
    return(net);
}

struct fsm *fsm_read_prolog (char *filename) {
    char buf [1024], temp [1024], in [128], out[128], *temp_ptr, *temp_ptr2;
    int arity, source, target, has_net;
    struct fsm *outnet;
    struct fsm_construct_handle *outh = NULL;
    FILE *prolog_file;
    
    has_net = 0;
    prolog_file = fopen(filename, "r");
    if (prolog_file == NULL) {
	return NULL;
    }

    while (fgets(buf, 1023, prolog_file) != NULL) {
	if (strstr(buf, "network(") == buf) {
	    /* Extract network name */
	    if (has_net == 1) {
		perror("WARNING: prolog file contains multiple nets. Only returning the first one.\n");
		break;
	    } else {
		has_net = 1;
	    }
	    temp_ptr = strstr(buf, "network(")+8;
	    temp_ptr2 = strstr(buf, ").");
	    strncpy(temp, temp_ptr, (temp_ptr2 - temp_ptr));
	    temp[(temp_ptr2-temp_ptr)] = '\0';
	    
	    /* Start network */
	    outh = fsm_construct_init(temp);
	}
	if (strstr(buf, "final(") == buf) {
	    temp_ptr = strstr(buf, " ");
	    temp_ptr++;
	    temp_ptr2 = strstr(temp_ptr, ").");
	    strncpy(temp, temp_ptr, (temp_ptr2 - temp_ptr));
	    temp[(temp_ptr2-temp_ptr)] = '\0';
	    
	    fsm_construct_set_final(outh, atoi(temp));
	}
	if (strstr(buf, "symbol(") == buf) {
	    temp_ptr = strstr(buf, ", \"")+3;
	    temp_ptr2 = strstr(temp_ptr, "\").");
	    strncpy(temp, temp_ptr, (temp_ptr2 - temp_ptr));
	    temp[(temp_ptr2-temp_ptr)] = '\0';
	    if (strcmp(temp, "%0") == 0)
		strcpy(temp, "0");
	    //printf("special: %s\n",temp);
	    
	    if (fsm_construct_check_symbol(outh, temp) == -1) {
		fsm_construct_add_symbol(outh, temp);
	    }      
	    continue;
	}
	if (strstr(buf, "arc(") == buf) {
	    in[0] = '\0';
	    out[0] = '\0';
	    
	    if (strstr(buf, "\":\"") == NULL || strstr(buf, ", \":\").") != NULL) {
		arity = 1;
	    } else {
		arity = 2;
	    }
	    
	    /* Get source */
	    temp_ptr = strstr(buf, " ");
	    temp_ptr++;
	    temp_ptr2 = strstr(temp_ptr, ",");
	    strncpy(temp, temp_ptr, (temp_ptr2 - temp_ptr));
	    temp[(temp_ptr2-temp_ptr)] = '\0';
	    source = atoi(temp);
	    
	    /* Get target */
	    temp_ptr = strstr(temp_ptr2, " ");
	    temp_ptr++;
	    temp_ptr2 = strstr(temp_ptr, ",");
	    strncpy(temp, temp_ptr, (temp_ptr2 - temp_ptr));
	    temp[(temp_ptr2-temp_ptr)] = '\0';
	    target = atoi(temp);
	    
	    temp_ptr = strstr(temp_ptr2, "\"");
	    temp_ptr++;
	    if (arity == 2)  { 
		temp_ptr2 = strstr(temp_ptr, "\":");
	    } else {
		temp_ptr2 = strstr(temp_ptr, "\").");
	    }
	    strncpy(in, temp_ptr, (temp_ptr2 - temp_ptr));
	    in[(temp_ptr2 - temp_ptr)] = '\0';
	    
	    if (arity == 2) {
		temp_ptr = strstr(temp_ptr2, ":\"");
		temp_ptr += 2;
		temp_ptr2 = strstr(temp_ptr, "\").");
		strncpy(out, temp_ptr, (temp_ptr2 - temp_ptr));
		out[(temp_ptr2 - temp_ptr)] = '\0';
	    }
	    if (arity == 1 && (strcmp(in, "?") == 0)) {
		strcpy(in,"@_IDENTITY_SYMBOL_@");
	    }
	    if (arity == 2 && (strcmp(in, "?") == 0)) {
		strcpy(in,"@_UNKNOWN_SYMBOL_@");
	    }
	    if (arity == 2 && (strcmp(out, "?") == 0)) {
		strcpy(out,"@_UNKNOWN_SYMBOL_@");
	    }
	    if (strcmp(in, "0") == 0) {
		strcpy(in,"@_EPSILON_SYMBOL_@");
	    }
	    if (strcmp(out, "0") == 0) {
		strcpy(out,"@_EPSILON_SYMBOL_@");
	    }
	    if (strcmp(in, "%0") == 0) {
		strcpy(in,"0");
	    }
	    if (strcmp(out, "%0") == 0) {
		strcpy(out,"0");
	    }
	    if (strcmp(in, "%?") == 0) {
		strcpy(in,"?");
	    }
	    if (strcmp(out, "%?") == 0) {
		strcpy(out,"?");
	    }
	    
	    if (arity == 1) { 
		fsm_construct_add_arc(outh, source, target, in, in);	    
	    } else {
		fsm_construct_add_arc(outh, source, target, in, out);
	    }
	}
    }
    fclose(prolog_file);
    if (has_net == 1) {
	fsm_construct_set_initial(outh, 0);
	outnet = fsm_construct_done(outh);
	fsm_topsort(outnet);
	return(outnet);
    } else {
	return(NULL);
    }
}

struct io_buf_handle *io_init() {
    struct io_buf_handle *iobh;
    iobh = xxmalloc(sizeof(struct io_buf_handle));
    (iobh->io_buf) = NULL;
    (iobh->io_buf_ptr) = NULL;
    return(iobh);
}

void io_free(struct io_buf_handle *iobh) {
    if (iobh->io_buf != NULL) {
        xxfree(iobh->io_buf);
        (iobh->io_buf) = NULL;
    }
    xxfree(iobh);
}

char *spacedtext_get_next_line(char **text) {
    char *t, *ret;
    ret = *text;
    if (**text == '\0')
	return NULL;
    for (t = *text; *t != '\0' && *t != '\n'; t++) {	
    }
    if (*t == '\0')
	*text = t;
    else 
	*text = t+1;
    *t = '\0';
    return(ret);
}

char *spacedtext_get_next_token(char **text) {
    char *t, *ret;
    if (**text == '\0' || **text == '\n')
	return NULL;
    for ( ; **text == ' ' ; (*text)++) {
    }
    ret = *text;
    for (t = *text; *t != '\0' && *t != '\n' && *t != ' '; t++) {
    }
    if (*t == '\0' || *t == '\n')
	*text = t;
    else
	*text = t+1;
    *t = '\0';
    return(ret);
}

struct fsm *fsm_read_spaced_text_file(char *filename) {
    struct fsm_trie_handle *th;
    char *text, *textorig, *insym, *outsym, *t1, *t2, *l1, *l2;

    text = textorig = file_to_mem(filename);
    
    if (text == NULL)
	return NULL;
    th = fsm_trie_init();
    for (;;) {
	for ( ; *text != '\0' && *text == '\n'; text++) { }
	t1 = spacedtext_get_next_line(&text);
	if (t1 == NULL)
	    break;
	if (strlen(t1) == 0)
	    continue;
	t2 = spacedtext_get_next_line(&text);
	if (t2 == NULL || strlen(t2) == 0) {
	    for (l1 = t1; (insym = spacedtext_get_next_token(&l1)) != NULL; ) {
		if (strcmp(insym, "0") == 0)
		    fsm_trie_symbol(th,  "@_EPSILON_SYMBOL_@", "@_EPSILON_SYMBOL_@");
		else if (strcmp(insym, "%0") == 0)
		    fsm_trie_symbol(th,  "0", "0");
		else
		    fsm_trie_symbol(th,  insym, insym);
	    }
	    fsm_trie_end_word(th);
	} else {
	    for (l1 = t1, l2 = t2; ; ) {
		insym = spacedtext_get_next_token(&l1);
		outsym = spacedtext_get_next_token(&l2);
		if (insym == NULL && outsym == NULL)
		    break;
		if (insym == NULL || strcmp(insym, "0") == 0)
		    insym = "@_EPSILON_SYMBOL_@";
		if (strcmp(insym, "%0") == 0)
		    insym = "0";
		if (outsym == NULL || strcmp(outsym, "0") == 0)
		    outsym = "@_EPSILON_SYMBOL_@";
		if (strcmp(outsym, "%0") == 0)
		    outsym = "0";
		fsm_trie_symbol(th, insym, outsym);
	    }
	    fsm_trie_end_word(th);
	}
    }
    xxfree(textorig);
    return(fsm_trie_done(th));
}

/****************************************
// Original FOMA version. 
// Builds an automata from a vocabulary in a text FILE

struct fsm *fsm_read_text_file(char *filename) {
    struct fsm_trie_handle *th;
    char *text, *textp1, *textp2;
    int lastword;

    text = file_to_mem(filename);
    if (text == NULL) {
	return NULL;
    }
    textp1 = text;
    th = fsm_trie_init();

    for (lastword = 0 ; lastword == 0 ; textp1 = textp2+1) {
	for (textp2 = textp1 ; *textp2 != '\n' && *textp2 != '\0'; textp2++) {
	}
	if (*textp2 == '\0') {
	    lastword = 1;
	    if (textp2 == textp1)
		break;
	}
	*textp2 = '\0';
	if (strlen(textp1) > 0)
	    fsm_trie_add_word(th, textp1);
    }
    xxfree(text);
    return(fsm_trie_done(th));
}
***********************************************/

// Modified version for FreeLing.
// Builds an automata from a vocabulary in a text BUFFER

struct fsm *fsm_read_text_file(const char *buffer) {
    struct fsm_trie_handle *th;
    const char *textp1, *textp2;
    int lastword;

    textp1 = buffer;
    th = fsm_trie_init();

    for (lastword = 0 ; lastword == 0 ; textp1 = textp2+1) {
	for (textp2 = textp1 ; *textp2 != '\n' && *textp2 != '\0'; textp2++) {
	}
	if (*textp2 == '\0') {
	    lastword = 1;
	    if (textp2 == textp1)
		break;
	}
        if (textp2-textp1 > 0) {
          // we need a non-const copy to add the '\0'
          int len = textp2-textp1;
          char *aux = malloc((len+1)*sizeof(char));
          memcpy(aux,textp1,len);
          aux[len]='\0';

          fsm_trie_add_word(th, aux);
          free(aux);
        } 
    }
    return(fsm_trie_done(th));
}


int fsm_write_binary_file(struct fsm *net, char *filename) {
    gzFile outfile;
    if ((outfile = gzopen(filename,"wb")) == NULL) {
	return(1);
    }
    foma_net_print(net, outfile);
    gzclose(outfile);
    return(0);
}

struct fsm *fsm_read_binary_file_multiple(fsm_read_binary_handle fsrh) {
    char *net_name;
    struct fsm *net;
    struct io_buf_handle *iobh;
    iobh = (struct io_buf_handle *) fsrh;
    net = io_net_read(iobh, &net_name);
    if (net == NULL) {
	io_free(iobh);
	return(NULL);
    } else {
	xxfree(net_name);
	return(net);
    }
}

fsm_read_binary_handle fsm_read_binary_file_multiple_init(char *filename) {

    struct io_buf_handle *iobh;
    fsm_read_binary_handle fsm_read_handle;

    iobh = io_init();
    if (io_gz_file_to_mem(iobh, filename) == 0) {
	io_free(iobh);
	return NULL;
    }
    fsm_read_handle = (void *) iobh;
    return(fsm_read_handle);
}

struct fsm *fsm_read_binary_file(char *filename) {
    char *net_name;
    struct fsm *net;
    struct io_buf_handle *iobh;
    iobh = io_init();
    if (io_gz_file_to_mem(iobh, filename) == 0) {
	io_free(iobh);
        return NULL;
    }
    net = io_net_read(iobh, &net_name);
    io_free(iobh);
    return(net);
}

int save_defined(struct defined_networks *def, char *filename) {
    struct defined_networks *d;
    gzFile outfile;
    if (def == NULL) {
        fprintf(stderr, "No defined networks.\n");
        return(0);
    }
    if ((outfile = gzopen(filename, "wb")) == NULL) {
        printf("Error opening file %s for writing.\n", filename);
        return(-1);
    }
    printf("Writing definitions to file %s.\n", filename);
    for (d = def; d != NULL; d = d->next) {
        strcpy(d->net->name, d->name);
        foma_net_print(d->net, outfile);
    }
    gzclose(outfile);
    return(1);
}

int load_defined(struct defined_networks *def, char *filename) {
    struct fsm *net;
    char *net_name;
    struct io_buf_handle *iobh;

    iobh = io_init();
    printf("Loading definitions from %s.\n",filename);
    if (io_gz_file_to_mem(iobh, filename) == 0) {
        fprintf(stderr, "File error.\n");
	io_free(iobh);
        return 0;
    }
    while ((net = io_net_read(iobh, &net_name)) != NULL) {
        add_defined(def, net, net_name);
    }
    io_free(iobh);
    return(1);
}

static INLINE int explode_line(char *buf, int *values) {
    int i, j, items;
    j = i = items = 0;
    for (;;) {
        for (i = j; *(buf+j) != ' ' && *(buf+j) != '\0'; j++) { }
        if (*(buf+j) == '\0') {
            *(values+items) = atoi(buf+i);
            items++;
            break;
        } else{
            *(buf+j) = '\0';
            *(values+items) = atoi(buf+i);
            items++;
            j++;
        }
    }
    return(items);
}

/* The file format we use is an extremely simple text format */
/* which is gzip compressed through libz and consists of the following sections: */

/* ##foma-net VERSION##*/
/* ##props## */
/* PROPERTIES LINE */
/* ##sigma## */
/* ...SIGMA LINES... */
/* ##states## */
/* ...TRANSITION LINES... */ 
/* ##end## */

/* Several networks may be concatenated in one file */

/* The initial identifier is "##foma-net 1.0##" */
/* where 1.0 is the version number for the file format */
/* followed by the line "##props##" */
/* which is followed by a line of space separated integers */
/* which correpond to: */

/* arity arccount statecount linecount finalcount pathcount is_deterministic */
/* is_pruned is_minimized is_epsilon_free is_loop_free is_completed name  */

/* where name is used if defined networks are saved/loaded */

/* Following the props line, we accept anything (for future expansion) */
/* until we find ##sigma## */

/* the section beginning with "##sigma##" consists of lines with two fields: */
/* number string */
/* correponding to the symbol number and the symbol string */

/* the section beginning with "##states##" consists of lines of ASCII integers */
/* with 2-5 fields to avoid some redundancy in every line corresponding to a */
/* transition where otherwise state numbers would be unnecessarily repeated and */
/* out symbols also (if in = out as is the case for recognizers/simple automata) */

/* The information depending on the number of fields in the lines is as follows: */

/* 2: in target (here state_no is the same as the last mentioned one and out = in) */
/* 3: in out target (again, state_no is the same as the last mentioned one) */
/* 4: state_no in target final_state (where out = in) */
/* 5: state_no in out target final_state */

/* There is no harm in always using 5 fields; however this will take up more space */

/* As in struct fsm_state, states without transitions are represented as a 4-field: */
/* state_no -1 -1 final_state (since in=out for 4-field lines, out = -1 as well) */

/* AS gzopen will read uncompressed files as well, one can gunzip a file */
/* that contains a network and still read it */

struct fsm *io_net_read(struct io_buf_handle *iobh, char **net_name) {

    char buf[READ_BUF_SIZE];
    struct fsm *net;
    struct fsm_state *fsm;
    
    char *new_symbol;
    int i, items, new_symbol_number, laststate, lineint[5], *cm;
    int extras;
    char last_final = '1';

    if (io_gets(iobh, buf) == 0) {
        return NULL;
    }
    
    net = fsm_create("");

    if (strcmp(buf, "##foma-net 1.0##") != 0) {
	fsm_destroy(net);
        perror("File format error foma!\n");
        return NULL;
    }
    io_gets(iobh, buf);
    if (strcmp(buf, "##props##") != 0) {
        perror("File format error props!\n");
	fsm_destroy(net);
        return NULL;
    }
    /* Properties */
    io_gets(iobh, buf);
    extras = 0;
    sscanf(buf, "%i %i %i %i %i %lld %i %i %i %i %i %i %s", &net->arity, &net->arccount, &net->statecount, &net->linecount, &net->finalcount, &net->pathcount, &net->is_deterministic, &net->is_pruned, &net->is_minimized, &net->is_epsilon_free, &net->is_loop_free, &extras, buf);
    strcpy(net->name, buf);
    *net_name = xxstrdup(buf);
    io_gets(iobh, buf);

    net->is_completed = (extras & 3);
    net->arcs_sorted_in = (extras & 12) >> 2;
    net->arcs_sorted_out = (extras & 48) >> 4;

    /* Sigma */
    while (strcmp(buf, "##sigma##") != 0) { /* Loop until we encounter ##sigma## */
        if (buf[0] == '\0') {
	  printf("File format error at sigma definition!\n");
	  fsm_destroy(net);
	  return NULL;
        }
        io_gets(iobh, buf);
    }

    for (;;) {
        io_gets(iobh, buf);
        if (buf[0] == '#') break;
        if (buf[0] == '\0') continue;
        new_symbol = strstr(buf, " ");
	new_symbol[0] = '\0';
	new_symbol++;
	if (new_symbol[0] == '\0') {
	    sscanf(buf,"%i", &new_symbol_number);
	    sigma_add_number(net->sigma, "\n", new_symbol_number);
	} else {
	    sscanf(buf,"%i", &new_symbol_number);
	    sigma_add_number(net->sigma, new_symbol, new_symbol_number);
	}
    }

    /* States */
    if (strcmp(buf, "##states##") != 0) {
        printf("File format error!\n");
        return NULL;
    }
    net->states = xxmalloc(net->linecount*sizeof(struct fsm_state));
    fsm = net->states;
    laststate = -1;
    for (i=0; ;i++) {
        io_gets(iobh, buf);
        if (buf[0] == '#') break;

        /* scanf is just too slow here */

        //items = sscanf(buf, "%i %i %i %i %i",&lineint[0], &lineint[1], &lineint[2], &lineint[3], &lineint[4]);

        items = explode_line(buf, &lineint[0]);

        switch (items) {
        case 2:
            (fsm+i)->state_no = laststate;
            (fsm+i)->in = lineint[0];
            (fsm+i)->out = lineint[0];
            (fsm+i)->target = lineint[1];
            (fsm+i)->final_state = last_final;
            break;
        case 3:
            (fsm+i)->state_no = laststate;
            (fsm+i)->in = lineint[0];
            (fsm+i)->out = lineint[1];
            (fsm+i)->target = lineint[2];
            (fsm+i)->final_state = last_final;
            break;
        case 4:
            (fsm+i)->state_no = lineint[0];
            (fsm+i)->in = lineint[1];
            (fsm+i)->out = lineint[1];
            (fsm+i)->target = lineint[2];
            (fsm+i)->final_state = lineint[3];
            laststate = lineint[0];
            last_final = lineint[3];
            break;
        case 5:
            (fsm+i)->state_no = lineint[0];
            (fsm+i)->in = lineint[1];
            (fsm+i)->out = lineint[2];
            (fsm+i)->target = lineint[3];
            (fsm+i)->final_state = lineint[4];
            laststate = lineint[0];
            last_final = lineint[4];
            break;
        default:
            printf("File format error\n");
            return NULL;
        }
        if (laststate > 0) {
            (fsm+i)->start_state = 0;
        } else if (laststate == -1) {
            (fsm+i)->start_state = -1;
        } else {
            (fsm+i)->start_state = 1;
        }

    }
    if (strcmp(buf, "##cmatrix##") == 0) {
        cmatrix_init(net);
        cm = net->medlookup->confusion_matrix;
        for (;;) {
            io_gets(iobh, buf);
            if (buf[0] == '#') break;
            sscanf(buf,"%i", &i);
            *cm = i;
            cm++;
        }
    }
    if (strcmp(buf, "##end##") != 0) {
        printf("File format error!\n");
        return NULL;
    }
    return(net);
}

static int io_gets(struct io_buf_handle *iobh, char *target) {
    int i;
    for (i = 0; *((iobh->io_buf_ptr)+i) != '\n' && *((iobh->io_buf_ptr)+i) != '\0'; i++) {
        *(target+i) = *((iobh->io_buf_ptr)+i);
    }
    *(target+i) = '\0';
    if (*((iobh->io_buf_ptr)+i) == '\0')
    (iobh->io_buf_ptr) = (iobh->io_buf_ptr) + i;
    else
        (iobh->io_buf_ptr) = (iobh->io_buf_ptr) + i + 1;

    return(i);
}

int foma_net_print(struct fsm *net, gzFile outfile) {
    struct sigma *sigma;
    struct fsm_state *fsm;
    int i, maxsigma, laststate, *cm, extras;

    /* Header */
    gzprintf(outfile, "%s","##foma-net 1.0##\n");

    /* Properties */
    gzprintf(outfile, "%s","##props##\n");

    extras = (net->is_completed) | (net->arcs_sorted_in << 2) | (net->arcs_sorted_out << 4);
 
    gzprintf(outfile, 
	     "%i %i %i %i %i %lld %i %i %i %i %i %i %s\n", net->arity, net->arccount, net->statecount, net->linecount, net->finalcount, net->pathcount, net->is_deterministic, net->is_pruned, net->is_minimized, net->is_epsilon_free, net->is_loop_free, extras, net->name);
    
    /* Sigma */
    gzprintf(outfile, "%s","##sigma##\n");
    for (sigma = net->sigma; sigma != NULL && sigma->number != -1; sigma = sigma->next) {
        gzprintf(outfile, "%i %s\n",sigma->number, sigma->symbol);
    }

    /* State array */
    laststate = -1;
    gzprintf(outfile, "%s","##states##\n");
    for (fsm = net->states; fsm->state_no !=-1; fsm++) {
        if (fsm->state_no != laststate) {
            if (fsm->in != fsm->out) {
                gzprintf(outfile, "%i %i %i %i %i\n",fsm->state_no, fsm->in, fsm->out, fsm->target, fsm->final_state);
            } else {
                gzprintf(outfile, "%i %i %i %i\n",fsm->state_no, fsm->in, fsm->target, fsm->final_state);
            }
        } else {
            if (fsm->in != fsm->out) {
                gzprintf(outfile, "%i %i %i\n", fsm->in, fsm->out, fsm->target);
            } else {
                gzprintf(outfile, "%i %i\n", fsm->in, fsm->target);
            }
        }
        laststate = fsm->state_no;
    }
    /* Sentinel for states */
    gzprintf(outfile, "-1 -1 -1 -1 -1\n");

    /* Store confusion matrix */
    if (net->medlookup != NULL && net->medlookup->confusion_matrix != NULL) {

        gzprintf(outfile, "%s","##cmatrix##\n");
        cm = net->medlookup->confusion_matrix;
        maxsigma = sigma_max(net->sigma)+1;
        for (i=0; i < maxsigma*maxsigma; i++) {
            gzprintf(outfile, "%i\n", *(cm+i));
        }
    }

    /* End */
    gzprintf(outfile, "%s","##end##\n");
    return(1);
}

int net_print_att(struct fsm *net, FILE *outfile) {
    struct fsm_state *fsm;
    struct fsm_sigma_list *sl;
    int i, prev;

    fsm = net->states;
    sl = sigma_to_list(net->sigma);
    if (sigma_max(net->sigma) >= 0) {
        (sl+0)->symbol = g_att_epsilon;
    }
    for (i=0; (fsm+i)->state_no != -1; i++) {
        if ((fsm+i)->target != -1) {
            fprintf(outfile, "%i\t%i\t%s\t%s\n",(fsm+i)->state_no,(fsm+i)->target, (sl+(fsm+i)->in)->symbol, (sl+(fsm+i)->out)->symbol);            
        }
    }
    prev = -1;
    for (i=0; (fsm+i)->state_no != -1; prev = (fsm+i)->state_no, i++) {
        if ((fsm+i)->state_no != prev && (fsm+i)->final_state == 1) {
            fprintf(outfile, "%i\n",(fsm+i)->state_no);
        }
    }
    xxfree(sl);
    return(1);
}

static size_t io_get_gz_file_size(char *filename) {

    FILE    *infile;
    size_t    numbytes;
    unsigned char bytes[4];
    unsigned int ints[4], i;

    /* The last four bytes in a .gz file shows the size of the uncompressed data */
    infile = fopen(filename, "r");
    fseek(infile, -4, SEEK_END);
    fread(&bytes, 1, 4, infile);
    fclose(infile);
    for (i = 0 ; i < 4 ; i++) {
        ints[i] = bytes[i];
    }
    numbytes = ints[0] | (ints[1] << 8) | (ints[2] << 16 ) | (ints[3] << 24);
    return(numbytes);
}

static size_t io_get_regular_file_size(char *filename) {

    FILE    *infile;
    size_t    numbytes;

    infile = fopen(filename, "r");
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);
    fclose(infile);
    return(numbytes);
}


static size_t io_get_file_size(char *filename) {
    gzFile FILE;
    size_t size;
    FILE = gzopen(filename, "r");
    if (FILE == NULL) {
        return(0);
    }
    if (gzdirect(FILE) == 1) {
        gzclose(FILE);
        size = io_get_regular_file_size(filename);
    } else {
        gzclose(FILE);
        size = io_get_gz_file_size(filename);
    }
    return(size);
}

size_t io_gz_file_to_mem(struct io_buf_handle *iobh, char *filename) {

    size_t size;
    gzFile FILE;

    size = io_get_file_size(filename);
    if (size == 0) {
        return 0;
    }
    (iobh->io_buf) = xxmalloc((size+1)*sizeof(char));
    FILE = gzopen(filename, "rb");
    gzread(FILE, iobh->io_buf, size);
    gzclose(FILE);
    *((iobh->io_buf)+size) = '\0';
    iobh->io_buf_ptr = iobh->io_buf;
    return(size);
}

char *file_to_mem(char *name) {
    FILE    *infile;
    size_t    numbytes;
    char *buffer;
    infile = fopen(name, "r");
    if(infile == NULL) {
        printf("Error opening file '%s'\n",name);
        return NULL;
    }
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);
    fseek(infile, 0L, SEEK_SET);
    buffer = (char*)xxmalloc((numbytes+1) * sizeof(char));
    if(buffer == NULL) {
        printf("Error reading file '%s'\n",name);
        return NULL;
    }
    if (fread(buffer, sizeof(char), numbytes, infile) != numbytes) {
        printf("Error reading file '%s'\n",name);
        return NULL;
    }
    fclose(infile);
    *(buffer+numbytes)='\0';
    return(buffer);
}
