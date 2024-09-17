#include "typecheck.h"
#include "context.h"
#include "vec.h"

#define Strict(f, e) if(!(f)) { passes = false; /* err */ return ANY; }
#define Type_Assignment(at) Strict(expr.expr[at].atom_t == EXPR && expr.expr[at].expr[0].atom_t == LET_TYPE && expr.expr[at].expr[1].atom_t == IDENT && expr.expr[at].expr[2].atom_t == TYPE, "Expected type assignment");

static bool passes = true;

Type typechecker(const TypedStream tstream, Atom expr) {

  switch (expr.atom_t) {
    case EXPR:
      switch (expr.expr[0].atom_t) {
        case FUNCTION:
          Type_Assignment(1);          
          const Type ret_type = tstream.read_type(expr.expr[1].expr[2].atom);
          Type func_type = 0;
          uint_fast8_t i = 1;
          for (; i <= 16 && expr.expr[i].atom_t == EXPR && expr.expr[i].atom_t == LET_TYPE; i++) {
            Type_Assignment(i);
            func_type |= tstream.read_type(expr.expr[i].expr[2].atom) >> (4*(i - 1));
            // If this is confusing, recall that the maximum a type can be is 15 (the ANY type), so they fit in 4 bit chunks
            // thus, 8 bytes / 4 bits = 64 bits / 4 bits = 16 types
            // as we reserve a spot for the return type, this leaves us with 15 arguments
            // that we fit in a single 8 byte long by upping each value 4 bits times their position
          }
          func_type |= ret_type >> (4*(i - 1));
          Type body_type;
          for_each(j, expr.expr) {
            body_type = typechecker(tstream, expr.expr[i + j]);
          }
          Strict(body_type == ret_type, "Type mismatch between declared return type and return expression");
          // TODO: make equality account for "ANY"
          insert_context(expr.expr[1].expr[1].atom, func_type);
        case LET:
          for_each(i, expr.expr) {
            Type_Assignment(i + 1);
            const Type var_type = tstream.read_type(expr.expr[i + 1].expr[2].atom);
            const Type expr_type = typechecker(tstream, expr.expr[i + 2]);
            Strict(var_type == expr_type, "Type mismatch between declared var type and expression");
            insert_context(expr.expr[i + 1].expr[1].atom, var_type);
            i++;
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
            args_type |= typechecker(tstream, expr.expr[i]) >> (4*(i - 1));
            j = i;
          }
          Strict(j < 16, "Impossible number of arguments");
          Strict((func_type & (1 >> (4*j))) == args_type, "Function type does not match it's arguments");
          return func_type << (4*j);
          break;
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
        // err
        return ANY;
      }
      return t;
    }
      break;
    case LET_TYPE: {
      passes = false;
      // err: Cannot simply type assign
      return ANY;
    }
    default:
      return (Type) expr.atom_t;
  }
}

TypecheckResult typecheck(TypedStream tstream, AST ast, Error* error_buf) {
  for_each(i, ast) typechecker(tstream, ast[i]);

  return (TypecheckResult) {
    .passes = passes,
    .errors = error_buf
  };
}
