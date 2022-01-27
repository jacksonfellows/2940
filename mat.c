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
  if (x != NULL) {
    free_cons(x->cdr);
    free(x);
  }
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

int cons_product(Cons* cons) {
  if (cons == NULL) {
    return 1;
  }
  return cons->car * cons_product(cons->cdr);
}

Mat* alloc_mat(Cons* shape) {
  Mat* mat = malloc(sizeof(Mat));
  mat->refs = 0;
  mat->shape = shape;
  int size = cons_product(shape);
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

Cons* cons_copy(Cons* x) {
  if (x == NULL) {
    return NULL;
  }
  return cons(x->car, cons_copy(x->cdr));
}

int cons_len(Cons* cons) {
  if (cons == NULL) {
    return 0;
  }
  return 1 + cons_len(cons->cdr);
}

int cons_eq(Cons* a, Cons* b) {
  if (a == NULL && b == NULL)
    return 1;
  if (a == NULL || b == NULL)
    return 0;
  return a->car == b->car && cons_eq(a->cdr, b->cdr);
}

Mat* cons_to_mat(Cons* x) {
  int len = cons_len(x);
  Mat* mat = alloc_mat(cons(len, NULL));
  for (int i = 0; x != NULL; i++, x = x->cdr) {
    mat->data[i] = x->car;
  }
  return mat;
}

void read_mat() {
  skip_whitespace();
  int c = getc(stdin);
  if (c == '[') {
    inside = 1;
    int i;
    for (i = 0; ; i++) {
      skip_whitespace();
      if (!parse_int(&temp_mat[i]))
        break;
    }
    assert(getc(stdin) == ']');
    inside = 0;
    Mat* mat = alloc_mat(cons(i, NULL));
    memcpy(mat->data, &temp_mat, sizeof(int) * i);
    push(mat);
  } else {
    ungetc(c, stdin);
    int x;
    assert(parse_int(&x));
    Mat* mat = alloc_mat(NULL);
    mat->data[0] = x;
    push(mat);
  }
}

void print_mat() {
  Mat* mat = pop();
  int rank = cons_len(mat->shape);
  switch (rank) {
  case 0:
    printf("%d", mat->data[0]);
    break;
  case 1:
    printf("[");
    for (int i = 0; i < first(mat->shape); i++) {
      if (i != 0) {
        printf(" ");
      }
      printf("%d", mat->data[i]);
    }
    printf("]");
    break;
  default:
    assert(0);
    break;
  }
  free_maybe(mat);
}

void add_mats() {
  Mat* a = pop();
  Mat* b = pop();
  assert(cons_eq(a->shape, b->shape));
  Mat* res = alloc_mat(cons_copy(a->shape));
  int size = cons_product(res->shape);
  for (int i = 0; i < size; i++) {
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

void get_rank() {
  Mat* mat = pop();
  Mat* rank = alloc_mat(NULL);
  rank->data[0] = cons_len(mat->shape);
  push(rank);
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
  } else if (c == 'r') {
    assert(getc(stdin) == 'a');
    assert(getc(stdin) == 'n');
    assert(getc(stdin) == 'k');
    read_term();
    get_rank();
  } else {
    ungetc(c, stdin);
    read_mat();
  }
}

void read_expr() {
  read_term();
  inside = 0;
  skip_whitespace();
  int c = getc(stdin);
  if (c == '+') {
    inside = 1;
    read_expr();
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
