#pragma once

#include <stdlib.h>

typedef struct HmapRecord {
  void *key;
  void *value;
  struct HmapRecord *next;
} HmapRecord;

typedef struct HashMap {
  size_t size;
  size_t n_records;
  HmapRecord **records;
  int (*compare)(void *, void *);
  unsigned long int (*hash)(void *);
} HashMap;

HashMap hmapCreate(int (*compare)(void *, void *),
                   unsigned long int (*hash)(void *), size_t size);

void *hmapGet(HashMap *map, void *key);

void hmapSet(HashMap *map, void *key, void *value);

void hmapDestroy(HashMap *map);
