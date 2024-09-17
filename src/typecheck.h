#ifndef INSITUCS_TYPECHECK_HEADER
#define INSITUCS_TYPECHECK_HEADER

#include <stdint.h>
#include "context.h"
#include "parser.h"

typedef struct TypedStream {
  Type (*read_type)(Token);
} TypedStream;

typedef struct TypecheckResult {
  bool passes;
  Error* errors;
} TypecheckResult;

TypecheckResult typecheck(TypedStream tstream, AST ast, Error* error_buf);

#endif // !INSITUCS_TYPECHECK_HEADER
