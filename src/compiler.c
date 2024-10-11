#include "compiler.h"
#include "typecheck.h"
#include "vec.h"
#include <stdint.h>
#include <stdio.h>

typedef unsigned long StackOffset;

static inline void fprint_atom(FILE* file, Atom atom) {
  for (uint_fast64_t i = 0; i < atom.atom.len; i++) {
    fprintf(file, "%c", ((char*) atom.atom.src)[i]);
  }
}

static inline void fspush(FILE* file, const char* rep) {
  fprintf(file, "push %s\n", rep);
}

StackOffset compile(Block block, FILE* file) {
  if (block.block_t >= _)
    return 8*(block.block_t - _);

  switch (block.block_t) {
    case FN:

    case VAR:
    case EVAL:
    case LIT_I8:
    case LIT_I64:
    case LIT_F64:
    case LIT_I8_PTR:
    default:
      break;
  }
}

void compiler(Blocks stream, FILE* file) {
  for_each(i, stream) compile(stream[i], file);
}

