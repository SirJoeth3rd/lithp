#include "lithp.hpp"
#include <memory>

//TODO: how to handle f() 

string lval_type_to_string(lval_type type) {
  switch (type) {
  case Symbol:
    return "Symbol";
    break;
  case String:
    return "String";
    break;
  case Float:
    return "Float";
    break;
  case Int:
    return "Int";
    break;
  case Lambda:
    return "Lmabda";
    break;
  case Nil:
    return "Nil";
    break;
  case List:
    return "List";
    break;
  }
  return "TYPE HAS NO STRING REPPRESENTATION, PLEASE INSERT";
}

void newline() {
  std::cout << std::endl;
}

template <typename T>
std::shared_ptr<T> lnode<T>::get_next() {
  return next;
}

template <typename T>
bool lnode<T>::has_next() {
  return next;
}

template <typename T>
void lnode<T>::insert_next(std::shared_ptr<T> nxt) {
  next = nxt;
  nxt->prev = this->shared_from_this();
}

template <typename T>
void lnode<T>::remove_next() {
  std::shared_ptr<T> tmp = next;
  if (next) {
    next = nullptr;
    tmp->prev.reset();
  }
}

template <typename T>
std::shared_ptr<T> lnode<T>::get_prev() {
  return prev.lock();
}

template <typename T>
void lnode<T>::insert_prev(std::shared_ptr<T> prv) {
  prev = prv;
  prev.lock()->next = this->shared_from_this();
}

template <typename T>
void lnode<T>::insert_prev_branch(std::shared_ptr<T> prv) {
  prv->insert_branch(this->shared_from_this());
}

template <typename T>
bool lnode<T>::has_prev() {
  return prev.expired();
}

template <typename T>
void lnode<T>::remove_prev() {
  if (!prev.expired()) {
    std::shared_ptr<T> tmp = prev.lock();
    if (tmp->get_next() == this->shared_from_this()) {
      tmp->remove_next();
    } else {
      tmp->remove_branch();
    }
    prev.reset();
  }
}

template <typename T>
std::shared_ptr<T> lnode<T>::get_branch() {
  return branch;
}

template <typename T>
void lnode<T>::remove_branch() {
  std::shared_ptr<T> tmp = branch;
  if (branch) {
    branch = nullptr;
    tmp->prev.reset();
  }
}

template <typename T>
void lnode<T>::insert_branch(std::shared_ptr<T> br) {
  if (branch) {
    branch->remove_prev();
  }
  branch = br;
  br->prev = this->shared_from_this();
}

template <typename T>
bool lnode<T>::has_branch() {
  return (bool)branch;
}

template <typename T>
void lnode<T>::replace_in_list(std::shared_ptr<T> ln) {
  std::shared_ptr<T> prev = get_prev();
  std::shared_ptr<T> next = get_next();

  if (prev) prev->insert_next(ln);
  if (next) next->insert_next(ln);
}

template <typename T>
void lnode<T>::replace_in_list_with_branch(std::shared_ptr<T> ln) {
  replace_in_list(ln);
  std::shared_ptr<T> b = get_branch();
  if (b) ln->insert_branch(b);
}

 
lval::lval() {
  type = Nil;
}

lval::lval(lval_type tp) {
  type = tp;
}

lval::lval(lval_type tp, lval_data dat) {
  type = tp;
  data = dat;
}

lval::lval(lval_type tp, lval_data dat, lval_sptr n, lval_sptr p, lval_sptr b) {
  type = tp;
  data = dat;
  next = n;
  prev = p;
  branch = b;
}

lval::lval(lval_sptr l) {
  *this = *l; 
}

void lval::set_macro(bool ismacro) {
  is_macro = ismacro;
}

lval_sptr lval::operator&() {
  return shared_from_this();
}

lval lval::clone() {
  lval clone;
  clone.data = data;
  clone.type = type;
  clone.is_macro = is_macro;
  return clone;
}

lval_sptr lval::clone_recurse() {
  lval_sptr self = shared_from_this();
  lval_sptr clone;
  clone = std::make_shared<lval>(self->clone());
  lval_sptr start = clone;
  if (branch) {
    lval_sptr b_clone = std::make_shared<lval>(branch->clone_recurse());
    clone->insert_branch(b_clone);
  }
  while (self->next) {
    if (self->branch) {
      clone->insert_branch(std::make_shared<lval>(branch->clone_recurse()));
    }
    clone->insert_next(std::make_shared<lval>(self->get_next()->clone()));
    clone = clone->get_next();
    self = self->get_next();
  }
  return start;
}

