#ifndef CGNS_HASHMAP_H
#define CGNS_HASHMAP_H
#include <stdint.h>
#include <limits.h>
/* This is a simple hashmap inspired by cpython dict
 * It maps a char[33] name to its array index.
 * It is not memory efficient since the (hash, name, index) is stored
 * and is a duplicate of existing data.
 * The indexing struct is compact to be cache friendly.
 * One of the difficulty is to keep the hash table indices in sync
 * with the cgns_zone list when there is deletion.
 */

typedef char char_name[33];
typedef intptr_t map_ssize_t;
typedef uintptr_t map_usize_t;
#if UINTPTR_MAX == 0xffffffffffffffff
/* it's 64bits pointers */
#define SIZEOF_USIZE_T 8
#define SIZEOF_VOID_P 8
#elif UINTPTR_MAX == 0xffffffff
/* it's 32bits pointers */
#define SIZEOF_USIZE_T 4
#define SIZEOF_VOID_P 4
#endif

#if ULONG_MAX == 0xffffffffUL
#define SIZEOF_LONG 4
#elif ULONG_MAX == 0xffffffffffffffffL
#define SIZEOF_LONG 8
#endif

#define MAP_MINSIZE 8

typedef struct {
    /* Cached hash code of me_key. */
    map_ssize_t me_hash; // signed integer same size as size_t
    char_name me_key;
    map_ssize_t me_value; //index of key in mapped vector
} cgns_hashmap_entry;

#define MAPIX_EMPTY (-1)
#define MAPIX_DUMMY (-2)  /* Used internally */
#define MAPIX_ERROR (-3)

struct _hashmapobject {

    /* Size of the hash table (map_indices). It must be a power of 2. */
    map_ssize_t table_size; 

    /* Function to lookup in the hash table (dk_indices):

       - lookdict_unicode(): specialized to Unicode string keys, comparison of
         which can never raise an exception; that function can never return
         DKIX_ERROR.

       - lookdict_unicode_nodummy(): similar to lookdict_unicode() but further
         specialized for Unicode string keys that cannot be the <dummy> value.*/

    /* Number of usable entries in map_entries. */
    map_ssize_t map_usable;

    /* Number of used entries in map_entries. */
    map_ssize_t map_nentries;

    /* Actual hash table of map_size entries. It holds indices in map_entries,
       or MAPIX_EMPTY(-1) or MAPIX_DUMMY(-2).

       Indices must be: 0 <= indice < USABLE_FRACTION(map_size).

       The size in bytes of an indice depends on map_size:

       - 1 byte if map_size <= 0xff (char*)
       - 2 bytes if map_size <= 0xffff (int16_t*)
       - 4 bytes if map_size <= 0xffffffff (int32_t*)
       - 8 bytes otherwise (int64_t*)

       Dynamically sized, SIZEOF_VOID_P is minimum. */
    char map_indices[];  /* char is required to avoid strict aliasing. */

    /* "HashMapKeyEntry map_entries[dk_usable];" array follows:
       see the MAP_ENTRIES() macro */
};

typedef struct _hashmapobject cgns_hashmap_keyobject;

typedef struct {

    /* Number of items in the hashmap */
    map_ssize_t ma_used;

    /* Following python implementation key and values are stored
	 in a combined continuous struct to be cache friendly.
	 Further integration with CGNS struct could allow separation between
	 entry indexing and entry values to be more memory efficient.
     The hash would be a new property of the cgns_zone node. */
    cgns_hashmap_keyobject *ma_keys;

    /* The table is "combined": keys and values
       are stored in ma_keys.
     */
} cgns_hashmap_object;

cgns_hashmap_object* cgi_new_hashmap(void);
cgns_hashmap_object* cgi_new_presized_hashmap(map_ssize_t minused);
void cgi_hashmap_clear(cgns_hashmap_object* op);

map_ssize_t cgi_map_get_item(cgns_hashmap_object* op, char* key);
int cgi_map_set_item(cgns_hashmap_object* op, char* key, map_ssize_t value);
int cgi_map_contains(cgns_hashmap_object* op, char* key);
int cgi_map_del_shift_item(cgns_hashmap_object* op, char* key);

#endif CGNS_HASHMAP_H