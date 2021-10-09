#pragma once

#include <stdlib.h>

// shrink the vector if length / size drops below this.
#define VECTOR_SHRINK_THRESHOLD 0.75

typedef struct Vector {
  size_t size;
  size_t length;
  void **data;
} Vector;

Vector vectorCreate(size_t size);

/**
 * Function to append new element in vector.
 * @return 0 on successful insertion else -1.
 */
int vectorAppend(Vector *vec, void *data);

void *vectorPop(Vector *vec);

void vectorDestroy(Vector *vec);
