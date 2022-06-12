#include <utils/hashmap.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.hpp>
#include <memory/heap.hpp>

/* Private: Short and sweet hash function - "key mod capacity". The key type
 * is restricted to int right now.
 */
static MapBucketList* find_bucket(HashMap *map, int key) {
    MapBucketList* bucket = NULL;
    if(map->buckets != NULL) {
        bucket = &map->buckets[key % map->bucket_count];
    }
    return bucket;
}

void HashMap::emhashmap_deinitialize() {
    if(this->entries != NULL) {
        heapFree(this->entries);
        this->entries = NULL;
    }

    if(this->buckets != NULL) {
        heapFree(this->buckets);
        this->buckets = NULL;
    }
}

bool HashMap::emhashmap_initialize(int capacity, float load_factor) {
    this->bucket_count = ((int)(capacity / load_factor) + 1);
    this->capacity = capacity;
    this->entries = (MapEntry*) heapAllocate(sizeof(MapEntry) * this->capacity);
    memset(this->entries, 0, sizeof(MapEntry) * this->capacity);
    this->buckets = (MapBucketList*) heapAllocate(sizeof(MapBucketList) *
            this->bucket_count);
    memset(this->buckets, 0, sizeof(MapBucketList) * this->bucket_count);

    int i;
    for(i = 0; i < this->bucket_count; i++) {
        LIST_INIT(&this->buckets[i]);
    }

    LIST_INIT(&this->free_list);
    for(i = 0; i < this->capacity; i++) {
        LIST_INSERT_HEAD(&this->free_list, &this->entries[i], entries);
    }
    return this->buckets != NULL;
}

MapEntry* HashMap::emhashmap_get(int key) {
    MapBucketList* bucket = find_bucket(this, key);

    MapEntry* entry;
    LIST_FOREACH(entry, bucket, entries) {
        if(entry->key == key) {
            return entry;
        }
    }
    return NULL;
}

bool HashMap::emhashmap_contains(int key) {
    return emhashmap_get(key) != NULL;
}

bool HashMap::emhashmap_put(int key, void* value) {
    MapBucketList* bucket = find_bucket(this, key);

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
        MapEntry* new_entry = LIST_FIRST(&this->free_list);
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

void* HashMap::emhashmap_remove(int key) {
    MapBucketList* bucket = find_bucket(this, key);

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

int HashMap::emhashmap_size() {
    int size = 0;
    int i;
    for(i = 0; i < this->bucket_count; i++) {
        MapEntry* entry = NULL;
        LIST_FOREACH(entry, &this->buckets[i], entries) {
            ++size;
        }
    }
    return size;
}

bool HashMap::emhashmap_is_empty() {
    return emhashmap_size() == 0;
}

float HashMap::emhashmap_load_factor() {
    return emhashmap_size() / this->capacity;
}

MapIterator HashMap::emhashmap_iterator() {
    MapIterator iterator;
    iterator.current_bucket = 0;
    iterator.current_entry = NULL;
    iterator.map = this;
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