#ifndef INSITUCS_TYPECHECK_HEADER
#define INSITUCS_TYPECHECK_HEADER

#include <stdint.h>
#include "context.h"
#include "parser.h"

/* 
 * Names following the "_n" schema where n is a digit are not
 * utilized and are just so that BlockT and AtomT are
 * isomorphic
 */
typedef enum BlockT {
  EVAL,
  LIT_I8, LIT_I64, LIT_F64, LIT_I8_PTR,
  ARR,
  FN, VAR,
  _1, _2, _3, _4,
  _ /* !! This must be always the final element !!
  * This is not utilized as an element but rather
  * as a marker to where the type starts encoding
  * an argument rather than a type
  */
} BlockT;

/* 
 * Partially isomorphic to Type, we just check whether the type
 * is bigger than ANY to check for functions and if so we return
 * CALLABLE_PTR, otherwise the isomorphism is trivial
 */
typedef enum AsmTypeT {
  CALLABLE_PTR, I8, I64, F64, I8_PTR
} AsmTypeT;

/*
 * What i want is for sizeof(Block) == sizeof(Atom)
 * since if this is the case we can reutilize the allocated
 * vectors and dramatically save on memory usage
 * Also, the same function that prints Atoms works on Blocks
 * which is really useful for when the typechecker fails
 * while modifying some part of the ast to be blocks
 * the same function for reporting can be used
 */
typedef struct Block {
  BlockT block_t;
  union {
    struct {
      unsigned char arg_len;
    } fn_info;
    AsmTypeT asm_t;
  };
  union {
    Token atom;
    struct Block* expr;
  };
} Block;

typedef Block* Blocks;

typedef struct TypecheckResult {
  bool passes;
  Error* errors;
  Blocks blocks;
} TypecheckResult;

TypecheckResult typecheck(AST ast, Error* error_buf);

#endif // !INSITUCS_TYPECHECK_HEADER
