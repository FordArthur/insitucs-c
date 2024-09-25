#ifndef INSITUCS_TYPECHECK_HEADER
#define INSITUCS_TYPECHECK_HEADER

#include <stdint.h>
#include "context.h"
#include "parser.h"

typedef struct TypedStream {
  Type (*read_type)(Token);
} TypedStream;

typedef enum BlockT {
  FN,
  VAR,
  BLOCK /* is meant to map to something like "EXPR" */
} BlockT;

typedef struct Block {
  BlockT block_t;
} Blocks;

typedef struct TypecheckResult {
  bool passes;
  Error* errors;
  Blocks* blocks;
} TypecheckResult;

TypecheckResult typecheck(TypedStream tstream, AST ast, Error* error_buf);

#endif // !INSITUCS_TYPECHECK_HEADER
