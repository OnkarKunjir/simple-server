#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

Vector vectorCreate(size_t size) {
  Vector vec;
  vec.length = 0;
  vec.size = size;
  vec.data = calloc(size, sizeof(void *));
  return vec;
}

int vectorAppend(Vector *vec, void *data) {
  if (vec->length >= vec->size) {
    vec->size *= 2;
    void **expanded = realloc(vec->data, sizeof(void *) * vec->size);
    // check if memory was allocated else keep old memory and warn the user.
    if (expanded == NULL)
      return -1;
    vec->data = expanded;
  }
  vec->data[vec->length++] = data;
  return 0;
}

void *vectorPop(Vector *vec) {
  void *data = vec->data[--vec->length];
  if ((((float)vec->length / (float)vec->size) < VECTOR_SHRINK_THRESHOLD) &&
      (vec->length > 0)) {
    vec->size = vec->length;
    vec->data = realloc(vec->data, sizeof(void *) * vec->size);
  }
  return data;
}

void vectorDestroy(Vector *vec) {
  vec->size = 0;
  vec->length = 0;
  free(vec->data);
}
