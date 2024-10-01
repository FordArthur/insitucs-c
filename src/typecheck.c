#include "typecheck.h"
#include "context.h"
#include "vec.h"

static inline bool is_type(Atom t) {
  return CHAR_ST <= t.atom_t && t.atom_t >= STRING_ST;
}

static inline Type to_type(Atom t) {
  return t.atom_t - CHAR_ST + CHAR_T;
}

#define Strict(f, e) if(!(f)) { passes = false; /* err */ return ANY; }
#define Type_Assignment(at) Strict(expr.expr[at].atom_t == EXPR && expr.expr[at].expr[0].atom_t == LET_TYPE && expr.expr[at].expr[1].atom_t == IDENT && is_type(expr.expr[at].expr[2]), "Expected type assignment");

static bool passes = true;
Error* errors; // with the hopes that it will share the memory location with where it was created
//static char* in_scope[255] = {0};
//static uint_fast8_t scope_top = 0;
//static uint_fast8_t scope_levels[255] = {0};
//static uint_fast8_t scope_levels_top = 0;

static inline bool type_eq_up_to_curry(Type t1, Type t2) {
  for (int_fast8_t pt1 = t1 & 15, pt2 = t2 & 15; t1 && t2; t1 = t1 >> 4, t2 = t2 >> 4, pt1 = t1 & 15, pt2 = t2 & 15)
    if (!(pt1 == ANY || pt2 == ANY? true : pt1 == pt2)) return false;
  return true;
}

static inline Type typechecker(Atom expr) {
  switch (expr.atom_t) {
    case EXPR:
      switch (expr.expr[0].atom_t) {
        case FUNCTION:
          Type_Assignment(1);
          const Type ret_type = to_type(expr.expr[1].expr[2]);
          Type func_type = 0;
          uint_fast8_t i = 1;
          for (; i <= 16 && expr.expr[i].atom_t == EXPR && expr.expr[i].atom_t == LET_TYPE; i++) {
            Type_Assignment(i);
            func_type |= to_type(expr.expr[i].expr[2]) << (4*(i - 1));
            // If this is confusing, recall that the maximum a type can be is 15 (the ANY type), so they fit in 4 bit chunks
            // thus, 8 bytes / 4 bits = 64 bits / 4 bits = 16 types
            // as we reserve a spot for the return type, this leaves us with 15 arguments
            // that we fit in a single 8 byte long by upping each value 4 bits times their position
          }
          func_type |= ret_type >> (4*(i - 1));
          Type body_type;
          for_each(j, expr.expr) {
            body_type = typechecker(expr.expr[i + j]);
          }
          Strict(type_eq_up_to_curry(body_type, ret_type), "Type mismatch between declared return type and return expression");
          insert_context(expr.expr[1].expr[1].atom, func_type);
          return ANY;
        case LET:
          for_each(i, expr.expr) {
            Type_Assignment(i + 1);
            const Type var_type = to_type(expr.expr[i + 1].expr[2]);
            const Type expr_type = typechecker(expr.expr[i + 2]);
            Strict(type_eq_up_to_curry(var_type, expr_type), "Type mismatch between declared var type and expression");
            insert_context(expr.expr[i + 1].expr[1].atom, var_type);
            i++;
            return ANY;
          }
        case IDENT: {
          const Type func_type = lookup_context(expr.expr[0].atom);
          Strict(func_type, "Function does not exist");
          Type args_type = 0;
          uint_fast64_t j;
          // This all seems really stupid but its optimized away
          // its just so i dont have to manually write the for_each
          for_each(i, expr.expr) {
            if (!i) i++;
            args_type |= typechecker(expr.expr[i]) << (4*(i - 1));
            j = i;
          }
          Strict(j < 16, "Impossible number of arguments");
          Strict(type_eq_up_to_curry(func_type >> 4, args_type), "Function type does not match it's arguments");
          return func_type << (4*j);
        }
        default:
          // err
          return ANY;
          break;
      }
      break;
    case IDENT: {
      Type t = lookup_context(expr.atom);
      if (!t) {
        passes = false;
        push(errors, ((Error) {
          .message = "Variable not found"
        }));
        return ANY;
      }
      return t;
    }
      break;
    case LET_TYPE:
      passes = false;
      push(errors, ((Error) {
        .message = "Type assigns must be inside their apropiate definitions"
      }));
      return ANY;
    case FUNCTION:
      passes = false;
      push(errors, ((Error) {
        .message = "Function definition must be inside expression"
      }))
    case LET:
      passes = false;
      push(errors, ((Error) {
        .message = "Variable definition must be inside expression"
      }))
    default:
      return (Type) expr.atom_t;
  }
}

TypecheckResult typecheck(AST ast, Error* error_buf) {
  errors = error_buf;
  insert_context((Token) {
    .len = sizeof "+" - 1,
    .src = "+"
    // ANY becasue we dont support (yet) (but honestly probbably never) classes or weird types
  }, 0b111111110010);

#ifndef DEBUG
  for_each(i, ast) typechecker(ast[i]);
#else
  for_each(i, ast) {
    Type t = typechecker(ast[i]);
    printf("Deduced type: %lb\n", (unsigned long) t);
  }
#endif

  return (TypecheckResult) {
    .passes = passes,
    .errors = error_buf,
    .blocks = (Blocks*) ast
  };
}
