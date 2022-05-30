#if 0
#include <utils/hashmap.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.hpp>
#include <memory/heap.hpp>

/* Private: Short and sweet hash function - "key mod capacity". The key type
 * is restricted to int right now.
 */
static MapBucketList* find_bucket(HashMap* map, int key) {
    MapBucketList* bucket = NULL;
    if(map != NULL && map->buckets != NULL) {
        bucket = &map->buckets[key % map->bucket_count];
    }
    return bucket;
}

void emhashmap_deinitialize(HashMap* map) {
    if(map->entries != NULL) {
        heapFree(map->entries);
        map->entries = NULL;
    }

    if(map->buckets != NULL) {
        heapFree(map->buckets);
        map->buckets = NULL;
    }
}

bool emhashmap_initialize(HashMap* map, int capacity, float load_factor) {
    map->bucket_count = ((int)(capacity / load_factor) + 1);
    map->capacity = capacity;
    map->entries = (MapEntry*) heapAllocate(sizeof(MapEntry) * map->capacity);
    memset(map->entries, 0, sizeof(MapEntry) * map->capacity);
    map->buckets = (MapBucketList*) heapAllocate(sizeof(MapBucketList) *
            map->bucket_count);
    memset(map->buckets, 0, sizeof(MapBucketList) * map->bucket_count);

    int i;
    for(i = 0; i < map->bucket_count; i++) {
        LIST_INIT(&map->buckets[i]);
    }

    LIST_INIT(&map->free_list);
    for(i = 0; i < map->capacity; i++) {
        LIST_INSERT_HEAD(&map->free_list, &map->entries[i], entries);
    }
    return map->buckets != NULL;
}

MapEntry* emhashmap_get(HashMap* map, int key) {
    MapBucketList* bucket = find_bucket(map, key);

    MapEntry* entry;
    LIST_FOREACH(entry, bucket, entries) {
        if(entry->key == key) {
            return entry;
        }
    }
    return NULL;
}

bool emhashmap_contains(HashMap* map, int key) {
    return emhashmap_get(map, key) != NULL;
}

bool emhashmap_put(HashMap* map, int key, void* value) {
    MapBucketList* bucket = find_bucket(map, key);

    MapEntry* entry, *matching_entry = NULL;
    LIST_FOREACH(entry, bucket, entries) {
        if(entry->key == key) {
            matching_entry = entry;
        }
    }

    bool result = true;
    if(matching_entry != NULL) {
        matching_entry->value = value;
    } else {
        MapEntry* new_entry = LIST_FIRST(&map->free_list);
        if(new_entry == NULL) {
            result = false;
        } else {
            new_entry->key = key;
            new_entry->value = value;
            LIST_REMOVE(new_entry, entries);
            LIST_INSERT_HEAD(bucket, new_entry, entries);
        }
    }
    return result;
}

void* emhashmap_remove(HashMap* map, int key) {
    MapBucketList* bucket = find_bucket(map, key);

    MapEntry* entry, *matching_entry = NULL;
    LIST_FOREACH(entry, bucket, entries) {
        if(entry->key == key) {
            matching_entry = entry;
        }
    }

    void* value = NULL;
    if(matching_entry != NULL) {
        value = matching_entry->value;
        LIST_REMOVE(matching_entry, entries);
    }
    return value;
}

int emhashmap_size(HashMap* map) {
    int size = 0;
    int i;
    for(i = 0; i < map->bucket_count; i++) {
        MapEntry* entry = NULL;
        LIST_FOREACH(entry, &map->buckets[i], entries) {
            ++size;
        }
    }
    return size;
}

bool emhashmap_is_empty(HashMap* map) {
    return emhashmap_size(map) == 0;
}

float emhashmap_load_factor(HashMap* map) {
    return emhashmap_size(map) / map->capacity;
}

MapIterator emhashmap_iterator(HashMap* map) {
    MapIterator iterator;
    iterator.current_bucket = 0;
    iterator.current_entry = NULL;
    iterator.map = map;
    return iterator;
}

MapEntry* emhashmap_iterator_next(MapIterator* iterator) {
    if(iterator != NULL) {
        if(iterator->current_entry != NULL) {
            iterator->current_entry = LIST_NEXT(iterator->current_entry, entries);
        }

        if(iterator->current_entry == NULL) {
            do {
                iterator->current_entry = LIST_FIRST(&iterator->map->buckets[iterator->current_bucket++]);
            } while(iterator->current_entry == NULL &&
                    iterator->current_bucket < iterator->map->bucket_count - 1);
        }
    }
    return iterator->current_entry;
}
#endif