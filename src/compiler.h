#ifndef INSITUCS_COMPILER_HEADER
#define INSITUCS_COMPILER_HEADER

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "vec.h"
#include "parser.h"
#include "typecheck.h"

void compiler(Blocks ast, FILE* file);

#endif // !INSITUCS_COMPILER_HEADER
