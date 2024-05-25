#ifndef INSITUCS_PARSER_HEADER
#define INSITUCS_PARSER_HEADER

#include <string.h>
#include "sized_array.h"
#include "vec.h"

typedef enum Types {
  Expression,
  Function,
  Number,
  String,
  Vector,
// Dictionary,
// Closure
} Types;

typedef struct _InsiType InsiType;

struct _InsiType {
  Types type;
  union {
    InsiType* exp;
    char* var;
    char* func;
    double num;
    char* str;
    InsiType* vec;
//  hash dict;
//  ???? clo;
  } info;
}; 

s_array parser(char* stream);

#endif // !INSITUCS_PARSER_HEADER
