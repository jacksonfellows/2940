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
#define rest(x) (x->cdr)

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

int SKIP_NEWLINES = 0;

void skip_whitespace() {
  int c;
  while (1) {
    c = getc(stdin);
    if (!(c == ' ' || (SKIP_NEWLINES && c == '\n'))) {
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

Cons* mat_to_cons(Mat* mat) {
  assert(cons_len(mat->shape) == 1);
  int len = first(mat->shape);
  Cons* x = NULL;
  for (int i = len-1; i >= 0; i--) {
    x = cons(mat->data[i], x);
  }
  return x;
}

Cons* _read_mat(int offset) {
  Cons* row_shape = NULL;
  int row_size = 1;
  for (int len = 0; ; len++) {
    skip_whitespace();
    if (!parse_int(&temp_mat[len*row_size + offset])) {
      int c = getc(stdin);
      if (c == '[') {
        Cons* shape = _read_mat(len*row_size + offset);
        if (len == 0) {
          row_shape = shape;
          row_size = cons_product(row_shape);
        } else {
          assert(cons_eq(row_shape, shape));
          free_cons(shape);
        }
        assert(getc(stdin) == ']');
      } else {
        ungetc(c, stdin);
        return cons(len, row_shape);
      }
    }
  }
}

void read_mat() {
  skip_whitespace();
  int c = getc(stdin);
  if (c == '[') {
    SKIP_NEWLINES = 1;
    Cons* shape = _read_mat(0);
    assert(getc(stdin) == ']');
    SKIP_NEWLINES = 0;
    Mat* mat = alloc_mat(shape);
    memcpy(mat->data, &temp_mat, sizeof(int) * cons_product(shape));
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

void _print_row(int* data, int len) {
  for (int i = 0; i < len; i++) {
    if (i != 0) {
      printf(" ");
    }
    printf("%d", data[i]);
  }
  printf("]");
}

void _print_mat(int* data, Cons* shape, int padding, int outer);

void _print_mat_last(int* data, Cons* shape, int padding, int outer) {
  printf("[");
  int rank = cons_len(shape);
  int stride;
  switch (rank) {
  case 0:
    assert(0);
    break;
  case 1:
    _print_row(data, first(shape));
    break;
  default:
    stride = cons_product(rest(shape));
    for (int i = 0; i < first(shape); i++) {
      if (i != 0) {
        for (int j = 0; j < padding - rank + 1; j++) {
          printf(" ");
        }
      }
      if (i == first(shape) - 1) {
        _print_mat_last(&data[i*stride], rest(shape), padding, outer);
        printf("]");
      } else {
        _print_mat(&data[i*stride], rest(shape), padding, 0);
        printf("\n");
      }
    }
    break;
  }
}

void _print_mat(int* data, Cons* shape, int padding, int outer) {
  printf("[");
  int rank = cons_len(shape);
  int stride;
  switch (rank) {
  case 0:
    assert(0);
    break;
  case 1:
    _print_row(data, first(shape));
    break;
  default:
    stride = cons_product(rest(shape));
    for (int i = 0; i < first(shape); i++) {
      if (i != 0) {
        for (int j = 0; j < padding - rank + 1; j++) {
          printf(" ");
        }
      }
      if (i == first(shape) - 1) {
        _print_mat_last(&data[i*stride], rest(shape), padding, outer);
        printf("]");
        if (!outer) {
          for (int j = 0; j < rank-1; j++) {
            printf("\n");
          }
        }
      } else {
        _print_mat(&data[i*stride], rest(shape), padding, 0);
        printf("\n");
      }
    }
    break;
  }
}

void print_mat() {
  Mat* mat = pop();
   int rank = cons_len(mat->shape);
   if (rank == 0) {
     printf("%d", mat->data[0]);
   } else {
     _print_mat(mat->data, mat->shape, rank, 1);
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

void reshape() {
  Mat* mat = pop();
  Mat* new_shape_mat = pop();
  Cons* new_shape = mat_to_cons(new_shape_mat);
  assert(cons_product(mat->shape) == cons_product(new_shape));
  free_cons(mat->shape);
  mat->shape = new_shape;
  push(mat);
  free_maybe(new_shape_mat);
}

void read_expr();

void read_atom() {
  skip_whitespace();
  int c = getc(stdin);
  if (c == '(') {
    skip_whitespace();
    read_expr();
    skip_whitespace();
    assert(getc(stdin) == ')');
  } else {
    ungetc(c, stdin);
    read_mat();
  }
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
    c = getc(stdin);
    if (c == 'a') {
      assert(getc(stdin) == 'n');
      assert(getc(stdin) == 'k');
      read_term();
      get_rank();
    } else {
      assert(c == 'e');
      assert(getc(stdin) == 's');
      assert(getc(stdin) == 'h');
      assert(getc(stdin) == 'a');
      assert(getc(stdin) == 'p');
      assert(getc(stdin) == 'e');
      read_term();
      read_term();
      reshape();
    }
  } else {
    ungetc(c, stdin);
    read_atom();
  }
}

void read_expr() {
  read_term();
  skip_whitespace();
  int c = getc(stdin);
  if (c == '+') {
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
