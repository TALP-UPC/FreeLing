/*     Foma: a finite-state toolkit and library.                             */
/*     Copyright © 2008-2015 Mans Hulden                                     */

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
#include <string.h>
#include "foma.h"

unsigned char *int2utf8str(int codepoint);

static int hexstrtoint(char *str);

/* Removes trailing character c, as well as spaces and tabs */
char *remove_trailing(char *s, char c) {
    int i, len;
    len = strlen(s)-1;
    for (i = len; i>=0 ; i--) {
        if (*(s+i) != c && *(s+i) != ' ' && *(s+i) != '\t') {
            break;
        }
        *(s+i) = '\0';
    }
    return(s);
}

/* Remove trailing space and \t */
char *trim(char *string) {
    int i;
    if (string == NULL)
        return(string);
    for (i = strlen(string) - 1; i >=0; i--) {
        if (*(string+i) != ' ' && *(string+i) != '\t')
            break;
        *(string+i) = '\0';
    }
    return(string);
}

/* Reverses string in-place */
char *xstrrev(char *str) {
      char *p1, *p2;
      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

char *escape_string(char *string, char chr) {
    size_t i,j;
    char *newstring;
    for (i=0,j=0; i < strlen(string); i++) {
        if (string[i] == chr) {
            j++;
        }
    }
    if (j>0) {
        newstring = xxcalloc((strlen(string)+j),sizeof(char));
        for (i=0,j=0; i<strlen(string); i++, j++) {
            if (string[i] == chr) {
                newstring[j++] = '\\';
                newstring[j] = chr;
            } else {
                newstring[j] = string[i];
            }            
        }        
        return(newstring);
    } else {
        return(string);
    }
}

/* Substitute first \n for \0 */
void strip_newline(char *s) {
    int i, len;
    len = strlen(s);
    /* remove the null terminator */
    for (i = 0; i < len; i++ ) {
        if (s[i] == '\n' ) {
            s[i] = '\0';
            return;   
        }
    }
}
/* Removes initial and final quote, and decodes the string if it contains special chars */
void dequote_string(char *s) {
    int len, i, j;
    len = strlen(s);
    if (*s == 0x22 && *(s+len-1) == 0x22) {
        for (i = 1, j = 0; i<len-1; i++, j++) {
            *(s+j) = *(s+i);

        }
        *(s+j) = '\0';
        decode_quoted(s);
    }    
}

/* Decode quoted strings. This includes: */
/* Changing \uXXXX sequences to their unicode equivalents */

void decode_quoted(char *s) {
    int len, i, j, skip;
    unsigned char *unistr;
    
    len = strlen(s);
    for (i=0, j=0; i < len; ) {
        if (*(s+i) == 0x5c && len-i > 5 && *(s+i+1) == 0x75 && ishexstr(s+i+2)) {
            for (unistr=utf8code16tostr(s+i+2); *unistr; j++, unistr++) {
                *(s+j) = *unistr;
            }
            i += 6;
        } else {
            for(skip = utf8skip(s+i)+1; skip > 0; skip--) {
                *(s+j) = *(s+i);
                i++; j++;
            }
        }
    }
    *(s+j) = *(s+i);
}


/* Replace equal length substrings in s */
char *streqrep(char *s, char *oldstring, char *newstring) {
    char *ptr;
    int len;
    len = strlen(oldstring);

    while ((ptr = strstr(s, oldstring)) != NULL) {
        memcpy(ptr, newstring, len);
    }
    return(s);
}

int ishexstr (char *str) {
    int i;
    for (i=0; i<4; i++) {
	if ((*(str+i) > 0x2f && *(str+i) < 0x3a) || (*(str+i) > 0x40 && *(str+i) < 0x47) || (*(str+i) > 0x60 && *(str+i) < 0x67))
	    continue;
	return 0;
    }
    return 1;
}
int utf8strlen(char *str) {
    int i,j, len;
    len = strlen(str);
    for (i=0, j=0; *(str+i) != '\0' && i < len;j++ ) {
        i = i + utf8skip(str+i) + 1;
    }
    return j;
}

/* Checks if the next character in the string is a combining character     */
/* according to Unicode 7.0                                                */
/* i.e. codepoints 0300-036F  Combining Diacritical Marks                  */
/*                 1AB0-1ABE  Combining Diacritical Marks Extended         */
/*                 1DC0-1DFF  Combining Diacritical Marks Supplement       */
/*                 20D0-20F0  Combining Diacritical Marks for Symbols      */
/*                 FE20-FE2D  Combining Half Marks                         */
/* Returns number of bytes of char. representation, or 0 if not combining  */

int utf8iscombining(unsigned char *s) {
    if (*s == '\0' || *(s+1) == '\0')
	return 0;
    if (!(*s == 0xcc || *s == 0xcd || *s == 0xe1 || *s == 0xe2 || *s == 0xef))
	return 0;
    /* 0300-036F */
    if (*s == 0xcc && *(s+1) >= 0x80 && *(s+1) <= 0xbf)
	return 2;
    if (*s == 0xcd && *(s+1) >= 0x80 && *(s+1) <= 0xaf)
	return 2;
    if (*(s+2) == '\0')
	return 0;
    /* 1AB0-1ABE */
    if (*s == 0xe1 && *(s+1) == 0xaa && *(s+2) >= 0xb0 && *(s+2) <= 0xbe)
	return 3;
    /* 1DC0-1DFF */
    if (*s == 0xe1 && *(s+1) == 0xb7 && *(s+2) >= 0x80 && *(s+2) <= 0xbf)
	return 3;
    /* 20D0-20F0 */
    if (*s == 0xe2 && *(s+1) == 0x83 && *(s+2) >= 0x90 && *(s+2) <= 0xb0)
	return 3;
    /* FE20-FE2D */
    if (*s == 0xef && *(s+1) == 0xb8 && *(s+2) >= 0xa0 && *(s+2) <= 0xad)
	return 3;
    return 0;
}

int utf8skip(char *str) {
  unsigned char s;

  s = (unsigned char)(unsigned int) (*str);
  if (s < 0x80)
    return 0;
  if ((s & 0xe0) == 0xc0) {
    return 1;
  }
  if ((s & 0xf0) == 0xe0) {
    return 2;
  }
  if ((s & 0xf8) == 0xf0) {
    return 3;
  }
  return -1;
}

unsigned char *utf8code16tostr(char *str) {
  int codepoint;
  codepoint = (hexstrtoint(str) << 8) + hexstrtoint(str+2);
  return(int2utf8str(codepoint));
}

unsigned char *int2utf8str(int codepoint) {
  unsigned char *value;
  value = xxmalloc(sizeof(unsigned char)*5);

  if (codepoint < 0x80) {
    *(value) = (unsigned char)(codepoint);
    *(value+1) = 0;
    return(value);
  } else if (codepoint < 0x800) {
    *(value) = (0xc0 | (unsigned char)(codepoint >> 6));
    *(value+1) = (0x80 | (unsigned char)(codepoint & 0x3f));
    *(value+2) = 0;
    return(value);
  } else if (codepoint < 0x10000) {
    *(value) = (0xe0 | (unsigned char)(codepoint >> 12));
    *(value+1) = (0x80 | (unsigned char)((codepoint >> 6) & 0x3f));
    *(value+2) = (0x80 | (unsigned char)(codepoint & 0x3f));
    *(value+3) = 0;
    return(value);
  } else {
    return (0);
  }
}

int hexstrtoint(char *str) {
  int hex;

  if (*str > 0x60) {
    hex = (*str - 0x57) << 4; 
  } else if (*str > 0x40) {
    hex = (*str - 0x37) << 4; 
  } else {
    hex = (*str - 0x30) << 4;
  }
  if (*(str+1) > 0x60) {
    hex += (*(str+1) - 0x57); 
  } else if (*(str+1) > 0x40) {
    hex += (*(str+1) - 0x37); 
  } else {
    hex += (*(str+1) - 0x30);
  }
  return hex;
}
