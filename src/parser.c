#include "parser.h"
#include "trie.h"

static inline char consume_char(ParseableStream* stream) {
  return stream->consume_char(&stream->stream);
}

static inline char consume_char_ahead(ParseableStream* stream) {
  return stream->consume_char_ahead(&stream->stream);
}

static inline char get_char(ParseableStream stream) {
  return stream.get_char(stream.stream);
}

static inline char look_around(ParseableStream stream, long offs) {
  return stream.look_around(stream.stream, offs);
}

static inline void move_stream(ParseableStream* stream, long offs) {
  return stream->move_stream(&stream->stream, offs);
}

static inline void* copy_stream_offset(ParseableStream stream, long offs) {
  return stream.copy_stream_offset(stream.stream, offs);
}

static inline unsigned long distance_between(ParseableStream stream, void* other_stream) {
  return stream.distance_between(stream.stream, other_stream);
}

static inline bool is_end_of_atom(char c) {
  return isspace(c) || c == ')' || c == '(';
}

static inline bool is_quote(char c) {
  return c == '\"';
}

static inline int constant_true(int _) { return true; }

static TrieNode* atom_trie;
static bool is_correct_ast = true;
static Error* errors;

static Atom parse_expression(ParseableStream* stream) {
  char curchar = get_char(*stream);
  AtomT atom_t = IDENT;
  int (*validate_atom)(int) = &isalnum;
  // first int meant to represent bool
  // second int meant to represent char
  // because of course char x = (long) 42; is ok
  // but dont you dare do such things in a function type

  bool (*end_atom)(char) = &is_end_of_atom;
  switch (curchar) {
    case '(':
      consume_char(stream);
      Expression inner_expr = new_vector_with_capacity(*inner_expr, 8);
      Atom expr = parse_expression(stream);
      push(inner_expr, expr);
      for (curchar = get_char(*stream); curchar && curchar != ')'; curchar = get_char(*stream)) {
        expr = parse_expression(stream);
        push(inner_expr, expr);
      }
      if (!curchar) {
        is_correct_ast = false;
        push(errors, ((Error) {
          .message = "Unmatched parenthesis",
          .ill_atom.atom.src = expr.expr,
          .ill_atom.atom.len = distance_between(*stream, expr.expr)
        }));
        return (Atom) {};
      } else consume_char(stream);
      return (Atom) {
        .atom_t = EXPR,
        .expr = inner_expr
      };
    case ')':{
      void* stream_with_paren = copy_stream_offset(*stream, 0);
      consume_char(stream);
      is_correct_ast = false;
      push(errors, ((Error) { 
        .message = "Extraneous closing parenthesis",
        .ill_atom.atom.src = stream_with_paren,
        .ill_atom.atom.len = 1
      }));
      return (Atom) {};
    }
    case ' ':
    case '\n':
    case '\t':
      while (curchar && isspace(curchar)) curchar = consume_char_ahead(stream);
      if (!curchar) return (Atom) {};
      return parse_expression(stream);
    case '\'':
      consume_char(stream);
      void* start_char = copy_stream_offset(*stream, 0);
      if ((curchar = consume_char(stream)) == '\\') {
        consume_char(stream);
      }
      if (curchar != '\'') {
        is_correct_ast = false;
        push(errors, ((Error) {
          .message = "Character longer than typical or escaped",
          .ill_atom.atom.src = start_char,
          .ill_atom.atom.len = distance_between(*stream, start_char)
        }))
        return (Atom) {};
      }
      return (Atom) { 
        .atom_t = CHAR,
        .atom.src = start_char
      };
    case '"':
      atom_t = STRING;
      consume_char(stream);
      validate_atom = &constant_true;
      end_atom = &is_quote;
      goto parse_atom;
    case '+':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      atom_t = REAL;
      validate_atom = &isdigit;
    default: parse_atom: {
      TrieNode* trie_pos = step_up(curchar, atom_trie);
      void* ident_start = copy_stream_offset(*stream, 0);
      curchar = consume_char_ahead(stream);
      while (curchar && !end_atom(curchar)) {
        if (!(*validate_atom)(curchar)) {
          push(errors, ((Error) { 
            .message = "This can get a better error message, dont worry. In the meanwhile, stop doing silly errors",
            .ill_atom.atom.src = stream->stream,
            .ill_atom.atom.len = distance_between(*stream, ident_start)
          }));
          is_correct_ast = false;
          return (Atom) {};
        }
        if (trie_pos) trie_pos = step_up(curchar, trie_pos);
        curchar = consume_char_ahead(stream);
      }
      unsigned long len = distance_between(*stream, ident_start);
      if (atom_t == STRING) consume_char(stream);
      while (curchar && isspace(curchar)) curchar = consume_char_ahead(stream);
      return (Atom) {
        .atom_t = trie_pos && trie_pos->is_terminal ? trie_pos->value : atom_t,
        .atom.src = ident_start,
        .atom.len = len
      };
    }
  }
}

ParseResult parser(ParseableStream stream) {
  _Static_assert(sizeof(AST) == sizeof(Error*), "AST and Error* must be of the same size");
  
  atom_trie = create_node(0, -1);
  insert_trie("function", FUNCTION , atom_trie);
  insert_trie("let"     , LET      , atom_trie);
  insert_trie(":"       , LET_TYPE , atom_trie);
  insert_trie("char"    , CHAR_ST  , atom_trie);
  insert_trie("string"  , STRING_ST, atom_trie);
  insert_trie("int"     , INT_ST   , atom_trie);
  insert_trie("real"    , REAL_ST  , atom_trie);

  // since the parser switch breaks these, we fix them here
  // this however is only temporal becasue we also would want
  // to support arbitrary symbols
  insert_trie("+"       , IDENT    , atom_trie);
  insert_trie("-"       , IDENT    , atom_trie);

  AST ast = new_vector_with_capacity(*ast, 64);
  errors = new_vector_with_capacity(*errors, 16);

  while (get_char(stream)) {
    Atom atm = parse_expression(&stream);
    for (char curchar = get_char(stream); curchar && isspace(curchar);) curchar = consume_char_ahead(&stream);
    push(ast, atm);
  }

  return (ParseResult) {
    .is_correct_ast = is_correct_ast,
    .ast = ast,
    .errors = errors
  };
}
