#include "tokenizer.c"

typedef struct Lval {
  struct Lval* next;
  struct Lval* prev;
  struct Lval* child;
  struct Lval* parent;
  enum {
    Symbol,
    Number,
    String
  } ltype;
  union {
    int integer;
    const char* symbol;
    char* string;
  };
} Lval;

Lval* parse(Token* tokens) {
  //expect expr = symbol [expr..] | number | string 
  while (tokens->toktype != end) {
    
  }
  return 0; //lol
}

void print_ast(Token* tokens);

