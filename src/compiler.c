#include "compiler.h"
#include "vec.h"
#include <stdint.h>
#include <stdio.h>

typedef unsigned long StackOffset;

static inline void fprint_atom(FILE* file, Atom atom) {
  for (uint_fast64_t i = 0; i < atom.atom.len; i++) {
    fprintf(file, "%c", ((char*) atom.atom.src)[i]);
  }
}

StackOffset compile(Atom block, FILE* file) {
  if (block.atom_t == EXPR) {
    switch (block.expr[0].atom_t) {
      case FUNCTION:
        fprint_atom(file, block.expr[1]);
        fprintf(file, ":\n");
        break;
      case LET:
        break;
      case EXPR:
        for (uint_fast64_t i = 1; i < _get_header(block.expr)->size; i++) {
          fprintf(file, "push [rbp + %lu]\n", compile(block.expr[i], file));
        }
        fprintf(file, "call ");
        fprint_atom(file, block.expr[0]);
        fprintf(file, "\n");
        break;
      default:
        __builtin_unreachable();
    }
  }
}

void compiler(AST stream, FILE* file) {
  for_each(i, stream) compile(stream[i], file);
}
