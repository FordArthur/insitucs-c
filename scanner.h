#ifndef INSITUCS_SCANNER_HEADER
#define INSITUCS_SCANNER_HEADER

typedef enum TokenType {
  OPEN_PAREN, CLOSE_PAREN,     //
  OPEN_BRACKET, CLOSE_BRACKET, // Basic syntax structuring
  OPEN_CURLY, CLOSE_CURLY,     //

  STR, NUM, IDEN,              // Literals 

  // !! IDENTIFIER MUST BE BEFORE FUNCTION_DEF !!

  FUNCTION_DEF,                //
  IF, MATCH,                   // Keywords
  FOR,                         //

  EOF                          // End Of File
} TokenType;
// note to self: EOF is useful if multithreading

typedef struct Scanned {
  bool is_ok;
  unsigned int line;   // 
  unsigned int index;  // Token/Err position in source file
  union {
    struct {
      TokenType type;      // Token type

      unsigned int length; // Token length

      char* tok;           // Actual token (Don't use if obvious, i.e. predefined sequence of characters)
    } tok;
    char* err;             // Error message
  } res;
} Scanned;

Scanned* scanner(char* stream);

#endif // !INSITUCS_SCANNER_HEADER
