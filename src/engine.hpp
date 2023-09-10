#include "parser.hpp"
#include "tokenizer.hpp"

using std::string;

class Engine;
typedef std::function<lval*(lval*,Engine*)> lithp_func;

struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

class Engine {
public:
  void parse_push(const string& expr);
  lval* pop();
  void eval_top();
  void eval(lval* expr_head);
  void set_lval(lval* x, lval* y);
  void subscribe_func(lithp_func,const char*);
  lval* call_func(const char*, lval*);
  lval* fetch_symbol(const char*);
  void set_symbol(const char*, lval*);
  bool is_func(const char*);
  bool is_symbol(const char*);
  
private:
  std::stack<lval*> exprs;
  std::map<const char*, lithp_func, StrCompare> function_table;
  std::map<const char*, lval*, StrCompare> symbol_table;
};
