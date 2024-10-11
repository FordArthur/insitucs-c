#ifndef INSITUCS_STRINGMAP_HEADER
#define INSITUCS_STRINGMAP_HEADER

/* !! IMPORTANT: There only exist one context, that is, all calls affect the same underlying object  !! */

#include "parser.h"
#include "vec.h"
#include <stdint.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

typedef enum UnitTypes {
  BOTTOM,
  CHAR_T,
  INT_T,
  REAL_T,
  STRING_T,
  ANY = 15,
} UnitTypes;

// CHAR_T INT_T STRING_T INT_T 0000 0000 ....

typedef uint_fast64_t Type;

typedef struct _Entry {
  Token name;
  Type type;
  int_fast64_t counter;
} _Entry;

typedef _Entry* _Bucket;

typedef struct CtxVal {
  Type type;
  uint_fast64_t counter;
} CtxVal;

void insert_context(Token name, Type type, uint_fast64_t counter);

void delete_context(Token name);

CtxVal lookup_context(Token name);


#endif // !INSITUCS_STRINGMAP_HEADER
