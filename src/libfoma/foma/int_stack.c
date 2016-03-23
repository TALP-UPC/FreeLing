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

#include <stdio.h>
#include <stdlib.h>
#include "foma.h"

#define MAX_STACK 2097152
#define MAX_PTR_STACK 2097152

static int a[MAX_STACK];
static int top = -1;

static void *ptr_stack[MAX_PTR_STACK];
static int ptr_stack_top = -1;

int ptr_stack_isempty() {
    return ptr_stack_top == -1;
}

void ptr_stack_clear() {
    ptr_stack_top = -1;
}

void *ptr_stack_pop() {
    return ptr_stack[ptr_stack_top--];
}

int ptr_stack_isfull() {
    return (ptr_stack_top == (MAX_PTR_STACK - 1));
}

void ptr_stack_push(void *ptr) {
    if (ptr_stack_isfull()) {
        fprintf(stderr, "Pointer stack full!\n");
        exit(1);
    }
    ptr_stack[++ptr_stack_top] = ptr;
}


int int_stack_isempty() {
  return top == -1;
}

void int_stack_clear() {
  top = -1;
}

int int_stack_find (int entry) {
  int i;
  if (int_stack_isempty()) {
    return 0;
  }
  for(i = 0; i <= top ; i++) {
    if (entry == a[i]) {
      return 1;
    }
  }
  return 0;
}

int int_stack_size () {
  return (top + 1);
}

void int_stack_push(int c) {
  if (int_stack_isfull()) {
    fprintf(stderr, "Stack full!\n");
    exit(1);
  }
  a[++top] = c;
}


int int_stack_pop() {
  return a[top--];
}

int int_stack_isfull() {
  return (top == (MAX_STACK - 1));
}
