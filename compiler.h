#ifndef INSITUCS_COMPILER_HEADER
#define INSITUCS_COMPILER_HEADER

#include <string.h>
#include <stdbool.h>
#include "vec.h"
#include "scanner.h"
#include "vm.h"

/*
 * Returns a vector of bytes that represent the instruction stream
 */
Bytecode* compiler(Tokens stream);

#endif // !INSITUCS_COMPILER_HEADER
