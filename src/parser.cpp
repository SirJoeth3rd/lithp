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

lval::~lval() {
  //TODO this needs to delete the memory of string or symbol
}

string ltype_to_string(ltype type) {
  switch (type) {
  case Int:
    return "Int";
  case Float:
    return "Float";
  case String:
    return "String";
  case Func:
    return "Func";
  case Symbol:
    return "Symbol";
  case List:
    return "List";
  case Nil:
    return "Nil";
  case Lambda:
    return "Nil";
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
    prev = head;
    head = std::make_shared<lval>(lval());
    prev->next = head;
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

  parse_list = [&]() mutable {
    expect(Token::LBBrack);

    // *head = lval(nullptr,prev,nullptr,List,0);
    ldata data;
    data.Int = 0;
    *head = lval(nullptr, prev, nullptr, List, data);

    prev = head;
    head = std::make_shared<lval>(lval());
    prev->branch = head;
    lists.push(prev);
    
    while (tok->Type != Token::RBBrack) {
      parse_expr();
      if (tok->Type == Token::RBBrack) {
	break;
      } else {
	expect(Token::Comma);
      }
    }
    prev->next = nullptr;
    head = lists.top();
    lists.pop();
    prev = head->prev;
    inc_head();

    tok++;
  };

  parse_func = [&]() mutable {
    // std::cout << "parse_func\n";
    parse_symbol(true);
    expect(Token::LBrack);
    while (tok->Type != Token::RBrack) {
      parse_expr();
      if (tok->Type == Token::RBrack) {
	break;
      } else {
	expect(Token::Comma);
      }
    }

    //setting head back to top
    prev->next = nullptr;
    head = funcs.top();
    funcs.pop();
    prev = head->prev;
    inc_head();

    tok++;
  };

  parse_expr();
  prev->next = nullptr;
  return root;
}

void print_ast(lval_sptr head, int indent) {
  while (head) {
    switch (head->type) {
    case (Func):
      std::cout << string(indent,' ')
		<< "Func:"
		<< head->data.Symbol
		<< "\n";
      print_ast(head->branch,indent + 2);
      break;
    case (List):
      std::cout << string(indent,' ')
		<< "List:"
		<< "\n";
      print_ast(head->branch,indent + 2);
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
      std::cout << string(indent,' ') << "Float:" << head->data.Float << "\n";
      break;
    }
    head = head->next;
  }
}


