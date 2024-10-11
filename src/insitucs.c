#include "insitucs.h"
#include "vec.h"
#include <stdio.h>

char cs_consume_char(void** stream) {
  return *((*(char**)stream)++);
}

char cs_consume_char_ahead(void** stream) {
  return *(++(*(char**)stream));
}

char cs_get_char(void* stream) {
  return *(char*) stream;
}
char cs_look_around(void* stream, long offset) {
  return ((char*) stream)[offset];
}

void cs_move_stream(void** stream, long offset) {
  stream += offset;
}

unsigned long cs_distance_between(void* stream, void* other) {
  return stream - other;
}

void* cs_copy_stream_offset(void* stream, long offset) {
  return stream + offset;
}

// TODO: check for post-typecheck function/var declaration
void cs_print_ast(Atom atm) {
  if (atm.atom_t == EXPR) {
    printf("[");
    for_each (i, atm.expr) {
      cs_print_ast(atm.expr[i]);
      printf(", ");
    }
    printf("\b\b]");
    return;
  }
  for (unsigned long i = 0; i < atm.atom.len; i++)
    putchar(((char*)atm.atom.src)[i]);
}

#ifdef DEBUG
// This looks like it defeats the purpose of blocks being isomorphic to atoms
// but notice this is under a debug block, we only need printed the extra information
//  whenever we are debugging
void cs_print_block(Block block) {
  if (block.block_t >= _) {
    printf("%%%d", block.block_t - _);
    return;
  }
  switch (block.block_t) {
    case EVAL: printf("EVAL: ["); goto print_expr;
    case FN:   printf("FN: [");   goto print_expr;
    case VAR:  printf("VAR: [");
      print_expr:
      for_each (i, block.expr) {
        cs_print_block(block.expr[i]);
        printf(", ");
      }
      printf("\b\b]");
      return;
    default:
      for (unsigned long i = 0; i < block.atom.len; i++)
        putchar(((char*)block.atom.src)[i]);
      return;
  }
}
#endif

void cs_print_error(Error err) {
  printf("Error: %s\n, in atom: ", err.message);
  cs_print_ast(err.ill_atom);
  putchar('\n');
}

static Stream console_stream = (Stream) {
  .pstream = (ParseableStream) {
    .stream = NULL,
    .get_char = &cs_get_char,
    .move_stream = &cs_move_stream,
    .look_around = &cs_look_around,
    .consume_char = &cs_consume_char,
    .distance_between = &cs_distance_between,
    .copy_stream_offset = &cs_copy_stream_offset,
    .consume_char_ahead = &cs_consume_char_ahead
  },
  .estream = (ErrorStream) {
    .print_error = &cs_print_error
  },
#ifdef DEBUG
  .rstream = (PrintableStream) {
    .print_ast = &cs_print_ast,
    .print_block = &cs_print_block
  }
#endif
};
static Stream file_stream;
//static Stream repl_stream; ?

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: \033[1m%s mode [args]\033[0m\n\twhere \"mode\" = f, standing for \"file\" |  c, standing for \"console\"\n", argv[0]);
    return 1;
  }
  static Stream stream;
  switch (argv[1][0]) {
    case 'f':
      file_stream.pstream.stream = argv[2];
      stream = file_stream;
      break;
    case 'c':
      console_stream.pstream.stream = argv[2]; 
      stream = console_stream;
      break;
    default:
    fprintf(stderr, "Usage: %s mode [args]\n\t\033[1mwhere \"mode\" = f, standing for \"file\" |  c, standing for \"console\"\033[0m\n", argv[0]);
    return 1;
      break;
  }
  ParseResult parsed = parser(stream.pstream);
  if (!parsed.is_correct_ast) {
    for_each(i, parsed.errors) {
      stream.estream.print_error(parsed.errors[i]);
    }
    return 1;
  }
#ifdef DEBUG
  for_each(i, parsed.ast) {
    stream.rstream.print_ast(parsed.ast[i]);
    printf("\n");
  }
#endif
  TypecheckResult checked = typecheck(parsed.ast, parsed.errors);
  if (!checked.passes) {
    for_each(i, checked.errors) {
      stream.estream.print_error(checked.errors[i]);
    }
    return 1;
  }
#ifdef DEBUG
  for_each(i, checked.blocks) {
    stream.rstream.print_block(checked.blocks[i]);
  }
#endif 
  fflush(stdout);
  /*
  compiler((AST) checked.blocks, NULL);
  */
}

;;; ;;; ;;; ;;;    ;;; ;;;    ;;; ;;; ;;;     ;  ;   ;;; ;;;  ;  ; ; ;;  ;;; ;;;
;   ;    ;  ; ;    ;   ;      ; ; ; ; ; ;    ; ; ;   ;     ; ; ; ;;; ; ; ; ; ; ;
;;; ;;;  ;  ; ;    ;;; ;;;    ;;; ; ; ;;;    ;;; ;   ;;;   ; ;;; ; ; ; ; ;;; ; ;
;     ;  ;  ; ;    ;     ;    ;   ; ; ;;     ; ; ;   ;   ; ; ; ; ; ; ; ; ;;  ; ;
;;; ;;;  ;  ;;;    ;;; ;;;    ;   ;;; ; ;    ; ; ;;; ;;; ;;; ; ; ; ; ;;  ; ; ;;;
