#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

typedef struct _Cons {
  int car;
  struct _Cons* cdr;
} Cons;

#define first(x) (x->car)
#define second(x) (x->cdr->car)
#define rest(x) (x->cdr)

int n_cons = 0;

Cons* cons(int car, Cons* cdr) {
  Cons* x = malloc(sizeof(Cons));
  n_cons++;
  x->car = car;
  x->cdr = cdr;
  return x;
}

void free_cons(Cons* x) {
  if (x != NULL) {
    free_cons(x->cdr);
    free(x);
    n_cons--;
  }
}

typedef struct {
  int refs;
  Cons* shape;
  double* data;
} Mat;

Mat* stack[256];
int stack_top = 0;

#define HISTORY_SIZE 16
Mat* history[HISTORY_SIZE];
int history_i = 0;

void push(Mat* mat) {
  stack[stack_top++] = mat;
  mat->refs++;
}

Mat* pop() {
  stack[stack_top-1]->refs--;
  return stack[--stack_top];
}

double temp_mat[256];

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

double parse_double(double* dest) {
  return 1 == scanf("%lf", dest);
}

int cons_product(Cons* cons) {
  if (cons == NULL) {
    return 1;
  }
  return cons->car * cons_product(cons->cdr);
}

int n_mat = 0;

Mat* alloc_mat(Cons* shape) {
  Mat* mat = malloc(sizeof(Mat));
  n_mat++;
  mat->refs = 0;
  mat->shape = shape;
  int size = cons_product(shape);
  mat->data = malloc(sizeof(double) * size);
  return mat;
}

