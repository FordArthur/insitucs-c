#ifndef INSITUCS_SIZED_ARRAY_HEADER
#define INSITUCS_SIZED_ARRAY_HEADER

#include "vec.h"
typedef struct Sized_Array {
  void* array;
  unsigned long size;
} s_array;

/*
 * Test
 */
#define new_sized_array(T, s) _new_s_array(sizeof(T), s)
#define from_array(T, a, s) _from_array(a, sizeof(T), s)

s_array* _from_array(void* array, unsigned long obj_size, unsigned long size);

#endif // !INSITUCS_SIZED_ARRAY_HEADER
