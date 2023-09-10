#include "tokenizer.hpp"

using std::string;

std::string tok_type_to_string(Token::tok_type type) {
  std::vector<std::string> names = {
    "Int",
    "Float",
    "String",
    "Symbol",
    "LBrack",
    "RBrack",
    "LBBrack",
    "LBBrack",
    "Comma",
  };
  return names[type];
}

string::const_iterator END;

Token handle_string(string::const_iterator& c) {
  string* value = new string {""};
  ++c;

 escapedQuotationMarks:
  while (c != END && *c != '"') {
    value->push_back(*c);
    ++c;
  }

  if (c != END && *(--c) == '\\') {
    ++c;
    value->push_back(*c);
    ++c;
    goto escapedQuotationMarks;
  }

  ++c;

  return {Token::String, value->c_str()};
}

Token handle_number(string::const_iterator& c) {
  string* value = new string{""};
  bool dot_found = false;

 isFloat:
  while (c != END && std::isdigit(*c)) {
    value->push_back(*c);
    c++;
  }

  if (*c == '.') {
    if (dot_found) {
      //TODO:some error stuff here
    }
    dot_found = true;
    value->push_back(*c);
    c++;
    goto isFloat;
  }

  c--;
  if (dot_found) {
    return {Token::Float, value->c_str()};
  } else {
    return {Token::Int, value->c_str()};
  }
}

Token handle_symbol(string::const_iterator& c) {
  string* value = new string{""};

  while (c != END && std::isalnum(*c)) {
    value->push_back(*c);
    c++;
  }

  c--;
  return {Token::Symbol, value->c_str()};
}

std::vector<Token> tokenize(const string& prog) {
  END = prog.end();

  std::vector<Token> tokens;

  Token curr_tok;
  
  for (auto c = prog.begin(); c != END; ++c) {
    if (std::isdigit(*c)) {
      curr_tok = handle_number(c);
    } else if (std::isalpha(*c)) {
      curr_tok = handle_symbol(c);
    } else {
      switch (*c) {
      case '"':
	curr_tok = handle_string(c);
	break;

      case '(':
	curr_tok = {Token::LBrack,nullptr};
	break;

      case ')':
	curr_tok = {Token::RBrack,nullptr};
	break;

      case '[':
	curr_tok = {Token::LBBrack,nullptr};
	break;

      case ']':
	curr_tok = {Token::RBBrack,nullptr};
	break;

      case ',':
	curr_tok = {Token::Comma, nullptr};
	break;

      case ' ':
      case '\t':
      case '\n':
	continue;

      default:
	continue;
      }
    }
    tokens.push_back(curr_tok);
  }
  return tokens;
}

void print_tokens(const std::vector<Token>& tokens) {
  for (Token t : tokens) {
    if (t.val) {
      std::cout << t.val <<  ':' << tok_type_to_string(t.Type) << '\n';
    } else {
      switch (t.Type) {
      case Token::LBrack:
	std::cout << '(' << '\n';
	break;
      case Token::RBrack:
	std::cout << ')' << '\n';
	break;
      case Token::LBBrack:
	std::cout << '[' << '\n';
	break;
      case Token::RBBrack:
	std::cout << ']' << '\n';
	break;
      case Token::Comma:
	std::cout << ',' << '\n';
	break;
      }
    }
  }
}
