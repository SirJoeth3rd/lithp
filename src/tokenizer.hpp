#pragma once

#include <cctype>
#include <string>
#include <vector>
#include <typeinfo>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <stack>
#include <map>

struct Token {
  enum tok_type {
    Int,
    Float,
    String,
    Symbol,
    LBrack,
    RBrack,
    Comma
  } Type;
  const char* val;
};

std::string tok_type_to_string(Token::tok_type type);
std::vector<Token> tokenize(const std::string& prog);



