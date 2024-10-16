#ifndef INSITUCS_HEADER
#define INSITUCS_HEADER

#include "vec.h"
#include "parser.h"
#include "typecheck.h"
#include "compiler.h"
#include <stdio.h>

#ifdef DEBUG
typedef struct PrintableStream {
  void (*print_ast)(Atom);
  void (*print_block)(Block);
} PrintableStream;
#endif

typedef struct Stream {
  ParseableStream pstream;
  ErrorStream estream;
#ifdef DEBUG
  PrintableStream rstream;
#endif
} Stream;

int main(int argc, char *argv[]);

#endif  // !INSITUCS_HEADER
