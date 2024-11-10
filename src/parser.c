#include "tokenizer.c"
#include <stdbool.h>
#include <string.h>
#include "lithp.h"


bool expect(TokenType type, Token* tok) {
  if (tok->toktype != type) {
    return false;
  } else {
    return true;
  }
}

Lval* error_lval(char* error_msg) {
  
}


Lval* parse_expr(Token* token) {
  if (!expect(Symbol, token)) {
    
  }
}

void print_ast(Token* tokens);

