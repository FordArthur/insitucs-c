#include "compiler.h"

Bytecode codegen(Scanned** streamp) {
  Scanned* stream = *streamp;
}

Bytecode* compiler(Tokens stream) {
  if (!stream.is_correct_stream) {
    // report errors
    return 0;
  }
  return 0;
}
