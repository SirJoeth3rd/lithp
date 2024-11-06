#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum TokenType_ {
  lbrack,
  rbrack,
  comma,
  symbol,
  number,
  string,
  end
} TokenType;

typedef struct Token_ {
  const char* pos;
  int length;
  TokenType toktype;
} Token;

//GLOBALS
const int BUFFER = 100;

//function definitions
void tokenize_symbol(void);
void tokenize_number(void);
void tokenize_string(void);
void incchar(void);
bool isdelimiter(char);
void print_tokens(Token*);
void add_token(const char*,int,TokenType);

const char* CHR;
char CHRVAL;
void incchar(void) {
  CHR++;
  CHRVAL = *CHR;
}

Token* TOKENS;
int TOKIND = 0;
void add_token(const char* p, int l, TokenType t) {
  TOKENS[TOKIND] = (Token){p,l,t};
  TOKIND++;
}

bool isdelimiter(char chr) {
  switch (chr) {
  case '[':
  case ']':
  case ',':
    return true;
  default:
    return false;
  }
}

typedef enum TokenizationError_ {
  generic_error,
  multiple_dots_in_number,
  unexpected_end_of_string,
  newline_in_string
} TokenizationError;

bool ERROR = false; 
void handle_error(TokenizationError error) {
  printf("got error\n");
  TOKENS[0] = (Token){"error",5,string};
  TOKENS[1] = (Token){NULL,0,end};
  ERROR = true;
}

Token* tokenize(const char* string) { 
  TOKENS = malloc(sizeof(Token) * BUFFER);
  CHR = string;
  CHRVAL = *CHR;
  
  while (CHRVAL) {
    if (isspace(CHRVAL)) {
      incchar();
      continue;
    } else if (isalpha(CHRVAL)) {
      tokenize_symbol();
    } else if (isdigit(CHRVAL)) {
      tokenize_number();
    } else {
      switch (CHRVAL) {
      case '"':
	tokenize_string();
	break;
      case '[':
	add_token(NULL,0,lbrack);
	break;
      case ']':
	add_token(NULL,0,rbrack);
	break;
      case ',':
	add_token(NULL,0,comma);
	break;
      default:
	handle_error(generic_error);
      }
      incchar();
    }
    if (ERROR) {
      break;
    }
  }

  //ZII:)
  add_token(NULL,0,end);
  
  return TOKENS;
}

void tokenize_symbol() {
  const char* start = CHR;
  int length = 0;
  while (CHRVAL) {
    if (isalnum(CHRVAL)) {
      length++;
      incchar();
    } else if (isspace(CHRVAL) || isdelimiter(CHRVAL)) {
      add_token(start,length,symbol);
      return;
    } else {
      handle_error(generic_error);
      return;
    }
  }
  if (!CHRVAL) {
    handle_error(unexpected_end_of_string);
  }
}

void tokenize_number() {
  const char* start = CHR;
  int length = 0;
  bool isfloat = false;
  while (CHRVAL) {
    if (isdigit(CHRVAL)) {
      length++;
      incchar();
    }  else if (CHRVAL == '.') {
      if (isfloat) {
	handle_error(multiple_dots_in_number);
	return;
      } else {
	isfloat = true;
	length++;
	incchar();
      }
    } else if (isspace(CHRVAL) || isdelimiter(CHRVAL)) {
      add_token(start,length,number);
      return;
    } else {
      handle_error(generic_error);
      return;
    }
  }
  if (!CHRVAL) {
    handle_error(unexpected_end_of_string);
  }
}

void tokenize_string() {
  incchar(); // exclude the starting "
  const char* start = CHR;
  int length = 0;
  while (CHRVAL) {
    if (CHRVAL == '"') {
      add_token(start,length,string);
      return;
    } else if (CHRVAL == '\n') {
      handle_error(newline_in_string);
      return;
    }
    length++;
    incchar();
  }
  if (!CHRVAL) {
    handle_error(unexpected_end_of_string);
  }
}

void print_tokens(Token* tokens) {
  for (Token* token = tokens; token->toktype != end; token++) {
    switch (token->toktype) {
    case string:
      printf("STRING <%.*s>\n",token->length, token->pos);
      break;
    case number:
      printf("NUMBER <%.*s>\n",token->length, token->pos);
      break;
    case symbol:
      printf("SYMBOL <%.*s>\n",token->length, token->pos);
      break;
    case lbrack:
      printf("LBRACK\n");
      break;
    case rbrack:
      printf("RBRACK\n");
      break;
    case comma:
      printf("COMMA\n");
      break;
    case end:
      printf("END {not supposed to be reachable}\n");
      break;
    }
  }
}

int main() {
  const char* expr = "func[1,b,c,77.78]";

  Token* tokens = tokenize(expr);

  print_tokens(tokens);
}
