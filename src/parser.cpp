#include "lithp.hpp"
#include "tokenizer.hpp"

lval_sptr root;
lval_sptr prev;
lval_sptr head;
std::stack<lval_sptr> saves;
std::vector<Token>::iterator tok;
std::vector<Token>::iterator tok_end;

bool DO_LOGGING = false;

void log(const string& str) {
  if (DO_LOGGING) {
    std::cout << str;
  }
}
void inc_head(){
  log("inc head\n");
  prev = head;
  head->insert_next(std::make_shared<lval>(lval()));
  head = head->get_next();
};

string get_tok_type() {
  return tok_type_to_string(tok->Type);
};

void expect(Token::tok_type type) {
  log("expect = " + tok_type_to_string(type) + '\n');
  if (tok->Type != type) {
    std::string err = "\nInvalid token type\n  Expected ";
    err += tok_type_to_string(type);
    err += "\n  But got ";
    err += tok_type_to_string(tok->Type);
    err += '\n';
    throw std::invalid_argument(err);
  }
  tok++;
}

void parse_int() {
  log("parse_int\n");
  log("  value = " + (string) tok->val +'\n');
  lval_data data = std::atoi(tok->val);
  *head = lval(Int, data);
  tok++;
};

void parse_float() {
  log("parse_float\n");
  log("  value = " + (string) tok->val +'\n');
  lval_data data = std::atof(tok->val);
  *head = lval(Float, data);
  tok++;
};

void parse_string() {
  log("parse_string\n");
  log("  value = " + (string) tok->val +'\n');
  lval_data data = string(tok->val);
  *head = lval(String, data);
  tok++;
};

void parse_symbol() {
  log("parse_symbol\n");
  log("  value = " + (string) tok->val +'\n');
  lval_data data = (string) tok->val;
  *head = lval(Symbol,data);
  tok++;
};

void parse_atom() {
  log("parse_atom\n");
  switch (tok->Type) {
  case Token::Symbol:
    parse_symbol();
    inc_head();
    break;
  case Token::Int:
    parse_int();
    inc_head();
    break;
  case Token::String:
    parse_string();
    inc_head();
    break;
  case Token::Float:
    parse_float();
    inc_head();
    break;
  default:
    throw std::invalid_argument("Expected an atom");
    break;
  }
};

bool is_func_call() {
  log("is_func_call = ");
  if (tok->Type != Token::Symbol) {
    log("false\n");
    return false;
  }
  tok++;
  if (tok->Type != Token::LBrack) {
    log("false\n");
    tok--;
    return false;
  }
  tok--;
  log("true\n");
  return true;
};

bool is_atom() {
  log("is_atom\n");
  log("  tok_type in is_atom " + get_tok_type() + "\n");
  auto tok_type = tok->Type;
  switch (tok_type) {
  case Token::Symbol:
  case Token::String:
  case Token::Float:
  case Token::Int:
    return true;
  default:
    return false;
  };
};

bool is_list() {
  log("is list\n");
  auto tok_type = tok->Type;
  if (tok_type == Token::LBBrack) {
    return true;
  }
  return false;
};

void parse_func();
void parse_list();

void parse_expr() {
  //note do not handle tok in here
  log("parse_expr\n");
  if (is_func_call()) {
    parse_func();
  } else if (is_atom()){
    parse_atom();
  } else if (is_list()) {
    parse_list();
  } else {
    throw std::logic_error("Syntax error");
  }
};

void push_saves() {
  log("push save\n");
  saves.push(head);
  prev = head;
  head->insert_branch(std::make_shared<lval>(lval()));
  head = head->get_branch();
};

void pop_saves() {
  log("pop saves\n");
  prev->remove_next();
  head = saves.top();
  saves.pop();
  if (head->get_prev()) {
    prev = head->get_prev();
  } else {
    prev = nullptr;
  }
};

void parse_list() {
  log("parse list\n");
  expect(Token::LBBrack);
  
  *head = lval(List);
  push_saves();
  
  while (tok->Type != Token::RBBrack) {
    parse_expr();
    if (tok->Type == Token::RBBrack) {
      break;
    } else {
      expect(Token::Comma);
    }
  }

  pop_saves();
    
  inc_head();

  tok++;
};

void parse_func() {
  log("parse func\n");
  parse_symbol();
  push_saves();
    
  expect(Token::LBrack);
  while (tok->Type != Token::RBrack) {
    parse_expr();
    if (tok->Type == Token::RBrack) {
      break;
    } else {
      expect(Token::Comma);
    }
  }
    
  pop_saves();
  log("post pop\n");
  inc_head();
  log("pre tok++\n");
  tok++;
  log("post tok++\n");
};

lval_sptr parse_tokens(std::vector<Token>& tokens) {
  root = std::make_shared<lval>(lval());
  prev = nullptr;
  head = root;
  tok = tokens.begin();
  tok_end = tokens.end();

  parse_expr();

  //head always initialized to something
  head->get_prev()->remove_next();

  return root;
}

lval_sptr parse_string(const string& str) {
  std::vector<Token> tokens = tokenize(str);
  return parse_tokens(tokens);
}
