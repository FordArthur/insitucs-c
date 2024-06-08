#include "scanner.h"
#include "vec.h"
#define nsyms 4

#define mktok(t, l, token) (Scanned) { .is_ok = true, .line = line, .index = index, .res.tok.type = t, .res.tok.length = l, .res.tok.tok = token }
#define throwerr(e) (Scanned) { .is_ok = false, .line = line, .index = index, .res.err = e }

typedef struct BytePair {
  unsigned char left;
  unsigned char right;
} bytepair;

static unsigned int _LINE = 0;  //
static unsigned int _INDEX = 0; // Will keep track of the position as we are parsing

static bool _IS_PEAKED_TOKEN = false;
static Scanned _PEAKED_TOKEN;

static bool _HAS_ERRORS = false;
static bool _IS_IN_ERROR = false;

static const char symtablelike[nsyms][9] = {"function", "if", "match", "for"};

// This really should be implemented as a hashtable/trie but whatever
static inline TokenType selectfromidents(const char* potsym) {
  int i = nsyms - 1;
  for (; i >= 0 && strcmp(potsym, symtablelike[i]); i--);
  return FUNCTION_DEF + i;
}

static inline bool in_str(const char c, const char* s) {
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

static inline bytepair until(char** streamp, const char* delims) {
  char consumed = consume(streamp);
  bool numdot = false;
  bool _IS_NUMBER = true;

  for (; consumed && !in_str(consumed, delims); consumed = consume(streamp))
    if (_IS_NUMBER && !isdigit(consumed) || numdot && !(numdot = true))
      _IS_NUMBER = false;

  return (bytepair) {
    .left = consumed,
    .right = _IS_NUMBER
  };
}

Scanned scan(char** streamp) {
  bool _IS_PARSING_STRING = false;
  if (_IS_PEAKED_TOKEN) {
    _IS_PEAKED_TOKEN = false;
    return _PEAKED_TOKEN;
  }
  char tokchar;
  unsigned int line = _LINE;
  unsigned int index = _INDEX;

  switch ((tokchar = consume(streamp))) {
    case ' ':
    case '\t':
    case '\n':
      return scan(streamp);
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
    case '@':
      return mktok(AT, 1, 0);
      break;
    case '#':
      return mktok(HASHTAG, 1, 0);
      break;
    case '%':
      return mktok(PERCENT, 1, 0);
      break;
    case '"':
      _IS_PARSING_STRING = true;
    default: {
      char* tokinit = *streamp - !_IS_PARSING_STRING;
      _INDEX--; 
      char* tokcont = tokinit;

      bytepair parsed = until(&tokcont, " ,\t\n()[]{}\"");
      tokchar = parsed.left;

      if (_IS_PARSING_STRING && tokchar != '"') {
        _HAS_ERRORS = _IS_IN_ERROR = true;
        return throwerr("Unmatched quotation mark");
      }

      if (!_IS_PARSING_STRING && tokchar && in_str(tokchar, "()[]{}\"")) { // Sensitive chars, if we were to zero them out right away we would miss tokens
        char* senstream = tokcont - 1;
        _INDEX--;

        _PEAKED_TOKEN = scan(&senstream);
        _IS_PEAKED_TOKEN = true;

        tokcont[-(tokchar != '\0')] = '\0';
        *streamp = senstream;
      } else {
        tokcont[-(tokchar != '\0')] = '\0';
        *streamp = tokcont;
      }

      return mktok(
        _IS_PARSING_STRING?
          STR:
          parsed.right?
            NUM:
            selectfromidents(tokinit),
        tokcont - tokinit,
        tokinit
      );
    }
  }
}

Tokens scanner(char* stream) {
  Scanned* tokvec = new_vector_with_capacity(Scanned, 64);
  Scanned** errs = new_vector_with_capacity(Scanned*, 8);

  while (*stream || _IS_PEAKED_TOKEN) {
    push(tokvec, scan(&stream));
    if (_IS_IN_ERROR) {
      _IS_IN_ERROR = false;
      push(errs, tokvec + sizeof_vector(tokvec) - 1);
    }
  }
  
  if (_HAS_ERRORS) {
    return (Tokens) {
      .is_correct_stream = false,
      .errs = errs,
    };
  } else {
    unsigned int line = _LINE, index = _INDEX;
    push(tokvec, mktok(EOF, 1, 0));

    return (Tokens) {
      .is_correct_stream = true,
      .stream = tokvec,
    };
  }
}
