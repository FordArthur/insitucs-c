#include <stdbool.h>
#include "vec.h"
#include "scanner.h"
#include <stdio.h>

#define EOFval 26
#define mktok(t, l, token) (Scanned) { .is_ok = true, .line = _LINE, .index = _INDEX, .res.tok.type = t, .res.tok.length = l, .res.tok.tok = token }
#define throwerr(e) (Scanned) { .is_ok = false, .line = _LINE, .index = _INDEX, .res.err = e }

static unsigned int _LINE = 0;  //
static unsigned int _INDEX = 0; // Will keep track of the position as we are parsing

static bool _PARSING_STRING = false;
static bool _IS_PEAKED_TOKEN = false;
static Scanned _PEAKED_TOKEN;

inline static bool in_str(const char c, const char* s) {
  if (!c) return true;
  unsigned int i = 0;
  for (; s[i] && s[i] != c; i++);
  return s[i];
}

// Consume a character from the stream and update the relevant information
static inline char consume(char** stream) {
  char consumed = (*stream)[0];
  if (consumed == '\0')
    return '\0';
  (*stream)++;
  if (consumed == '\n') {
    _LINE++;
    _INDEX = 0;
  } else // maybe we have to handle \t?
    _INDEX++;

  return consumed;
}

Scanned scan(char** streamp) {
  if (_IS_PEAKED_TOKEN) {
    _IS_PEAKED_TOKEN = false;
    return _PEAKED_TOKEN;
  }
  char tokchar;
  switch ((tokchar = consume(streamp))) {
    case '(':
      return mktok(OPEN_PAREN, 1, 0);
      break;
    case ')':
      return mktok(CLOSE_PAREN, 1, 0);
      break;
    case '[':
      return mktok(OPEN_BRACKET, 1, 0);
      break;
    case ']':
      return mktok(CLOSE_BRACKET, 1, 0);
      break;
    case '{':
      return mktok(OPEN_CURLY, 1, 0);
      break;
    case '}':
      return mktok(CLOSE_CURLY, 1, 0);
      break;
    case '"':
    case '\'':
      _PARSING_STRING = true;
    default: {
      char* tokinit = *streamp - !_PARSING_STRING;
      if (tokchar <= '9' && tokchar >= '0' || tokchar == '.') {// Check if number literal
        bool is_final;
        while (!(is_final = in_str(consume(streamp), " \t\n,")) && tokchar <= '9' && tokchar >= '0');
        if (is_final) {
          *(*streamp - 1) = '\0';
          return mktok(NUM, *streamp - tokinit, tokinit);
        }
        else 
        goto iden_like;
      } else if (_PARSING_STRING) {
        while (!in_str(consume(streamp), "\'\""));
        if (!*streamp) return throwerr("Unmatched quotation");
        *(*streamp - 1) = '\0';
        _PARSING_STRING = false;
        return mktok(STR, *streamp - tokinit, tokinit);
      } else {
      iden_like:
        while (!in_str(tokchar = consume(streamp), " \t\n,\"\'()[]{}"));
        if (in_str(tokchar, "()[]{}\"\'")) { // Sensitive chars, if we were to zero them out right away we would miss tokens
          char* senstream = *streamp - 1;
          _PEAKED_TOKEN = scan(&senstream);
          _IS_PEAKED_TOKEN = true;
        }
        *(*streamp - (tokchar != '\0')) = '\0';
        return mktok(IDEN, *streamp - tokinit - (tokchar != '\0'), tokinit); // For now...
      }
    }
  }
}

Scanned* scanner(char* stream) {
  Scanned* tokvec = new_vector_with_capacity(Scanned, 64);
  char tokchar;

  while (*stream || _IS_PEAKED_TOKEN) {
    Scanned ptok = scan(&stream);
    push(
      tokvec,
      ptok
    );
  }
  
  return tokvec;
}

int main(int argc, char *argv[]) {
  // ----------------------------------------------
  if (argc == 1) {
    printf("Provide at least one expression\n");
    return 0;
  }

  for (int i = 2; i < argc; i++)
    argv[i][-1] = ' ';

  char* exp = argv[1];
  // ----------------------------------------------
  Scanned* expvec = scanner(exp); 
  for_each(expvec)
    printf("%d@(%d, %d --- %d): %s\n", expvec[i].res.tok.type, expvec[i].index, expvec[i].line, expvec[i].res.tok.length, expvec[i].res.tok.tok); 
}
