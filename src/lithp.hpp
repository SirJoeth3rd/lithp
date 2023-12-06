#pragma once

#include <memory>
#include <variant>
#include <functional>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <stack>

using std::string;

class lval;
class lithp_engine;

typedef std::shared_ptr<lval> lval_sptr;
typedef std::weak_ptr<lval> lval_wptr;
typedef std::function<lval_sptr(lval_sptr, lithp_engine*)> lithp_func;
typedef std::variant<std::monostate, int, double, string, lithp_func> lval_data;

enum lval_type {
  Symbol = 1,
  String = 2,
  Float = 4,
  Int = 8,
  Lambda = 16,
  Nil = 32,
  List = 64
};

string lval_type_to_string(lval_type);
void newline();
void print_tree(lval_sptr, int indent = 0);
lval_sptr parse_string(const string&);

template <typename T>
class lnode : public std::enable_shared_from_this<T> {
protected:
  std::shared_ptr<T> next = nullptr;
  std::shared_ptr<T> branch = nullptr;
  std::weak_ptr<T> prev;
public:
  std::shared_ptr<T> operator&();
  std::shared_ptr<T> get_next();
  bool has_next();
  void insert_next(std::shared_ptr<T>);
  void remove_next();
  std::shared_ptr<T> get_prev();
  bool has_prev();
  void insert_prev(std::shared_ptr<T>);
  void insert_prev_branch(std::shared_ptr<T>);
  void remove_prev();
  void insert_branch(std::shared_ptr<T>);
  std::shared_ptr<T> get_branch();
  bool has_branch();
  void remove_branch();
  void replace_in_list(std::shared_ptr<T>);
  void replace_in_list_with_branch(std::shared_ptr<T>);
};

class lval : public lnode<lval> {
public:
  lval_data data;
  lval_type type;
  bool is_macro = false;

  lval();
  lval(lval_type tp);
  lval(lval_type, lval_data);
  lval(lval_type, lval_data, lval_sptr, lval_sptr, lval_sptr);
  lval(lval_sptr);
  void set_macro(bool);
  lval_sptr operator&();
  lval clone();
  lval_sptr clone_recurse();
  void print_content();
  void print_address();
  void print();
};

//TODO support for named namespaces
class lval_symbol_table {
public:
  lval_symbol_table();
  lval_sptr operator[](string);
  int count(string);
  void set(string, lval_sptr);
  void set_global(string, lval_sptr);
  void push_namespace();
  void pop_namespace();
  int count();
private:
  std::vector<std::map<string, lval_sptr>> internal_map;
};

class lithp_engine {
public:
  void parse_push(const string&);
  lval_sptr pop();
  void eval_top();
  void eval(lval_sptr);
  lval_sptr call_func(lval_sptr, lithp_engine*);
  void set_symbol(string, lval_sptr);
  void set_global(string, lval_sptr);
  void push_namespace();
  void pop_namespace();
private:
  std::stack<lval_sptr> exprs;
  lval_symbol_table symbol_table = lval_symbol_table();
};
