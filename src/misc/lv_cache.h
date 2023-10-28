/**
 * @file lv_cache.h
 *
 */

#ifndef LV_CACHE_H
#define LV_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../osal/lv_os.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_cache_entry_t {
    /**The image source or other source related to the cache content.*/
    const void * src;

    /**Some data to describe the cache entry*/
    const void * info;

    size_t info_size;

    /**
     * Called to compare cache entry with an info.
     * Before calling this function LVGL checks that `info_size` of both entries are the same.
     * @param e     the cache entry to compare
     * @param info  the info to compare
     * @return      true: `info` is related to `e`; false: they are unrelated
     */
    bool (*compare_cb)(struct _lv_cache_entry_t * e, const void * info);

    /**
     * Called when the entry is invalidated to free info and src
     * @param e     the cache entry to free
     */
    void (*invalidate_cb)(struct _lv_cache_entry_t * e);

    /** User processing tag*/
    uint32_t process_state;

    /** The data to cache*/
    const void * data;

    /** Size of data in bytes*/
    uint32_t data_size;

    /** On access to any cache entry, `life` of each cache entry will be incremented by their own `weight` to keep the entry alive longer*/
    uint32_t weight;

    /** The current `life`. Entries with the smallest life will be purged from the cache if a new entry needs to be cached*/
    int32_t life;

    /** Count how many times the cached data is being used.
     * It will be incremented in lv_cache_get_data and decremented in lv_cache_release.
     * A data will dropped from the cache only if its usage_count is zero */
    uint32_t usage_count;

    /** The cache entry was larger then the max cache size so only a temporary entry was allocated
     * The entry will be closed and freed in `lv_cache_release` automatically*/
    uint32_t temporary  : 1;

    /**Any user data if needed*/
    void * user_data;
} lv_cache_entry_t;


/**
 * Add a new entry to the cache with the given size.
 * It won't allocate any buffers just free enough space to be a new entry
 * with `size` bytes fits.
 * @param size      the size of the new entry in bytes
 * @return          a handler for the new cache entry
 */
typedef lv_cache_entry_t * (*lv_cache_add_cb)(size_t size);

/**
 * Find a cache entry based on its info
 * @param info      the info to find
 * @param info_size size of info
 * @return          the cache entry with given info or NULL if not found
 */
typedef lv_cache_entry_t * (*lv_cache_find_cb)(const void * info, size_t info_size);

/**
 * Invalidate (drop) a cache entry
 * @param entry    the entry to invalidate. (can be retrieved by `lv_cache_find()`)
 */
typedef void (*lv_cache_invalidate_cb)(lv_cache_entry_t * entry);

/**
 * Get the data of a cache entry.
 * It is considered a cached data access so the cache manager can count that
 * this entry was used on more times, and therefore it's more relevant.
 * It also increments entry->usage_count to indicate that the data is being used
 * and cannot be dropped.
 * @param entry     the cache entry whose data should be retrieved
 */
typedef const void * (*lv_cache_get_data_cb)(lv_cache_entry_t * entry);

/**
 * Mark the cache entry as unused. It decrements entry->usage_count.
 * @param entry     the cache entry to invalidate
 */
typedef void (*lv_cache_release_cb)(lv_cache_entry_t * entry);

/**
 * Set maximum cache size in bytes.
 * @param size      the max size in byes
 */
typedef void (*lv_cache_set_max_size_cb)(size_t size);

/**
 * Empty the cache.
 */
typedef void (*lv_cache_empty_cb)(void);

typedef struct {
    lv_cache_add_cb add_cb;
    lv_cache_find_cb find_cb;
    lv_cache_invalidate_cb invalidate_cb;
    lv_cache_get_data_cb get_data_cb;
    lv_cache_release_cb release_cb;
    lv_cache_set_max_size_cb set_max_size_cb;
    lv_cache_empty_cb empty_cb;

    lv_mutex_t mutex;
    size_t max_size;
    uint32_t locked     : 1;    /**< Show the mutex state, used to log unlocked cache access*/
} lv_cache_manager_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the cache module
 */
void _lv_cache_init(void);

/**
 * Set new cache manager
 * @param manager   the new cache manager with callback functions set
 */
void lv_cache_set_manager(lv_cache_manager_t * manager);

/**
 * Add a new entry to the cache with the given size.
 * It won't allocate any buffers just free enough space to be a new entry
 * with `size` bytes fits.
 * @param size      the size of the new entry in bytes
 * @return          a handler for the new cache entry
 */
lv_cache_entry_t * lv_cache_add(size_t size);


/**
 * Find a cache entry based on its info
 * @param info      the info to find
 * @param info_size size of info
 * @return          the cache entry with given info or NULL if not found
 */
lv_cache_entry_t * lv_cache_find(const void * info, size_t info_size);

/**
 * Invalidate (drop) a cache entry. It will call the entry's `invalidate_cb` to free the resources
 * @param entry    the entry to invalidate. (can be retrieved by `lv_cache_find()`)
 */
void lv_cache_invalidate(lv_cache_entry_t * entry);

/**
 * Get the data of a cache entry.
 * It is considered a cached data access so the cache manager can count that
 * this entry was used on more times, and therefore it's more relevant.
 * It also increments entry->usage_count to indicate that the data is being used
 * and cannot be dropped.
 * @param entry     the cache entry whose data should be retrieved
 */
const void * lv_cache_get_data(lv_cache_entry_t * entry);

/**
 * Mark the cache entry as unused. It decrements entry->usage_count.
 * @param entry
 */
void lv_cache_release(lv_cache_entry_t * entry);

/**
 * Set maximum cache size in bytes.
 * @param size      the max size in byes
 */
void lv_cache_set_max_size(size_t size);

/**
 * Get the max size of the cache
 * @return      the max size in bytes
 */
size_t lv_cache_get_max_size(void);

/**
 * Lock the mutex of the cache.
 * Needs to be called manually before any cache operation,
 */
void lv_cache_lock(void);

/**
 * Unlock the mutex of the cache.
 * Needs to be called manually after any cache operation,
 */
void lv_cache_unlock(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_CACHE_H*/
