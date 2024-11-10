#include <stdlib.h>
#include "lithp.h"

const int INIT_SIZE = 1024;

typedef struct Lallocator {
  Lval* block;
  Lval* fop; // first open position
} Lallocator;

Lval* find_next_op(Lallocator* A, Lval* curr) {
  //todo need to check memory and realloc if needed
  while (curr->ltype) {
    curr++;
  }
  return curr;
}

void lalloc_init(Lallocator* A) {
  A->block = malloc(sizeof(Lval) * INIT_SIZE);
  A->fop = A->block;
}

Lval* spawn_lval(Lallocator* A) {
  Lval* fop = A->fop;
  A->fop = find_next_op(A, fop + 1);
  return fop;
}

Lval* create_next(Lallocator* A, Lval* curr) {
  Lval* nxt = spawn_lval(A);
  nxt->prv = curr;
  curr->nxt = nxt;
  return nxt;
}

Lval* create_child(Lallocator* A, Lval* curr) {
  Lval* cld = spawn_lval(A);
  curr->cld = cld;
  cld->prt = curr;
  return cld;
}

void remove(Lallocator* A, Lval* l) {
  l->ltype = Dead;
  if (l->cld) remove(A,l->cld);
  if (l->nxt) remove(A,l->nxt);
}
