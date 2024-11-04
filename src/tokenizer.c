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
Token* TOKENS;
int TOKIND = 0;

//function definitions
void tokenize_symbol(void);
void tokenize_number(void);
void tokenize_string(void);
void incchar(void);
bool isdelimiter(char);

const char* CHR;
char CHRVAL;
void incchar(void) {
  CHR++;
  CHRVAL = *CHR;
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
  //init TOKENS
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
	TOKENS[TOKIND] = (Token){NULL,0,lbrack};
	TOKIND += 1;
	break;
      case ']':
	TOKENS[TOKIND] = (Token){NULL,0,rbrack};
	TOKIND += 1;
	break;
      case ',':
	TOKENS[TOKIND] = (Token){NULL,0,comma};
	TOKIND += 1;
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

  TOKENS[TOKIND] = (Token){NULL,0,end};
  
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
      TOKENS[TOKIND] = (Token){start,length,symbol};
      TOKIND += 1;
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
      TOKENS[TOKIND] = (Token){start,length,number};
      TOKIND += 1;
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
      TOKENS[TOKIND] = (Token){start,length,string};
      TOKIND += 1;
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

int main() {
  const char* expr = "\"hello\"";

  Token* tokens = tokenize(expr);

  int len = 0;
  for (;(*tokens).toktype != end;tokens++) {
    len += 1;
  }

  printf("%i",len);
}
