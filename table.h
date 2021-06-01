#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

// Key-Value pair for "Table" struct
typedef struct {
  ObjString *key;
  Value value;
} Entry;

/* Hash Table
   - "count" is the current number of entries in the table (including
   tombstones)
   - "capacity" is the allocated size of the table
   - "entries" is the actual array of key and value pairs
 */
typedef struct {
  int count;
  int capacity;
  Entry *entries;
} Table;

// Initialise table fields with 0/NIL
void initTable(Table *table);

// Free any memory allocations within "table" and re-init
void freeTable(Table *table);

// Get value of "key" in "table" and assign to "value"
// if an entry already exists then it returns true, otherwise, it returns false
bool tableGet(Table *table, ObjString *key, Value *value);

// Add "key" "value" pair into "table" and return true if a new entry was added
bool tableSet(Table *table, ObjString *key, Value value);

// Replace value of "key" in "table" with tombstone value
bool tableDelete(Table *table, ObjString *key);

// Helper to copy over all values "from" one table "to" another
void tableAddAll(Table *from, Table *to);

// Find interned string "chars" in "table"
ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash);

// Delete unreachable values from the table
void tableRemoveWhite(Table *table);

// Mark values in table
void markTable(Table *table);

#endif
