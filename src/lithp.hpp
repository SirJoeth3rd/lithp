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
#include <memory>

#include "tokenizer.hpp"

using std::string;

class lval;
class Engine;

struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

typedef std::shared_ptr<lval> lval_sptr;
typedef std::function<lval_sptr(lval_sptr, Engine*)> lithp_func;

class SymbolLookup {
public:
  SymbolLookup();
  ~SymbolLookup();
  lval_sptr operator[](const char* name);
  int count(const char* name);
  void push_namespace();
  void pop_namespace();
  void insert(const char*,lval_sptr);
  void insert_global(const char*, lval_sptr);
private:
  std::vector<std::map<const char*, lval_sptr,StrCompare>> table;
};

class Engine {
public:
  Engine();
  ~Engine();
  void parse_push(const string& expr);
  lval_sptr pop();
  void eval_top();
  lval_sptr call_func(lval_sptr);
  void eval(lval_sptr expr_head);
  void set_lval(lval_sptr x, lval_sptr y);
  lval_sptr fetch_symbol(const char* name);
  void set_symbol(lval_sptr val, const char* name);
  void set_symbol_global(lval_sptr val, const char* name);
  void push_namespace();
  void pop_namespace();
  void subscribe_func(lithp_func, const char* name);
  void subscribe_macro(lithp_func, const char* name);
  void subscribe_macro_no_re_eval(lithp_func, const char* name);


  
private:
  SymbolLookup symbol_table;
  std::stack<lval_sptr> exprs;
};

enum ltype {
  Int,
  Float,
  String,
  Symbol,
  List,
  Lambda,
  Nil
};

union ldata {
  int Int;
  float Float;
  char* String;
  const char* Symbol;

  ldata() = default;

  ldata(int x) {
    Int = x;
  }
};

class lval {
public:
  ~lval();
  lval_sptr next;
  lval_sptr prev;
  lval_sptr branch;
  ltype type;
  ldata data; //for static data
  lithp_func lambda;
  bool is_macro;
  bool re_eval;
  void set_lamdba(lithp_func);
  lval();
  lval(lval_sptr, lval_sptr, lval_sptr, ltype, ldata);
  lval(ltype, ldata);
  lval(ltype);
  lval_sptr operator&();
  void operator[](lval_sptr);
  void operator[](lval&);
  void operator>>(lval&);
  void operator>>(lval_sptr);
  void insert_next(lval_sptr);
  lval_sptr get_last();
  void insert_branch(lval_sptr);
  void replace(lval_sptr);
  lval_sptr copy();
};

lval_sptr parse_tokens(std::vector<Token>&);
void print_ast(lval_sptr, int x = 0);
