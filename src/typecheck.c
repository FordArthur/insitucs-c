#include "typecheck.h"
#include "context.h"
#include "vec.h"
#include <stddef.h>

static inline bool is_type(Atom t) {
  return CHAR_ST <= t.atom_t && t.atom_t >= STRING_ST;
}

static inline Type to_type(Atom t) {
  return t.atom_t - CHAR_ST + CHAR_T;
}

#define Strict(f, t, e) if(!(f)) { passes = false; push(errors, ((Error) { .message = e, .ill_atom = t }) ); return ANY; }
#define StrictGoto(f, t, e, l) if(!(f)) { passes = false; /* err */ goto l; }
#define Type_Assignment(at) Strict(expr->expr[at].atom_t == EXPR && expr->expr[at].expr[0].atom_t == LET_TYPE && expr->expr[at].expr[1].atom_t == IDENT && is_type(expr->expr[at].expr[2]), expr->expr[at], "Expected type assignment");

static bool passes = true;
Error* errors; // with the hopes that it will share the memory location with where it was created
static Token in_scope[255] = {0};
static uint_fast8_t scope_top = 0;
static uint_fast8_t scope = 0;
static uint_fast8_t scope_table[255] = {0};

static inline AsmTypeT to_AsmT(Type t) {
  return t > ANY ? CALLABLE_PTR : t;
}

static inline bool type_eq_up_to_curry(Type t1, Type t2) {
  for (int_fast8_t pt1 = t1 & 15, pt2 = t2 & 15; t1 && t2; t1 = t1 >> 4, t2 = t2 >> 4, pt1 = t1 & 15, pt2 = t2 & 15)
    if (!(pt1 == ANY || pt2 == ANY? true : pt1 == pt2)) return false;
  return true;
}

static inline bool type_eq(Type t1, Type t2) {
  for (int_fast8_t pt1 = t1 & 15, pt2 = t2 & 15; t1 && t2; t1 = t1 >> 4, t2 = t2 >> 4, pt1 = t1 & 15, pt2 = t2 & 15)
    if (!(pt1 == ANY || pt2 == ANY? true : pt1 == pt2)) return false;
  return t1 == t2; // becasue we stop whenever t1 or t2 are 0, this will be true if and only if they are both 0
}

static inline Type typechecker(Atom* expr) {
  switch (expr->atom_t) {
    case EXPR:
      switch (expr->expr[0].atom_t) {
        case FUNCTION:
          Type_Assignment(1);
          const Type ret_type = to_type(expr->expr[1].expr[2]);
          Type func_type = ret_type;
          uint_fast8_t i = 1;
          for (; i <= 16 && expr->expr[i].atom_t == EXPR && expr->expr[i].atom_t == LET_TYPE; i++) {
            Type_Assignment(i);
            Type decl_arg_type = to_type(expr->expr[i].expr[2]);
            func_type |= decl_arg_type << (4*i);
            // If this is confusing, recall that the maximum a type can be is 15 (the ANY type), so they fit in 4 bit chunks
            // thus, 8 bytes / 4 bits = 64 bits / 4 bits = 16 types
            // as we reserve a spot for the return type, this leaves us with 15 arguments
            // that we fit in a single 8 byte long by upping each value 4 bits times their position
            insert_context(expr->expr[i].expr[1].atom, decl_arg_type, scope_top);
            in_scope[scope_top] = expr->expr[i].expr[1].atom;
            scope_table[scope_top++] = scope + 1;
          }
          StrictGoto(i <= 16, expr.expr[i], "Impossible number of arguments", cleanup);
          insert_context(expr->expr[1].expr[1].atom, func_type, 0);
          in_scope[scope_top] = expr->expr[1].expr[1].atom;
          scope_table[scope_top++] = scope;
          scope++;
          Type body_type;
          uint_fast64_t k = i;
          for_each(j, expr->expr) {
            body_type = typechecker(&expr->expr[i + j]);
            k = j;
          }
          scope--;
        cleanup:  
          for (; scope_top && scope_table[scope_top] != scope; scope_top--)
            delete_context(in_scope[scope_top]);
          Strict(type_eq(body_type, ret_type), expr->expr[k], "Type mismatch between declared return type and return expression");
          *(Block*)expr = (Block) {
            .block_t = FN,
            .fn_info.arg_len = i,
            .expr = expr->expr
          };
          return ANY;
        case LET:
          // Notice how this section does not need to perform any transformation to get the blocks!
          for_each(i, expr->expr) {
            Type_Assignment(i + 1);
            const Type var_type = to_type(expr->expr[i + 1].expr[2]);
            const Type expr_type = typechecker(&expr->expr[i + 2]);
            Strict(type_eq(var_type, expr_type), expr->expr[i + 1], "Type mismatch between declared var type and expression");
            insert_context(expr->expr[i + 1].expr[1].atom, var_type, scope_top);
            in_scope[scope_top] = expr->expr[i + 1].expr[1].atom;
            scope_table[scope_top++] = scope;
            i++;
          }
          return ANY;
        case IDENT: {
          const Type func_type = typechecker(expr->expr);
          Strict(func_type, expr->expr[0], "Function does not exist");
          Type args_type = 0;
          uint_fast64_t i;
          for (i = 1; i < _get_header(expr->expr)->size; i++) {
            args_type |= typechecker(&expr->expr[i]) << (4*(i - 1));
          }
          Strict(i <= 16, expr->expr[i], "Impossible number of arguments");
          Strict(type_eq_up_to_curry(func_type >> 4, args_type), expr->expr[0], "Function type does not match it's arguments");
          const Type app_type = func_type << (4*i);
          *(Block*)expr->expr = (Block) {
            .block_t = EVAL,
            .asm_t = to_AsmT(app_type),
            // Yes, this could be an expression also
            // but since its an union it doesnt matter
            .atom = expr->atom
          };
          return app_type;
        }
        default:
          // err
          return ANY;
          break;
      }
      break;
    case IDENT: {
      const CtxVal val = lookup_context(expr->atom);
      const Type t = val.type;
      if (!t) {
        passes = false;
        push(errors, ((Error) {
          .message = "Variable not found"
        }));
        return ANY;
      }
      *(Block*)expr->expr = (Block) {
        .block_t = val.counter + _,
        .asm_t = to_AsmT(t),
        .atom = expr->atom
      };
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
      return (Type) expr->atom_t;
  }
}

TypecheckResult typecheck(AST ast, Error* error_buf) {
  _Static_assert(sizeof(Atom) == sizeof(Block), "Atom and Block do not have the same size");
  _Static_assert(offsetof(Atom, atom_t) == offsetof(Block, block_t), "Atom_t and Block_t do not have the same offset");

  errors = error_buf;
  insert_context((Token) {
    .len = sizeof "+" - 1,
    .src = "+"
    // ANY becasue we dont support (yet) (but honestly probbably never) classes or weird types
  }, 0b111111110010, 0);

#ifndef DEBUG
  for_each(i, ast) typechecker(ast + i);
#else
  for_each(i, ast) {
    Type t = typechecker(ast[i]);
    printf("Deduced type: %lb\n", (unsigned long) t);
  }
#endif

  return (TypecheckResult) {
    .passes = passes,
    .errors = error_buf,
    .blocks = (Block*) ast
  };
}
