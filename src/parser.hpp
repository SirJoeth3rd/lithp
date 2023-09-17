#pragma once

#include <cctype>
#include <string>
#include <vector>
#include <typeinfo>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <stack>
#include <map>

#include "tokenizer.hpp"

struct lval;
class Engine;

union ldata {
  int Int;
  float Float;
  char* String;
  const char* Symbol;
  std::function<lval*(lval*, Engine*)> Lambda;
};

enum ltype {
  Int,
  Float,
  String,
  Func,
  Symbol,
  List,
  Lambda
};

/*
struct lval {
  lval* next;
  lval* prev;
  lval* branch;
  ldata data;
  ltype type;
};
*/

class lval {
public:
  lval();
  ~lval();
  lval* next;
  lval* prev;
  lval* branch;
  ltype type;
  ldata data;
};

std::string ltype_to_string(ltype type);
lval* parse_tokens(std::vector<Token>& tokens);
void print_ast(lval* head, int indent = 0);
