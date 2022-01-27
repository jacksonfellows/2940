#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

typedef struct _Cons {
  int car;
  struct _Cons* cdr;
} Cons;

#define first(x) (x->car)

Cons* cons(int car, Cons* cdr) {
  Cons* x = malloc(sizeof(Cons));
  x->car = car;
  x->cdr = cdr;
  return x;
}

void free_cons(Cons* x) {
  if (x->cdr != NULL) {
    free_cons(x->cdr);
  }
  free(x);
}

typedef struct {
  int refs;
  Cons* shape;
  int* data;
} Mat;

Mat* stack[256];
int stack_top = 0;

void push(Mat* mat) {
  stack[stack_top++] = mat;
  mat->refs++;
}

Mat* pop() {
  stack[stack_top-1]->refs--;
  return stack[--stack_top];
}

int temp_mat[256];

int inside = 0;

void skip_whitespace() {
  int c;
  while (1) {
    c = getc(stdin);
    if (!(c == ' ' || (inside && c == '\n'))) {
      ungetc(c, stdin);
      break;
    }
  }
}

int parse_int(int* dest) {
  int c = getc(stdin);
  if (!isdigit(c)) {
    ungetc(c, stdin);
    return 0;
  }
  int x = 0;
  do {
    x = 10*x + (int)(c - '0');
  } while (isdigit(c = getc(stdin)));
  *dest = x;
  ungetc(c, stdin);
  return 1;
}

Mat* alloc_mat(int size) {
  Mat* mat = malloc(sizeof(Mat));
  mat->refs = 0;
  mat->shape = cons(size, NULL);
  mat->data = malloc(sizeof(int) * size);
  return mat;
}

void free_maybe(Mat* mat) {
  if (mat->refs == 0) {
    free_cons(mat->shape);
    free(mat->data);
    free(mat);
  }
}

int cons_len(Cons* cons) {
  if (cons == NULL) {
    return 0;
  }
  return 1 + cons_len(cons->cdr);
}

Mat* cons_to_mat(Cons* cons) {
  int len = cons_len(cons);
  Mat* mat = alloc_mat(len);
  for (int i = 0; cons != NULL; i++, cons = cons->cdr) {
    mat->data[i] = cons->car;
  }
  return mat;
}

void read_mat() {
  skip_whitespace();
  assert(getc(stdin) == '[');
  inside = 1;
  int i;
  for (i = 0; ; i++) {
    skip_whitespace();
    if (!parse_int(&temp_mat[i]))
      break;
  }
  assert(getc(stdin) == ']');
  inside = 0;
  Mat* mat = alloc_mat(i);
  memcpy(mat->data, &temp_mat, sizeof(int) * i);
  push(mat);
}

void print_mat() {
  Mat* mat = pop();
  printf("[");
  for (int i = 0; i < first(mat->shape); i++) {
    if (i != 0) {
      printf(" ");
    }
    printf("%d", mat->data[i]);
  }
  printf("]");
  free_maybe(mat);
}

void add_mats() {
  Mat* a = pop();
  Mat* b = pop();
  assert(first(a->shape) == first(b->shape));
  Mat* res = alloc_mat(first(a->shape));
  for (int i = 0; i < first(res->shape); i++) {
    res->data[i] = a->data[i] + b->data[i];
  }
  free_maybe(a);
  free_maybe(b);
  push(res);
}

void get_shape() {
  Mat* mat = pop();
  Mat* shape = cons_to_mat(mat->shape);
  push(shape);
  free_maybe(mat);
}

void read_term() {
  skip_whitespace();
  int c = getc(stdin);
  if (c == 's') {
    assert(getc(stdin) == 'h');
    assert(getc(stdin) == 'a');
    assert(getc(stdin) == 'p');
    assert(getc(stdin) == 'e');
    read_term();
    get_shape();
  } else {
    ungetc(c, stdin);
    read_mat();
  }
}

void read_expr() {
  read_term();
  skip_whitespace();
  int c = getc(stdin);
  if (c == '+') {
    inside = 1;
    read_expr();
    inside = 0;
    add_mats();
  } else {
    ungetc(c, stdin);
  }
}

int main(int argc, char** argv) {
  while (1) {
    printf("> ");
    fflush(stdin);
    read_expr();
    print_mat();
    printf("\n");
  }
  return 0;
}
