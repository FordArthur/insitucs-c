#include "parser.h"
#include "vec.h"

static inline char consume_char(Stream* stream) {
  return stream->consume_char(stream->stream);
}

static inline char get_char(Stream stream) {
  return stream.get_char(stream.stream);
}

static inline char look_around(Stream stream, long offs) {
  return stream.look_around(stream.stream, offs);
}

static inline void move_stream(Stream* stream, long offs) {
  return stream->move_stream(stream->stream, offs);
}

static inline void* copy_stream_offset(Stream stream, long offs) {
  return stream.copy_stream_offset(stream.stream, offs);
}

static inline unsigned long distance_between(Stream stream, void* other_stream) {
  return stream.distance_between(stream.stream, other_stream);
}

static inline AtomT recognize_atom(Stream stream, unsigned long len) {
  return stream.recognize_atom(stream.stream, len);
}

static bool is_correct_ast = true;

static Atom parse_expression(Stream* stream) {
  char curchar = get_char(*stream);
  if (curchar == '(') {
    consume_char(stream);
    Expression inner_expr = new_vector_with_capacity(*inner_expr, 8);
    Atom expr = parse_expression(stream);
    push(inner_expr, expr);
    for (curchar = get_char(*stream); curchar && curchar != ')'; curchar = get_char(*stream)) {
      expr = parse_expression(stream);
      push(inner_expr, expr);
    }
    return (Atom) {
      .atom_t = EXPR,
      .expr = inner_expr
    };
  } if (curchar == ')') {
    // err
    __builtin_unreachable();
  } else {
    void* ident_start = copy_stream_offset(*stream, 0);
    while (consume_char(stream) != ' ');
    unsigned long len = distance_between(*stream, ident_start);
    return (Atom) {
      .atom_t = recognize_atom(*stream, len),
      .atom.src = ident_start,
      .atom.len = len
    };
  }
}

ParseResult parser(Stream stream) {
  _Static_assert(sizeof(AST) == sizeof(Error*), "AST and Error* must be of the same size");
  // idk how to check that stream and error are in the same spot on the struct, if you do pls PR

  AST ast = new_vector_with_capacity(*ast, 64);
  Error* errors = new_vector_with_capacity(*errors, 16);

  while (get_char(stream)) {
    Atom atm = parse_expression(&stream);
    push(ast, atm);
  }

  return (ParseResult) {
    .is_correct_ast = is_correct_ast,
    // This is fine, since ast and errors are both pointers (they have the same size)
    // and they both are in the same spot of the struct, bitwise on the struct they are the same
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-type-mismatch"
    .ast = is_correct_ast? ast : errors
#pragma GCC diagnostic pop
  };
}
