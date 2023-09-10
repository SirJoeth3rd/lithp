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
    function_table[name] = func;
  }

  void subscribe_macro(lithp_func func, const char* name) {
    function_table[name] = func;
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
    symbol_table[name] = value;
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
  std::map<
    const char*,
    lithp_func,
    StrCompare> function_table;
  std::map<
    const char*,
    lithp_func,
    StrCompare> macro_table;
  std::map<
    const char*,
    lval*,
    StrCompare> symbol_table;
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

void recurse_replace(lval* symbol, lval* head) {

}

lval* function(lval* head, Engine* engine) {
  //first lval is a list
  //from there we have the body
  lval* args = head; //this must be a list of symbols
  lval* body = head->next; //this is the body

  string anon_name = "lambda_";

  //we need to store the body and return a lval(lval*, Engine*) function
  engine->subscribe_func([](lval* head, Engine* engine) -> lval* {
    //in here head is just the args so we replace them
    while(head) {
      
      
      head = head->next;
    }
  }, anon_name.c_str());
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

  
  engine.parse_push("plus(2,mul(3,3))");
  engine.eval_top();
  lval* top = engine.pop();
  std::cout << top->data.Int << std::endl;
}
