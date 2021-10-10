#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

HmapRecord *hmapNewRecord(void *key, void *value);
void hmapResize(HashMap *map);
void hmapDestroyRecords(HmapRecord *record);
int hmapTestPrime(size_t n);
size_t hmapNextPrime(size_t n);

HmapRecord *hmapNewRecord(void *key, void *value) {
  HmapRecord *record = malloc(sizeof(HmapRecord));
  record->key = key;
  record->value = value;
  record->next = NULL;
  return record;
}

void hmapResize(HashMap *map) {
  size_t new_size = hmapNextPrime(map->size * 2);
  HmapRecord **new_records = calloc(new_size, sizeof(HmapRecord *));

  HmapRecord *record = NULL, *tmp = NULL;
  size_t new_index;
  for (size_t i = 0; i < map->size; i++) {
    record = map->records[i];
    while (record) {
      new_index = map->hash(record->key) % new_size;
      tmp = record->next;

      record->next = new_records[new_index];
      new_records[new_index] = record;

      record = tmp;
    }
  }
  map->size = new_size;
  free(map->records);
  map->records = new_records;
}

void hmapDestroyRecords(HmapRecord *record) {
  HmapRecord *next = NULL;
  while (record) {
    next = record->next;
    free(record);
    record = next;
  }
}

int hmapTestPrime(size_t n) {
  if (n <= 1)
    return 0;
  if (n <= 3)
    return 1;

  if (n % 2 == 0 || n % 3 == 0)
    return 0;

  for (int i = 5; i * i <= n; i = i + 6)
    if (n % i == 0 || n % (i + 2) == 0)
      return 0;

  return 1;
}

size_t hmapNextPrime(size_t n) {
  while (!hmapTestPrime(n)) {
    n++;
  }
  return n;
}

HashMap hmapCreate(int (*compare)(void *, void *),
                   unsigned long int (*hash)(void *), size_t size) {
  HashMap map;
  map.size = hmapNextPrime(size);
  map.n_records = 0;
  map.compare = compare;
  map.hash = hash;
  map.records = calloc(map.size, sizeof(HmapRecord *));
  return map;
}

void *hmapGet(HashMap *map, void *key) {
  size_t index = map->hash(key) % map->size;
  HmapRecord *record = map->records[index];

  while (record) {
    if (map->compare(record->key, key)) {
      return record->value;
    }
    record = record->next;
  }

  return NULL;
}

void hmapSet(HashMap *map, void *key, void *value) {
  size_t index = map->hash(key) % map->size;
  if (map->records[index] == NULL) {
    // this is first record in this bucket.
    map->records[index] = hmapNewRecord(key, value);
    map->n_records++;
  } else {
    // bucket is present.
    HmapRecord *current = map->records[index];
    HmapRecord *prev = current;
    while (current) {
      if (map->compare(key, current->key)) {
        // key is present in table, update the value.
        current->value = value;
        return;
      }
      prev = current;
      current = current->next;
    }
    prev->next = hmapNewRecord(key, value);
    map->n_records++;
  }
  float load = (float)map->n_records / (float)map->size;
  if (load > 0.75) {
    hmapResize(map);
  }
}

void hmapDestroy(HashMap *map) {
  for (size_t i = 0; i < map->size; i++) {
    hmapDestroyRecords(map->records[i]);
  }
  map->size = 0;
  map->n_records = 0;
  free(map->records);
}
