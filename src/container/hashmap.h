#pragma once

#define HMAP_DEFAULT_SIZE 7
#define HMAP_MAX_LOAD_FACTOR 0.75

typedef int (*mapCompare)(void *, void *);
typedef unsigned int (*mapHash)(void *);

typedef struct HashEntry {
  void *key;
  void *value;
  struct HashEntry *next;
} HashEntry;

typedef struct HashMap {
  unsigned int size;    // number of rows in table.
  unsigned int entries; // number of entries in the table.
  HashEntry **buckets;  // pointer to table in memory.
  mapCompare compare;   // function to compare keys.
  mapHash hash;         // function to hash the key.
} HashMap;

/**
 * Creates new hashmap.
 * @param compare Specify a function to compare two keys, function should return
 * 1 if both keys are equal else 0.
 * @param hash Specify function which generates hash value form key.
 * @param size Specify rough estimate of maximum number of elements to be
 * inserted into the map, should be > 0.
 * NOTE: size parameter is just to avoid any resizing and rehashing.
 */
HashMap hmapCreate(mapCompare compare, mapHash hash, unsigned int size);

/**
 * set the value of key in hash table. If key is already present in table it's
 * value is updated.
 * @param map Specify map into which value is to be inserted.
 * @param key Specify key of the entry.
 * @param value Specify value of key.
 */
void hmapSet(HashMap *map, void *key, void *value);

/**
 * Fetch the value of key form hash table.
 */
void *hmapGet(HashMap *map, void *key);

/**
 * Dallocate any resources allocated to map
 * call this function after done using map to avoid any memory leakes.
 */
void hmapDestroy(HashMap *map);
