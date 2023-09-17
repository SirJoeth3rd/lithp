#include "lithp.hpp"
#include "tokenizer.hpp"

using std::string;

SymbolLookup::SymbolLookup() {
  push_namespace();
}

SymbolLookup::~SymbolLookup() {
  table.clear();
}

lval_sptr SymbolLookup::operator[](const char* name) {
  for (auto sym_tab = table.rbegin(); sym_tab != table.rend(); sym_tab++) {
      if (sym_tab->count(name)) {
	return (*sym_tab)[name];
      }
    }
    return (*table.end())[name];
}

void SymbolLookup::insert(const char* name, lval_sptr item) {
  table.back()[name] = item;
}
  
int SymbolLookup::count(const char* name) {
  int count = 0;
  for (auto sym_tab = table.rbegin(); sym_tab != table.rend(); sym_tab++) {
    if (sym_tab->count(name)) {
      count++;
    }
  }
  return count;
}

void SymbolLookup::push_namespace() {
  std::map<const char*, lval_sptr, StrCompare> new_map;
  table.push_back(new_map);
}

void SymbolLookup::pop_namespace() {
  table.pop_back();
}

/*
Engine::Engine() {}

Engine::~Engine() {
  bool dealloc_success = exprs.empty();
  if (dealloc_success) {
    std::cout << "deallocation success of engine" << std::endl;
  } else {
    std::cout << "deallocation failure of engine" << std::endl;
  }
}
  
void Engine::parse_push(const string& expr){
  std::vector<Token> tokens = tokenize(expr);
  lval_sptr root = parse_tokens(tokens);
  exprs.push(root);
}

lval_sptr Engine::pop() {
  lval_sptr top = exprs.top();
  exprs.pop();
  return top;
}

void Engine::eval_top() {
  //pop the top level lval and eval, push back the new lval
  lval_sptr top = exprs.top();
  exprs.pop();
  eval(top);
  exprs.push(top);
}

void Engine::eval(lval_sptr expr_head) {
  //Note we probaly need to pass a double pointer here
  bool found;
  lval_sptr result;
  while (expr_head) {
    switch (expr_head->type) {
    case Int:
      break;
    case Float:
      break;
    case String:
      break;
    case List:
      break;
    case Nil:
      break;
    case Symbol:
      //Here we do symbol lookup
      found = symbol_table.count(expr_head->data.Symbol);
      if  (found) {
	set_lval(expr_head,fetch_symbol(expr_head->data.Symbol));
      }
      break;
      //NOTE: CAN JUST CONTINUE IF SYMBOL LOOKUP PRODUCED FUNCTION LVAL
      //LIKE SO
      //set_lval(expr_head, .....)
      //if expr_head.type == lambda {continue}
      //Then it will evaluate the symbol as well
    case Lambda:
      //Here we do function lookup
      if (is_macro(expr_head->data.Symbol)) {
	lval_sptr result = call_func(
				 expr_head->branch,
				 expr_head->data.Symbol
				 );
	set_lval(expr_head, result);
	eval(result);
      } else {
	eval(expr_head->branch);
	lval_sptr result = call_func(
				 expr_head->branch,
				 expr_head->data.Symbol
				 );
	set_lval(expr_head, result);
      }
      break;
    }
    expr_head = expr_head->next;
  }
}

void Engine::set_lval(lval_sptr x, lval_sptr y) {
  if (!y || !x) {
    return;
  }
  y->next = y->next ? y->next : x->next;
  y->prev = y->prev ? y->prev : x->prev;
  *x = *y;
}

lval_sptr Engine::call_func(lval_sptr input, const char* name) {
  //TODO, check for built in functions here
  if (!is_func(name)) {
    //must be a symbol //TODO: this needs to be in a while loop
    lval_sptr lookup = fetch_symbol(name);
    name = lookup->data.Symbol;
  }
  return function_table[name](input, this);
}

lval_sptr Engine::fetch_symbol(const char* name) {
  return symbol_table[name];
}

void Engine::set_symbol(lval_sptr value, const char* name) {
    symbol_table.insert(name, value);
  }

bool Engine::is_func(const char* name) {
    return function_.count(name);
  }

bool Engine::is_macro(const char* name) {
  return macro_table.count(name);
}

bool Engine::is_symbol(const char* name) {
    return symbol_table.count(name);
}
*/

//custom function declarations
/*
lval_sptr plus(lval_sptr head, Engine* engine) {
  int total = 0;
  while (head) {
    if (head->type == Int) {
      total += head->data.Int;
    } else {
      //do the error thing over here
    }
    head = head->next;
  }
  
  lval_sptr ret_val = lval(nullptr, nullptr, nullptr, Int, (ldata)total);
  return ret_val;
};

lval_sptr mul(const lval_sptr head, Engine* engine) {
  int total = 1;
  while (head) {
    if (head->type == Int) {
      total *= head->data.Int;
    } else {
      //error here
    }
    head = head->next;
  }
  lval_sptr ret_val = new lval{nullptr, nullptr, nullptr, total, Int};
  return ret_val;
}

lval_sptr set(const lval_sptr head,Engine* engine) {
  //we expect a symbol name first and a lval_sptr
  if (head->type != Symbol){
    //wud?
  }

  const char* name = head->data.Symbol;
  engine->set_symbol(name, head->next);
  
  return nullptr;
}
*/

int main() {
  string test = "h(f(g,88,[1,2,3]),[gg(hello, world)])";

  std::vector<Token> tokens = tokenize(test);

  print_tokens(tokens);

  std::cout << "==========PARSER TESTS==========" << '\n';
  lval_sptr root = parse_tokens(tokens);
  std::cout << "==========PRINT_AST==========\n";
  print_ast(root);

  /*
  std::cout << "==========EVUALATOR TEST========" << '\n';
  Engine engine;
  engine.subscribe_func(plus, "plus");
  engine.subscribe_func(mul, "mul");
  engine.subscribe_func(set, "set");

  engine.parse_push("set(x,2)");
  engine.eval_top();
  engine.parse_push("plus(x,2)");
  engine.eval_top();
  lval_sptr top = engine.pop();
  std::cout << top->data.Int << std::endl;
  */
}
