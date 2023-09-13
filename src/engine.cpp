#include "parser.hpp"
#include "tokenizer.hpp"

using std::string;

//TODO, delete later when you use engine header file
class Engine;
typedef std::function<lval*(lval*,Engine*)> lithp_func;

struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

template <class T>
class SymbolLookup {
public:
  SymbolLookup() {
    //initialise with first empty map
    push_namespace();
  }

  ~SymbolLookup() {
    table.clear();
  }
  
  T& operator[](const char* name) {
    //search from bottom to top
    for (auto sym_tab = table.rbegin(); sym_tab != table.rend(); sym_tab++) {
      if (sym_tab->count(name)) {
	return (*sym_tab)[name];
      }
    }
    return (*table.end())[name];
  }

  void insert(const char* name, T item) {
    table.back()[name] = item;
  }

  int count(const char* name) {
    int count = 0;
    for (auto sym_tab = table.rbegin(); sym_tab != table.rend(); sym_tab++) {
      if (sym_tab->count(name)) {
	count++;
      }
    }
    return count;
  }

  void push_namespace() {
    std::map<const char*, T, StrCompare> new_map;
    table.push_back(new_map);
  }

  void pop_namespace() {
    table.pop_back();
  }
  
private:
  std::vector<std::map<const char*, T, StrCompare>> table;
};

class Engine {
public:
  int lambda_count;

  Engine() {
    lambda_count = 0;
  }
  
  void parse_push(const string& expr){
    std::vector<Token> tokens = tokenize(expr);
    lval* root = parse_tokens(tokens);
    exprs.push(root);
  }

  lval* pop() {
    lval* top = exprs.top();
    exprs.pop();
    return top;
  }

  void eval_top() {
    //pop the top level lval and eval, push back the new lval
    lval* top = exprs.top();
    exprs.pop();
    eval(top);
    exprs.push(top);
  }

  void eval(lval* expr_head) {
    //Note we probaly need to pass a double pointer here
    bool found;
    lval* result;
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
      case Symbol:
	//Here we do symbol lookup
	found = symbol_table.count(expr_head->data.Symbol);
	if  (found) {
	  set_lval(expr_head,fetch_symbol(expr_head->data.Symbol));
	}
	break;
      case Func:
	//Here we do function lookup
	if (is_macro(expr_head->data.Symbol)) {
	  lval* result = call_func(
				   expr_head->data.Symbol,
				   expr_head->branch
				   );
	  set_lval(expr_head, result);
	  eval(result);
	} else {
	  eval(expr_head->branch);
	  lval* result = call_func(
				   expr_head->data.Symbol,
				   expr_head->branch
				   );
	  set_lval(expr_head, result);
	}
	break;
      }
      expr_head = expr_head->next;
    }
  }

  void set_lval(lval* x, lval* y) {
    if (!y || !x) {
      return;
    }
    y->next = y->next ? y->next : x->next;
    y->prev = y->prev ? y->prev : x->prev;
    *x = *y;
  }

  void subscribe_func(lithp_func func,const char* name) {
    function_table.insert(name, func);
  }

  void subscribe_macro(lithp_func func, const char* name) {
    function_table.insert(name, func);
  }

  lval* call_func(const char* name, lval* input) {
    //TODO, check for built in functions here
    if (!is_func(name)) {
      //must be a symbol //TODO: this needs to be in a while loop
      lval* lookup = fetch_symbol(name);
      name = lookup->data.Symbol;
    }
    return function_table[name](input, this);
  }

  lval* fetch_symbol(const char* name) {
    return symbol_table[name];
  }

  void set_symbol(const char* name, lval* value) {
    symbol_table.insert(name, value);
  }

  bool is_func(const char* name) {
    return function_table.count(name);
  }

  bool is_macro(const char* name) {
    return macro_table.count(name);
  }

  bool is_symbol(const char* name) {
    return symbol_table.count(name);
  }
  
private:
  std::stack<lval*> exprs;
  SymbolLookup<lithp_func> function_table;
  SymbolLookup<lithp_func> macro_table;
  SymbolLookup<lval*> symbol_table;
};

lval* plus(const lval* head, Engine* engine) {
  int total = 0;
  while (head) {
    if (head->type == Int) {
      total += head->data.Int;
    } else {
      //do the error thing over here
    }
    head = head->next;
  }
  lval* ret_val = new lval{nullptr, nullptr, nullptr, total, Int};
  return ret_val;
};

lval* mul(const lval* head, Engine* engine) {
  int total = 1;
  while (head) {
    if (head->type == Int) {
      total *= head->data.Int;
    } else {
      //error here
    }
    head = head->next;
  }
  lval* ret_val = new lval{nullptr, nullptr, nullptr, total, Int};
  return ret_val;
}

lval* set(const lval* head,Engine* engine) {
  //we expect a symbol name first and a lval*
  if (head->type != Symbol){
    //wud?
  }

  const char* name = head->data.Symbol;
  engine->set_symbol(name, head->next);
  
  return nullptr;
}

int main() {
  string test = "h([1,2,[4,f(x,y)],[z],99])";

  std::vector<Token> tokens = tokenize(test);

  print_tokens(tokens);


  std::cout << "==========PARSER TESTS==========" << '\n';
  lval* root = parse_tokens(tokens);
  std::cout << "==========PRINT_AST==========\n";
  print_ast(root);


  std::cout << "==========EVUALATOR TEST========" << '\n';
  Engine engine;
  engine.subscribe_func(plus, "plus");
  engine.subscribe_func(mul, "mul");
  engine.subscribe_func(set, "set");

  engine.parse_push("set(x,2)");
  engine.eval_top();
  engine.parse_push("plus(x,2)");
  engine.eval_top();
  lval* top = engine.pop();
  std::cout << top->data.Int << std::endl;
}
