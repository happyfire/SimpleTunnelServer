/*
 * Original Author:  Oliver Lorenz (ol), olli@olorenz.org, https://olorenz.org
 * License:  This is licensed under the same terms as uthash itself
 */

#ifndef _CACHE_
#define _CACHE_

#include "uthash.h"
#include <ev.h>


/**
 * A cache entry
 */
struct cache_entry {
    char *key;         /**<The key */
    void *data;        /**<Payload */
    ev_tstamp ts;    /**<Timestamp */
    UT_hash_handle hh; /**<Hash Handle for uthash */
};

/**
 * A cache object
 */
struct cache {
    size_t max_entries;              /**<Amount of entries this cache object can hold */
    struct cache_entry *entries;     /**<Head pointer for uthash */
    void (*free_cb) (void *key, void *element); /**<Callback function to free cache entries */
};

int cache_create(struct cache **dst, const size_t capacity,
                        void (*free_cb)(void *key, void *element));
int cache_delete(struct cache *cache, int keep_data);
int cache_clear(struct cache *cache, ev_tstamp age);
int cache_lookup(struct cache *cache, char *key, size_t key_len, void *result);
int cache_insert(struct cache *cache, char *key, size_t key_len, void *data);
int cache_remove(struct cache *cache, char *key, size_t key_len);
int cache_key_exist(struct cache *cache, char *key, size_t key_len);

#endif
