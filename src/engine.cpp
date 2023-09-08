#include "parser.hpp"
#include "tokenizer.hpp"

using std::string;

struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

class Engine {
public:
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
    bool symbol_exists;
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
	symbol_exists = symbol_table.count(expr_head->data.Symbol);
	if  (symbol_exists) {
	  set_lval(expr_head,fetch_symbol(expr_head->data.Symbol));
	}
	break;
      case Func:
	//Here we do function llokup
	eval(expr_head->branch);
	lval* result = call_func(
				  expr_head->data.Symbol,
				  expr_head->branch
				  );
	if (!result) {
	  break;
	}
	
	if (result->next == nullptr) {
	  result->next = expr_head->next;
	}

	set_lval(expr_head, result);
	break;
      }
      expr_head = expr_head->next;
    }
  }

  void set_lval(lval* x, lval* y) {
    y->next = y->next ? y->next : x->next;
    y->prev = y->prev ? y->prev : x->prev;
    *x = *y;
  }

  void subscribe_func(
		      std::function<lval*(lval*, Engine* engine)> func,
		      const char* name
		      )
  {
    function_table[name] = func;
  }

  lval* call_func(const char* name, lval* input) {
    //TODO, check for built in functions here
    return function_table[name](input, this);
  }

  lval* fetch_symbol(const char* name) {
    return symbol_table[name];
  }

  void set_symbol(const char* name, lval* value) {
    symbol_table[name] = value;
  }
  
private:
  std::stack<lval*> exprs;
  std::map<
    const char*,
    std::function<lval*(lval*, Engine* engine)>,
    StrCompare> function_table;
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

  engine->set_symbol(head->data.Symbol, head->next);
  return nullptr;
}

int main() {
  string test = "set(x,2)";

  std::vector<Token> tokens = tokenize(test);
  for (Token t : tokens) {
    if (t.val) {
      std::cout << t.val <<  ':' << t.Type << '\n';
    } else {
      switch (t.Type) {
      case Token::LBrack:
	std::cout << '(' << '\n';
	break;
      case Token::RBrack:
	std::cout << ')' << '\n';
	break;
      case Token::Comma:
	std::cout << ',' << '\n';
	break;
      }
    }
  }

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
  engine.parse_push("plus(x,3)");
  engine.eval_top();
  lval* top = engine.pop();
  std::cout << top->data.Int << std::endl;
}