void lval::print_content() {
  std::cout << lval_type_to_string(type);
  switch (type) {
  case Symbol:
  case String:
    std::cout << ":";
    std::cout << std::get<string>(data);
    break;
  case Int:
    std::cout << ":";
    std::cout << std::get<int>(data);
    break;
  case Float:
    std::cout << ":";
    std::cout << std::get<double>(data);
    break;
  case Lambda:
    std::cout << ":";
    std::cout << (is_macro ? "macro" : "function");
    break;
  default:
    break;
  }
}

void lval::print_address() {
  std::cout << this;
}

void print_lval_and_address(lval_sptr head, int indent) {
  std::cout << string(indent, ' ');
  head->print_content();
  std::cout << " @ ";
  head->print_address();
  std::cout << std::endl;  
}

void print_tree(lval_sptr head, int indent) {
  do {
    print_lval_and_address(head, indent);
    if (head->has_branch()) {
      print_tree(head->get_branch(), indent + 2);
    }
    head = head->get_next();
  } while (head);
}

/*
Now comes the symbol table implementation, which allows for 
pushing and popping namespaces
*/

lval_symbol_table::lval_symbol_table() {
  internal_map.push_back(std::map<string,lval_sptr>());
}

lval_sptr lval_symbol_table::operator[](string sym) {
  for (auto m = internal_map.rbegin(); m != internal_map.rend(); m++) {
    if (m->count(sym)) {
      return (*m)[sym];
    }
  }
  return nullptr;
}

int lval_symbol_table::count(string sym) {
  for (auto m = internal_map.rbegin(); m != internal_map.rend(); m++) {
    if (m->count(sym)) {
      return 1;
    }
  }
  return 0;
}

void lval_symbol_table::set(string sym, lval_sptr l) {
  internal_map.back()[sym] = l;
}

void lval_symbol_table::set_global(string sym, lval_sptr l) {
  std::map<string, lval_sptr> m = internal_map.front();
  m[sym] = l;
}

void lval_symbol_table::push_namespace() {
  std::map<string, lval_sptr> m;
  internal_map.push_back(m);
}

void lval_symbol_table::pop_namespace() {
  internal_map.pop_back();
}

/*
From here on is Engine implementation which is the class
from which all of lithp is accessed
*/

void lithp_engine::parse_push(const string &expr_str) {
  lval_sptr h = parse_string(expr_str);
  exprs.push(h);
}

lval_sptr lithp_engine::pop() {
  lval_sptr top = exprs.top();
  exprs.pop();
  return top;
}

void lithp_engine::eval_top() {
  lval_sptr top = pop();
  eval(top);
  exprs.push(top);
}

void lithp_engine::eval(lval_sptr l) {
  string symbol;
  lval_sptr result;
  while (l) {
    switch (l->type) {
    case Symbol:
      symbol = std::get<string>(l->data);
      if (symbol_table.count(symbol)) {
	result = symbol_table[symbol];
	l->replace_in_list_with_branch(result);
	*l = *result;
	continue;
      } else {
	break;
      }
    case Lambda:
      if (!l->has_branch()) {
	break;
	// f() will get parsed to f -> nil (in branch)
	// so no need to worry that an empty func won't be called
      }
      if (l->is_macro) {
	result = call_func(l, this);
	l->replace_in_list(result);
	*l = *result;
	//always re-eval
      } else {
	eval(l->get_branch());
	result = call_func(l, this);
	l->replace_in_list(result);
	*l = *result;
      }
      continue;
    default:
      break;
    }
    l = l->get_next();
  }
}

lval_sptr lithp_engine::call_func(lval_sptr l , lithp_engine* engine) {
  lval_sptr result = nullptr;
  if (std::holds_alternative<lithp_func>(l->data)) {
    result = std::get<lithp_func>(l->data)(l->get_branch(), engine);
  }
  return result;
}

void lithp_engine::set_symbol(string sym, lval_sptr l) {
  symbol_table.set(sym, l);
}

void lithp_engine::set_global(string sym, lval_sptr l) {
  symbol_table.set_global(sym, l);
}