void free_maybe(Mat* mat) {
  if (mat->refs == 0) {
    free_cons(mat->shape);
    free(mat->data);
    free(mat);
    n_mat--;
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
    if (!parse_double(&temp_mat[len*row_size + offset])) {
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
    memcpy(mat->data, &temp_mat, sizeof(double) * cons_product(shape));
    push(mat);
  } else {
    ungetc(c, stdin);
    double x;
    assert(parse_double(&x));
    Mat* mat = alloc_mat(NULL);
    mat->data[0] = x;
    push(mat);
  }
}

#define printdouble(x) (printf("%g", x))

void _print_row(double* data, int len) {
  for (int i = 0; i < len; i++) {
    if (i != 0) {
      printf(" ");
    }
    printdouble(data[i]);
  }
  printf("]");
}

void _print_mat(double* data, Cons* shape, int padding, int outer);

void _print_mat_last(double* data, Cons* shape, int padding, int outer) {
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

void _print_mat(double* data, Cons* shape, int padding, int outer) {
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
    printdouble(mat->data[0]);
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

void sub_mats() {
  Mat* b = pop();
  Mat* a = pop();
  assert(cons_eq(a->shape, b->shape));
  Mat* res = alloc_mat(cons_copy(a->shape));
  int size = cons_product(res->shape);
  for (int i = 0; i < size; i++) {
    res->data[i] = a->data[i] - b->data[i];
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
  Mat* new_mat = alloc_mat(new_shape);
  int size = cons_product(new_shape);
  memcpy(new_mat->data, mat->data, sizeof(double) * size);
  push(new_mat);
  free_maybe(new_shape_mat);
  free_maybe(mat);
}

void make_range() {
  Mat* upper = pop();
  Mat* lower = pop();
  int upper_rank = cons_len(upper->shape);
  int lower_rank = cons_len(lower->shape);
  assert(upper_rank == 0 && lower_rank == 0);
  int upper_n = upper->data[0];
  int lower_n = lower->data[0];
  int size = upper_n - lower_n + 1;
  Mat* new = alloc_mat(cons(size, NULL));
  for (int i = 0; i < size; i++) {
    new->data[i] = lower_n++;
  }
  push(new);
  free_maybe(upper);
  free_maybe(lower);
}

void _rref(double* data, int rows, int cols) {
  int first_row = 0;
  while (first_row < rows - 1) {
    int pivot_col = 0;
    for (int col = 0; col < cols; col++) {
      int is_zero = 1;
      for (int row = first_row; row < rows; row++) {
        if (data[row*cols + col] != 0) {
          is_zero = 0;
          break;
        }
      }
      if (!is_zero) {
        pivot_col = col;
        break;
      }
    }
    int pivot_row;
    double max_pivot = 0;
    for (int row = first_row; row < rows; row++) {
      double abs_pivot = fabs(data[row*cols + pivot_col]);
      if (abs_pivot > max_pivot) {
        pivot_row = row;
        max_pivot = abs_pivot;
      }
    }
    if (pivot_row != first_row) {
      for (int col = 0; col < cols; col++) {
        double tmp = data[first_row*cols + col];
        data[first_row*cols + col] = data[pivot_row*cols + col];
        data[pivot_row*cols + col] = tmp;
      }
    }
    double pivot = data[first_row*cols + pivot_col];
    for (int row = first_row + 1; row < rows; row++) {
      double x = data[row*cols + pivot_col];
      if (x != 0) {
        double factor = -x / pivot;
        for (int col = pivot_col; col < cols; col++) {
          data[row*cols + col] += factor * data[first_row*cols + col];
        }
      }
    }
    first_row++;
  }
  int bottom_row = rows - 1;
  while (bottom_row >= 0) {
    int leading_col = -1;
    for (int col = 0; col < cols; col++) {
      if (data[bottom_row*cols + col] != 0) {
        leading_col = col;
        break;
      }
    }
    if (leading_col == -1) {
      bottom_row--;
      continue;
    }
    double lead = data[bottom_row*cols + leading_col];
    for (int row = bottom_row - 1; row >= 0; row--) {
      double x = data[row*cols + leading_col];
      if (x != 0) {
        double factor = -x / lead;
        for (int col = leading_col; col < cols; col++) {
          data[row*cols + col] += factor * data[bottom_row*cols + col];
        }
      }
    }
    double scale = 1 / lead;
    for (int col = leading_col; col < cols; col++) {
      data[bottom_row*cols + col] *= scale;
    }
    bottom_row--;
  }
}

void rref() {
  Mat* mat = pop();
  int rank = cons_len(mat->shape);
  assert(rank == 2);
  int rows = first(mat->shape);
  int cols = second(mat->shape);
  Mat* new = alloc_mat(cons_copy(mat->shape));
  int size = rows * cols;
  memcpy(new->data, mat->data, sizeof(double) * size);
  _rref(new->data, rows, cols);
  push(new);
  free_maybe(mat);
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
  } else if (c == '%') {
    int n = 1;
    while ((c = getc(stdin)) == '%') {
      n++;
    }
    ungetc(c, stdin);
    push(history[(history_i - n + HISTORY_SIZE) % HISTORY_SIZE]);
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
      c = getc(stdin);
      if (c == 'g') {
        assert(getc(stdin) == 'e');
        read_term();
        read_term();
        make_range();
      } else {
        assert(c == 'k');
        read_term();
        get_rank();
      }
    } else if (c == 'r') {
      assert(getc(stdin) == 'e');
      assert(getc(stdin) == 'f');
      read_term();
      rref();
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
  } else if (c == '-') {
    read_expr();
    sub_mats();
  } else {
    ungetc(c, stdin);
  }
}

void save_to_history() {
  Mat* mat = stack[stack_top - 1];
  if (history[history_i] != NULL) {
    history[history_i]->refs--;
    free_maybe(history[history_i]);
  }
  history[history_i] = mat;
  mat->refs++;
  history_i = (history_i + 1) % HISTORY_SIZE;
}

void cleanup() {
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (history[i] != NULL) {
      history[i]->refs--;
      free_maybe(history[i]);
    }
  }
  for (int i = 0; i < stack_top; i++) {
    stack[i]->refs--;
    free_maybe(stack[i]);
  }
  assert(n_mat == 0);
  assert(n_cons == 0);
}

int main(int argc, char** argv) {
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history[i] = NULL;
  }
  int is_term = isatty(fileno(stdin));
  while (1) {
    printf("> ");
    fflush(stdin);
    SKIP_NEWLINES = 1;
    skip_whitespace();
    SKIP_NEWLINES = 0;
    int c = getc(stdin);
    if (c == ':') {
      if (getc(stdin) == 'q') {
        cleanup();
        return 0;
      }
      assert(0);
    } else {
      ungetc(c, stdin);
    }
    read_expr();
    save_to_history();
    if (!is_term) {
      printf("\n");
    }
    print_mat();
    printf("\n");
  }
}
