#include "sized_array.h"
#include <stdlib.h>

s_array* _new_s_array(unsigned long obj_size, unsigned long size) {
  s_array* sarr = malloc(obj_size*size);
  sarr->size = size;
  return sarr;
}

s_array* _from_array(void* array, unsigned long obj_size, unsigned long size) {
  s_array* sarr = _new_s_array(obj_size, size);
  sarr->array = array;
  return sarr;
}
