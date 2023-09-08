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
    while (expr_head) {
      switch (expr_head->type) {
      case Int:
	break;
      case Float:
	break;
      case String:
	break;
      case Symbol:
	break;
      case Func:
	eval(expr_head->branch);
	//TODO use call func
	// expr_head = symbol_table[expr_head->data.Symbol](expr_head->branch);
	*(expr_head) = *call_func(
				  expr_head->data.Symbol,
				  expr_head->branch
				  );
	break;
      }
      expr_head = expr_head->next;
    }
  }

  void subscribe_func(std::function<lval*(lval*)> func, const char* name) {
    symbol_table[name] = func;
  }

  lval* call_func(const char* name, lval* input) {
    //TODO, check for built in functions here
    return symbol_table[name](input);
  }
  
private:
  std::stack<lval*> exprs;
  std::map<
    const char*,
    std::function<lval*(lval*)>,
    StrCompare> symbol_table;
};


lval* plus(const lval* head) {
  int total = 0;
  while (head) {
    if (head->type == Int) {
      total += head->data.Int;
    } else {
      //do the error thing over here
    }
    std::cout << "TOTAL = " << total << '\n';
    head = head->next;
  }
  lval* ret_val = new lval{nullptr, nullptr, nullptr, total, Int};
  return ret_val;
};

lval* mul(const lval* head) {
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

int main() {
  string test = "plus(mul(2,2),9)";

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
  const char* p1 = "plus";
  const char* p2 = "plus";
  engine.subscribe_func(plus, p1);
  engine.subscribe_func(mul, "mul");

  
  engine.parse_push(test);
  engine.eval_top();
  lval* n = plus(root->branch);
  lval* top = engine.pop();
  std::cout << top->data.Int << '\n';
  
  // lval* out = engine.call_func(p2, root->branch);
  // std::cout << out->data.Int << std::endl;
}
