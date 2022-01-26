#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

typedef struct {
  int refs;
  int len;
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

Mat* alloc_mat(int len) {
  Mat* mat = malloc(sizeof(Mat));
  mat->refs = 0;
  mat->len = len;
  mat->data = malloc(sizeof(int) * len);
  return mat;
}

void free_maybe(Mat* mat) {
  if (mat->refs == 0) {
    free(mat);
  }
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
  for (int i = 0; i < mat->len; i++) {
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
  assert(a->len == b->len);
  Mat* res = alloc_mat(a->len);
  for (int i = 0; i < res->len; i++) {
    res->data[i] = a->data[i] + b->data[i];
  }
  free_maybe(a);
  free_maybe(b);
  push(res);
}

void read_expr() {
  read_mat();
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
