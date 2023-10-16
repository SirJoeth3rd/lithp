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

void SymbolLookup::insert_global(const char* name, lval_sptr item) {
  table.front()[name] = item;
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


Engine::Engine() {
  //alloc
}

Engine::~Engine() {
  //dealloc
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

lval_sptr Engine::call_func(lval_sptr lambda_lval) {
  lval_sptr result = lambda_lval->lambda(lambda_lval->branch, this);
  return result;
}

void Engine::eval(lval_sptr head) {
  while (head) {
    switch (head->type) {
    case Symbol:
      //do symbol lookup
      if (symbol_table.count(head->data.Symbol)) {
	set_lval(head, fetch_symbol(head->data.Symbol));
	//re-eval head
	continue;
      }
      break;
    case Lambda:
      if (head->is_macro) {
	*head = *call_func(head);
	eval(head);
      } else {
	eval(head->branch);
	*head = *call_func(head);
      }
    default:
      //just leave values alone like Nil and Int
      break;
    }
    head = head->next;
  }
}

void Engine::set_lval(lval_sptr x, lval_sptr y) {
  if (!y || !x) {
    return;
  }
  auto next = y->next ? y->next : x->next;
  auto prev = y->prev ? y->prev : x->prev;
  auto branch = y->branch ? y->branch : x->branch;
  *x = *y;
  x->next = next;
  x->prev = prev;
  x->branch = branch;
}

lval_sptr Engine::fetch_symbol(const char* name) {
  return symbol_table[name];
}

void Engine::set_symbol(lval_sptr value, const char* name) {
  symbol_table.insert(name, value);
}

void Engine::set_symbol_global(lval_sptr value, const char* name) {
  symbol_table.insert_global(name, value);
}

void Engine::push_namespace() {
  symbol_table.push_namespace();
}

void Engine::pop_namespace() {
  symbol_table.pop_namespace();
}

void Engine::subscribe_func(lithp_func func, const char *name) {
  lval lambda = lval();
  lambda.is_macro = false;
  lambda.lambda = func;
  lambda.type = Lambda;
  symbol_table.insert_global(name, std::make_unique<lval>(lambda));
}

void Engine::subscribe_macro(lithp_func func, const char *name) {
  lval lambda = lval();
  lambda.is_macro = true;
  lambda.lambda = func;
  lambda.type = Lambda;
  symbol_table.insert_global(name, std::make_unique<lval>(lambda));
}

//custom function declarations here
lval_sptr plus(lval_sptr head, Engine* engine) {
  int total = 0;
  while (head) {
    if (head->type == Int) {
      total += head->data.Int;
    } else {
      //wud
    }
    head = head->next;
  }
  auto result = lval(nullptr, nullptr, nullptr, Int, total);
  return std::make_unique<lval>(result);
}

lval_sptr with(lval_sptr head, Engine* engine) {
  //first arg has the shape [[symbol expr]]
  //second arg is the body
  //I think it's a good time to start building the ast manipulation library
  auto args = head->branch;
  auto body = head->next;
  engine->push_namespace();
  while (args) {
    engine->set_symbol(args->branch->next,args->branch->data.Symbol);
    args = args->next;
  }
  engine->eval(body);
  engine->pop_namespace();
  return body;
}

/*
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
*/

lval_sptr set(lval_sptr head,Engine* engine) {
  //we expect a symbol name first and a lval_sptr
  if (head->type != Symbol){
    //wud?
  }

  const char* name = head->data.Symbol;
  engine->set_symbol(head->next, name);
  
  return std::make_unique<lval>(lval());
}


int main() {
  /*
  string test = "h(f(g,88,[1,2,3]),[9])(hello, world)])";

  std::vector<Token> tokens = tokenize(test);

  print_tokens(tokens);

  std::cout << "==========PARSER TESTS==========" << '\n';
  lval_sptr root = parse_tokens(tokens);
  std::cout << "==========PRINT_AST==========\n";
  print_ast(root);

  */
  std::cout << "==========EVUALATOR TEST========" << '\n';
  Engine engine;
  engine.subscribe_func(plus, "plus");
  // engine.subscribe_func(mul, "mul");
  engine.subscribe_func(set, "set");
  engine.subscribe_macro(with, "with");

  engine.parse_push("with([[x,2],[y,3]],plus(x,y))");
  engine.eval_top();
  lval_sptr top = engine.pop();
  std::cout << top->data.Int << std::endl;
  
}
