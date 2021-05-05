#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

// Figure out where "key" belongs in "entries" (reuses tombstone slots)
static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;

  // Loop for linear probing
  for (;;) {
    Entry *entry = &entries[index];

    if (entry->key == NULL) {
      // Check if it is an empty entry
      if (IS_NIL(entry->value))
        return tombstone != NULL ? tombstone : entry;
      // Check if the entry is a tombstone
      else if (tombstone == NULL)
        tombstone = entry;
    } else if (entry->key == key)
      return entry;

    index = (index + 1) % capacity;
  }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  // Check if table is empty
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

// Adjust the allocated size of "table" to "capacity"
static void adjustCapacity(Table *table, int capacity) {
  // Allocate and initialise array of entries
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  // Copy over every single non-empty bucket
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    // Discard all of the tombstones
    if (entry->key == NULL)
      continue;

    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table *table, ObjString *key, Value value) {
  // Check if there is space in "table->entries"
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  // Empty place for "key" and "value" to be stored in
  Entry *entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = entry->key == NULL;
  if (isNewKey && IS_NIL(entry->value))
    table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableDelete(Table *table, ObjString *key) {
  // Check if table is empty
  if (table->count == 0)
    return false;

  // Find entry
  Entry *entry = findEntry(table->entries, table->capacity, key);
  // Check if the entry itself is a tombstone (deleted)
  if (entry->key == NULL)
    return false;

  // Place tombstone where value goes
  entry->key = NULL;
  entry->value = BOOL_VAL(true);

  return true;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL)
      tableSet(to, entry->key, entry->value);
  }
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash % table->capacity;

  for (;;) {
    Entry *entry = &table->entries[index];

    if (entry->key == NULL) {
      // Stop if we find a non-tombstone entry
      if (IS_NIL(entry->value))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key->chars, chars, length) == 0)
      // Found it 😎
      return entry->key;

    index = (index + 1) % table->capacity;
  }
}