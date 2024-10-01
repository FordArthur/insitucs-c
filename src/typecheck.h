#ifndef INSITUCS_TYPECHECK_HEADER
#define INSITUCS_TYPECHECK_HEADER

#include <stdint.h>
#include "context.h"
#include "parser.h"

typedef enum BlockT {
  FN,
  VAR,
  BLOCK /* is meant to map EXPR */
} BlockT;

/*
 * What i want is for sizeof(Block) == sizeof(Atom)
 * since if this is the case we can reutilize the allocated
 * vectors and dramatically save on memory usage
 *
 * Therefore, the structure of blocks is defined implicitly:
 * FN: [name, [(: var type) ...] [(expressions)]]
 * VAR: [(: var type), (expression), (: var' type'), (expression') ...]
 * BLOCK: (expression)
 */
typedef struct Block {
  BlockT block_t;
  union {
    Token atom;
    struct Atom* expr;
  };
} Blocks;

typedef struct TypecheckResult {
  bool passes;
  Error* errors;
  Blocks* blocks;
} TypecheckResult;

TypecheckResult typecheck(AST ast, Error* error_buf);

#endif // !INSITUCS_TYPECHECK_HEADER
