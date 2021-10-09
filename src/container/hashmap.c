#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Function returns new hash map entry with next pointer set to NULL.
 */
HashEntry *hmapNewEntry(void *key, void *value);

/**
 * Function deallocates memory allocated to all entries inside bucket.
 */
void hmapDestroyBucket(HashEntry *entry);

/**
 * Function inserts a HashEntry insde a bucket. If key is present inside bucket
 * it's value is updated.
 * @param bucket Specify bucket in which entry is to be inserted.
 * @param compare Specify function to compare two keys.
 * @param key Specify key.
 * @param value Specify value for key.
 * @return function return 1 if new entry was created and return 0 in case if
 * entry value was updated.
 */
int hmapInsertEntry(HashEntry **bucket, mapCompare compare, void *key,
                    void *value);

/**
 * Function provides same functionality as hmapInsertEntry it just inserts
 * already created entry to bucket. used in rehashing.
 * NOTE: function assigns the next pointer of oldentry to NULL.
 */
void hmapInsertOldEntry(HashEntry **bucket, HashEntry *oldentry);

/**
 * Check if number is prime or not.
 */
int hmapIsPrime(unsigned int n);

/**
 * Function return next smallest prime number greater than base.
 */
unsigned int hmapNextPrime(unsigned int base);

/**
 * Function to resize the map and rehash all the values into new buckets.
 * this also deallocates old buckets.
 */
void hmapResize(HashMap *map);

HashEntry *hmapNewEntry(void *key, void *value) {
  HashEntry *entry = malloc(sizeof(HashEntry));
  entry->key = key;
  entry->value = value;
  entry->next = NULL;
  return entry;
}

void hmapDestroyBucket(HashEntry *entry) {
  HashEntry *next = NULL;
  while (entry) {
    next = entry->next;
    free(entry);
    entry = next;
  }
}

int hmapInsertEntry(HashEntry **bucket, mapCompare compare, void *key,
                    void *value) {
  if (!*bucket) {
    // bucket is empty.
    *bucket = hmapNewEntry(key, value);
    return 1;
  } else {
    HashEntry *entry = *bucket;
    HashEntry *prev = NULL;
    while (entry) {
      if (compare(key, entry->key)) {
        entry->value = value;
        return 0;
      }
      prev = entry;
      entry = entry->next;
    }
    prev->next = hmapNewEntry(key, value);
    return 1;
  }
}

void hmapInsertOldEntry(HashEntry **bucket, HashEntry *oldentry) {
  oldentry->next = NULL;
  if (!*bucket) {
    *bucket = oldentry;
  } else {
    HashEntry *entry = *bucket;
    while (entry->next) {
      entry = entry->next;
    }
    entry->next = oldentry;
  }
}

int hmapIsPrime(unsigned int n) {

  if (n <= 3)
    return 1;

  if (((n % 2) == 0) || ((n % 3) == 0))
    return 0;

  for (unsigned int i = 5; (i * i) <= n; i += 6) {
    if (((n % i) == 0) || ((n % (i + 2)) == 0))
      return 0;
  }

  return 1;
}

unsigned int hmapNextPrime(unsigned int base) {
  while (!hmapIsPrime(base)) {
    base++;
  }
  return base;
}

void hmapResize(HashMap *map) {

  unsigned int size = hmapNextPrime(map->size * 2);
  HashEntry **buckets = (HashEntry **)calloc(size, sizeof(HashEntry **));

  // rehash all the entries into new buckets.
  HashEntry *entry, *next;
  for (unsigned int i = 0; i < map->size; i++) {
    entry = map->buckets[i];
    while (entry) {
      next = entry->next;
      hmapInsertOldEntry(&buckets[map->hash(entry->key) % size], entry);
      entry = next;
    }
  }
  // delete old bucket containers.
  free(map->buckets);

  map->size = size;
  map->buckets = buckets;
}

/// functions from header files are below.

HashMap hmapCreate(mapCompare compare, mapHash hash, unsigned int size) {
  HashMap map;
  map.size = hmapNextPrime(size);
  map.buckets = (HashEntry **)calloc(size, sizeof(HashEntry **));
  map.compare = compare;
  map.hash = hash;
  return map;
}

void hmapSet(HashMap *map, void *key, void *value) {
  unsigned int index = map->hash(key) % map->size;
  map->entries +=
      hmapInsertEntry(&map->buckets[index], map->compare, key, value);

  float load = (float)map->entries / (float)map->size;
  if (load >= HMAP_MAX_LOAD_FACTOR) {
    hmapResize(map);
  }
}

void *hmapGet(HashMap *map, void *key) {
  unsigned int index = map->hash(key) % map->size;
  HashEntry *entry = map->buckets[index];

  while (entry) {
    if (map->compare(key, entry->key)) {
      return entry->value;
    }
    entry = entry->next;
  }
  fprintf(stderr, "[ERROR] : key is not present in table\n");
  return entry;
}

void hmapDestroy(HashMap *map) {
  for (unsigned int i = 0; i < map->size; i++) {
    hmapDestroyBucket(map->buckets[i]);
  }
  free(map->buckets);
}
