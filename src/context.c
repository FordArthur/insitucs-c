#include "context.h"
#include "vec.h"

#define TABLE_SIZE 256

static _Bucket table[TABLE_SIZE] = {0};

// from http://www.cse.yorku.ca/~oz/hash.html
static inline __attribute_pure__ unsigned long hash(const Token name) {
  unsigned long hash_number = 5381;
  char c;

  for (unsigned long i = 0; i < name.len; c = ((char*)name.src)[i])
    hash_number = ((hash_number << 5) + hash_number) + c;

  return hash_number;
}

static inline void ordered_remove(_Bucket bucket, Token name) {

}

void insert_context(Token name, Type type) {
  const unsigned long key = hash(name) % TABLE_SIZE;

  if (!table[key]) {
    table[key] = new_vector(_Bucket);
  }

  push(table[key], ((_Entry) { .name = name, .type = type }));
}

void delete_context(Token name) {
  const unsigned long key = hash(name) % TABLE_SIZE;

  if (!table[key]) return;

  ordered_remove(table[key], name);
}

Type lookup_context(Token name) {
  const unsigned long key = hash(name) % TABLE_SIZE;

  if (!table[key]) return 0;

  const vect_h* bucket_h = _get_header(table[key]);

  for (_Entry* bucket = table[key]; bucket < bucket_h->size*bucket_h->obj_size + table[key]; bucket++) {
    if (strncmp(bucket->name.src, name.src, name.len) == 0) return bucket->type;
  }

  return 0;
}
