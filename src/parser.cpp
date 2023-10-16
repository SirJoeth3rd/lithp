// #include "parser.hpp"
#include "tokenizer.hpp"
#include "lithp.hpp"

using std::string;

lval::lval() {
  type = Nil;
}

lval::lval(lval_sptr n, lval_sptr p, lval_sptr b, ltype t, ldata d) {
  next = n;
  prev = p;
  branch = b;
  type = t;
  data = d;
}

lval::lval(ltype t) {
  type = t;
}

lval::lval(ltype t, ldata v) {
  type = t;
  data = v;
}

lval_sptr lval::operator&() {
  return std::make_unique<lval>(*this);
}

void lval::operator[](lval_sptr br) {
  this->branch = br;
}

void lval::operator[](lval& l) {
  this->branch = &l;
}

lval lval::operator++() {
  return *(this->next);
}

//this is a very expensive operation
//but thats what you get for working with linked-lists
lval lval::operator>>(lval& l) {
  lval head = *this;
  while (head.next) {
    ++head;
  }
  head.next = &l;
  return *this;
}

lval::~lval() {
  //TODO this needs to delete the memory of string or symbol
};

string ltype_to_string(ltype type) {
  switch (type) {
  case Int:
    return "Int";
  case Float:
    return "Float";
  case String:
    return "String";
  case Symbol:
    return "Symbol";
  case List:
    return "List";
  case Nil:
    return "Nil";
  case Lambda:
    return "Lambda";
  }
  return "";
}

lval_sptr parse_tokens(std::vector<Token>& tokens) {
  //TODO: BIGGIE We need better error handling here
  lval_sptr root = std::make_shared<lval>(lval());
  lval_sptr prev = nullptr;
  lval_sptr head = root;
  
  std::stack<lval_sptr> funcs;
  std::stack<lval_sptr> lists;

  std::stack<lval_sptr> saves;

  auto END = tokens.end();
  auto tok = tokens.begin();

  auto inc_head = [&] () mutable {
    std::cout << "inc head\n";
    prev = head;
    head = std::make_shared<lval>(lval());
    prev->next = head;
    std::cout << "post inc head\n";
  };

  auto get_tok_type = [&] () -> std::string {
    return tok_type_to_string(tok->Type);
  };

  auto expect = [&] (Token::tok_type type) mutable {
    std::cout << "expect = " << tok_type_to_string(tok->Type) << '\n';
    if (tok->Type != type) {
      std::string err = "\nInvalid token type\n  Expected ";
      err += tok_type_to_string(type);
      err += "\n  But got ";
      err += tok_type_to_string(tok->Type);
      err += '\n';
      throw std::invalid_argument(err);
    }
    tok++;
  };
    
  auto parse_int = [&]() mutable {
    std::cout << "parse_int\n";
    std::cout << "  value = " << tok->val << '\n';
    ldata data;
    data.Int = std::atoi(tok->val);
    *head = lval(nullptr, prev, nullptr, Int, data);
    inc_head();
    tok++;
  };

  auto parse_float = [&]() mutable {
    std::cout << "parse_float\n";
    std::cout << "  value = " << tok->val << '\n';
    ldata data;
    data.Float = std::atof(tok->val);
    *head = lval(nullptr, prev, nullptr, Float, data);
    inc_head();
    tok++;
  };

  auto parse_string = [&]() mutable {
    std::cout << "parse_string\n";
    std::cout << "  value = " << tok->val << '\n';
    ldata data;
    data.String = (char*)tok->val;
    *head = lval(nullptr,prev,nullptr,String,data);
    inc_head();
    tok++;
  };

  auto parse_symbol = [&](bool is_func) mutable {
    std::cout << "parse_symbol\n";
    std::cout << "  tok_type " << get_tok_type() << '\n';
    std::cout << "  val " << tok->val << '\n';
    ldata data;
    data.Symbol = tok->val;
    *head = lval(nullptr,prev,nullptr,Symbol,data);
    if (is_func) {
      prev = head;
      head = std::make_shared<lval>(lval());
      prev->branch = head;
      funcs.push(prev);
    } else {
      inc_head();
    }
    tok++;
  };

  auto parse_atom = [&]() mutable {
    std::cout << "parse_atom\n";
    switch (tok->Type) {
    case Token::Symbol:
      parse_symbol(false);
      break;
    case Token::Int:
      parse_int();
      break;
    case Token::String:
      parse_string();
      break;
    case Token::Float:
      parse_float();
      break;
    default:
      throw std::invalid_argument("Expected an atom");
      break;
    }
  };

  auto is_func_call = [&]() mutable {
    std::cout << "is_func_call = ";
    if (tok->Type != Token::Symbol) {
      std::cout << "false\n";
      return false;
    }
    tok++;
    if (tok->Type != Token::LBrack) {
      std::cout << "false\n";
      tok--;
      return false;
    }
    tok--;
    std::cout << "true\n";
    return true;
  };

  auto is_atom = [&]() mutable {
    std::cout << "is_atom\n";
    std::cout << "  tok_type in is_atom " << get_tok_type() << "\n";
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

  auto is_list = [&]() mutable {
    std::cout << "is list\n";
    auto tok_type = tok->Type;
    if (tok_type == Token::LBBrack) {
      return true;
    }
    return false;
  };

  std::function<void()> parse_func;
  std::function<void()> parse_list;

  auto parse_expr = [&]() mutable {
    //note do not handle tok in here
    std::cout << "parse_expr\n";
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

  auto push_saves = [&]() mutable {
    std::cout << "push save\n";
    prev = head;
    head = std::make_shared<lval>(lval());
    prev->branch = head;
    saves.push(prev);    
  };

  auto pop_saves = [&]() mutable {
    std::cout << "pop saves\n";
    prev->next = nullptr;
    head = saves.top();
    saves.pop();
    prev = head->prev;
    std::cout << "post pop saves\n";
  };

  parse_list = [&]() mutable {
    expect(Token::LBBrack);

    // *head = lval(nullptr,prev,nullptr,List,0);
    ldata data;
    data.Int = 0;
    *head = lval(nullptr, prev, nullptr, List, data);

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

  parse_func = [&]() mutable {
    ldata data;
    data.Symbol = tok->val;
    *head = lval(nullptr, prev, nullptr, Symbol, data);

    push_saves();

    tok++;
    
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
    
    inc_head();

    std::cout << "pre tok++\n";
    tok++;
    std::cout << "post tok++\n";
  };

  parse_expr();
  std::cout << "DONE PARSING\n"; 
  prev->next = nullptr;
  return root;
}

void print_ast(lval_sptr head, int indent) {
  while (head) {
    switch (head->type) {
    case (Nil):
      std::cout << string(indent,' ')
		<< "Nil"
		<< "\n";
      break;
    case (Lambda):
      std::cout << string(indent,' ')
		<< "Lambda"
		<< "\n";
      break;
    case (List):
      std::cout << string(indent,' ')
		<< "List"
		<< "\n";

      break;
    case (Symbol):
      std::cout << string(indent,' ')
		<< "Symbol:"
		<< head->data.Symbol
		<< "\n";
      break;
    case (String):
      std::cout << string(indent,' ')
		<< "String:"
		<< head->data.String
		<< "\n";
      break;
    case (Int):
      std::cout << string(indent,' ')
		<< "Int:"
		<< head->data.Int
		<< "\n";
      break;
    case (Float):
      std::cout << string(indent,' ')
		<< "Float:"
		<< head->data.Float
		<< "\n";
      break;
    }
    
    if (head->branch) {
      print_ast(head->branch,indent + 2);
    }
    
    head = head->next;
  }
}