void lithp_engine::push_namespace() {
  symbol_table.push_namespace();
}

void lithp_engine::pop_namespace() {
  symbol_table.pop_namespace();
}

/*
  Testing starts here
*/

lval_sptr plus(lval_sptr b, lithp_engine* e) {
  int result = 0;
  while (b) {
    if (b->type == Int) {
      result += std::get<int>(b->data);
    }
    b = b->get_next();
  }
  return std::make_shared<lval>(lval(Int,result));
}

lval_sptr mul(lval_sptr b, lithp_engine* e) {
  int result = 1;
  while (b) {
    if (b->type == Int) {
      result *= std::get<int>(b->data);
    }
    b = b->get_next();
  }
  return std::make_shared<lval>(lval(Int,result));
}

lval_sptr with(lval_sptr b, lithp_engine* e) {
  //expect shape with[[[symbol, expr]...],expr]
  lval_sptr args_lists = b->get_branch();
  lval_sptr expr = b->get_next();
  e->push_namespace();
  while (args_lists) {
    lval_sptr sym = args_lists->get_branch();
    lval_sptr expr = sym->get_next();
    e->eval(expr);
    e->set_symbol(std::get<string>(sym->data), expr);
    args_lists = args_lists->get_next();
  }
  e->eval(expr);
  e->pop_namespace();
  return expr;
}

class lfunc_store {
public:
  lval_sptr symbols;
  lval_sptr body;
  
  lval_sptr operator()(lval_sptr args, lithp_engine* engine) {
    //expect shape function([symbol,...], expr)
    //mathced with f(expr,...)
    lval_sptr syms = symbols;
    
    lval_sptr With = std::make_shared<lval>(lval(Lambda, with));
    With->is_macro = true;

    lval_sptr arg_lists = std::make_shared<lval>(lval(List));
    arg_lists->insert_branch(std::make_shared<lval>(lval(List)));
    lval_sptr alist = arg_lists->get_branch();
    
    while (syms) {
      alist->insert_branch(std::make_shared<lval>(syms->clone()));
      alist->get_branch()->insert_next(std::make_shared<lval>(args->clone()));
      alist->insert_next(std::make_shared<lval>(lval(List)));
      alist = alist->get_next();
      syms = syms->get_next();
      args = args->get_next();
    }

    //last alist should be removed
    if (!(alist->get_prev() == arg_lists)) {
      alist->remove_prev();
    }

    With->insert_branch(arg_lists);
    With->get_branch()->insert_next(body->clone_recurse()); //very inefficient

    engine->eval(With);

    return With;
  }

  lfunc_store(lval_sptr Symbols, lval_sptr Body) {
    symbols = Symbols->clone_recurse();
    body = Body->clone_recurse();
  }
};

lval_sptr function(lval_sptr h, lithp_engine* engine) {
  //expect shape function([s,...], expr)
  lfunc_store func_store = lfunc_store(h->get_branch(), h->get_next());
  return std::make_shared<lval>(lval(Lambda, func_store));
}

lval_sptr set(lval_sptr h, lithp_engine* engine) {
  //expect set(symbol, expr)
  engine->set_symbol(std::get<string>(h->data),h->get_next());
  return std::make_unique<lval>(lval()); //just return nil
}

int main() {
  lithp_engine engine;
  
  lval_sptr Plus = std::make_shared<lval>(lval(Lambda,plus));
  lval_sptr Mul = std::make_shared<lval>(lval(Lambda,mul));
  lval_sptr With = std::make_shared<lval>(lval(Lambda, with));
  lval_sptr Function = std::make_shared<lval>(lval(Lambda, function));
  lval_sptr Set = std::make_shared<lval>(lval(Lambda, set));
  
  With->set_macro(true);
  Function->set_macro(true);

  engine.set_symbol("plus", Plus);
  engine.set_symbol("mul", Mul);
  engine.set_symbol("with", With);
  engine.set_symbol("function", Function);
  engine.set_symbol("set", Set);

  engine.parse_push("set(f,function([x,y],plus(x,y)))");
  engine.eval_top();
  engine.parse_push("set(g,function([x,y],mul(x,y)))");
  engine.eval_top();
  engine.parse_push("f(2,g(3,3))");
  engine.eval_top();
  lval_sptr top = engine.pop();
  print_tree(top);
}

