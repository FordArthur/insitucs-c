#ifndef INSITUCS_PARSER_HEADER
#define INSITUCS_PARSER_HEADER

#include "vec.h"
#include "trie.h"
#include "stdbool.h"
#include <ctype.h>

typedef enum AtomT {
  EXPR,
  CHAR, INT, REAL, STRING, /* volatile position */  
  IDENT, TYPE,
  FUNCTION, LET, LET_TYPE,
} AtomT;

typedef struct ParseableStream {
  void* stream;
  char (*consume_char)(void**);
  char (*consume_char_ahead)(void**);
  char (*get_char)(void*);
  char (*look_around)(void*, long);
  void (*move_stream)(void**, long);
  void* (*copy_stream_offset)(void*, long);
  unsigned long (*distance_between)(void*, void*);
} ParseableStream;

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
  Atom ill_atom;
  char* message;
} Error;

typedef struct ErrorStream {
  void (*print_error)(Error);
} ErrorStream;

#ifdef DEBUG
typedef struct PrintableStream {
  void (*print_stream)(Atom);
} PrintableStream;
#endif

typedef Atom* Expression;

typedef Expression AST;

typedef struct ParseResult {
  bool is_correct_ast;
  Error* errors;
  AST ast;
} ParseResult;

ParseResult parser(ParseableStream stream);

#endif  // INSITUCS_PARSER_HEADER
