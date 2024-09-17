#ifndef INSITUCS_PARSER_HEADER
#define INSITUCS_PARSER_HEADER

#include "vec.h"
#include "stdbool.h"

typedef enum AtomT {
  EXPR,
  /* volatile position */
  CHAR, INT, REAL, STRING,
  IDENT, TYPE,
  FUNCTION, LET, LET_TYPE
} AtomT;

typedef struct Stream {
  void* stream;
  char (*consume_char)(void**);
  char (*get_char)(void*);
  char (*look_around)(void*, long);
  void (*move_stream)(void**, long);
  void* (*copy_stream_offset)(void*, long);
  unsigned long (*distance_between)(void*, void*);
  AtomT (*recognize_atom)(void*, unsigned long);
} Stream;

typedef struct Token {
  void* src;
  /* length in bytes */
  unsigned long len;
} Token;

typedef struct Atom {
  AtomT atom_t;
  union {
    Token atom;
    struct Atom* expr;
  };
} Atom;

typedef struct Error {

} Error;

typedef Atom* Expression;

typedef Expression AST;

typedef struct ParseResult {
  bool is_correct_ast;
  union {
    Error* errors;
    AST ast;
  };
} ParseResult;

ParseResult parser(Stream stream);

#endif  // INSITUCS_PARSER_HEADER
