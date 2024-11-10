/* Common definitions and functions in lithp
   This file should serve as the documentation
   point for all extension to the base language
 */

typedef enum Ltype {
  Dead,
  Number,
  Symbol,
  String
} Ltype;

typedef struct Lval {
  struct Lval* nxt;
  struct Lval* prv;
  struct Lval* cld;
  struct Lval* prt;
  Ltype ltype;
  union {
    float number;
    const char* symbol;
    char* string;
  };
} Lval;
