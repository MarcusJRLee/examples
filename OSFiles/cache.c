#include "filesys/cache.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Project 3: cache.c */

/* Main cache for the filesystem */
struct cache * buffer_cache;


/* Makes new cache and assigns it to buffer_cache */
void
cache_init (void) 
{
  buffer_cache = malloc (sizeof(struct cache));
  buffer_cache->cache_sectors = malloc (sizeof(struct cache_sectors));
  lock_init (&buffer_cache->cache_lock);

  // initialize metadata structs
  int i;
  for (i = 0; i < CACHE_SECTOR_COUNT; i++) {
    metadata_init (&buffer_cache->metadata[i]);
  }
}

/* Initialization function for cache_init to initialize its metadata structs */
void
metadata_init (struct metadata *md) {
  md->populated = false;
  lock_init (&md->sect_lock);
  md->clock_count = 1;
  md->dirty = false;
  md->loading = false;
  md->evicting = false;
}

/* Returns tne index in the buffer_cache of the desired sector_num if one exists.
  Returns -1 otherwise. */
int
get_index (block_sector_t sector_num) {
  int i;
  for (i = 0; i < CACHE_SECTOR_COUNT; i++) {
    if (meta_with_index (i)->sector_num == sector_num && meta_with_index (i)->populated) {
      return i;
    }
  }
  return -1; // Returns -1 if no such sector exists in the cache
}

/* Returns the cache_sect * with the index correspoding to cache_index in buffer_cache */
struct cache_sect *
sect_with_index (int cache_index) {
  return (struct cache_sect *) (&buffer_cache->cache_sectors->sector_array[cache_index]);
}

/* Returns the metadata * with the index correspoding to cache_index in buffer_cache */
struct metadata *
meta_with_index (int cache_index) {
  return (struct metadata *) (&buffer_cache->metadata[cache_index]);
}

/* Returns the cache_sect * corresponding to the sector_num if it exists in the cache.
  Returns NULL otherwise. */
struct cache_sect *
cache_get (block_sector_t sector_num)
{
  int i = get_index (sector_num);
  if (i != -1 && meta_with_index (i)->populated) {
    return sect_with_index (i);
  } else {
    return NULL;
  }
}

/* Returns the metadata * of the desired sector_num if it exists in the cache.
  Returns NULL otherwise. */
struct metadata *
meta_get (block_sector_t sector_num) {
  int i = get_index (sector_num);
  if (i != -1 && meta_with_index (i)->populated) {
    return meta_with_index (i);
  } else {
    return NULL;
  }
}

/* Returns an empty index in the cache if one exists. Returns -1 otherwise. */  
int
cache_empty_index (void) {
  int i;
  for (i = 0; i < CACHE_SECTOR_COUNT; i++) {
    if (meta_with_index (i)->populated == false) {
      return i;
    }
  }
  return -1; // Returns -1 if no empty index
}

/* Returns the cache index of the new empty block. The cache lock should
  already be acquired. */
int
clock_replacement(struct block *block) {
  int i;
  int start = buffer_cache->hand;
  struct metadata *cur_meta;
  int cache_index = -1;
  for (i = 0; true; i++) {
    int j = (i + start)%CACHE_SECTOR_COUNT;
    cur_meta = meta_with_index (j);
    if ((!cur_meta->populated || cur_meta->clock_count == 0) && !cur_meta->loading && !cur_meta->evicting) {
      // The for loop should not terminate before entering this if statement
      cache_index = j;
      buffer_cache->hand = (j + 1)%CACHE_SECTOR_COUNT; // Set the cache's hand to the proper index
      break;
    } else {
      cur_meta->clock_count = 0;
    }
  }

  // Make sure the acquired sector is empty
  if (cur_meta->populated) {
    // This should never not be the case, since cache_empty_index should have been called first
    sector_flush (block, cur_meta->sector_num, true);
  }
  
  return cache_index;
}

/* Flushes an individual sector from cache to disk and evicts it if evict is true. */
void
sector_flush (struct block *block, block_sector_t sector_num, bool evict) {
  struct cache_sect *cur_sect = cache_get (sector_num);
  ASSERT (cur_sect != NULL); // The sector_num must be in the cache
  ASSERT (buffer_cache->cache_lock.semaphore.value == 0); // IMPORTANT: Cache should already be locked.
  struct metadata *cur_meta = meta_get (sector_num);


  // The lock acquire and release order is important, I think acquire release, release acquire will work
  // struct metadata *meta = meta_get (sector_num);
  lock_acquire (&cur_meta->sect_lock);
  if (evict) cur_meta->evicting = true;
  lock_release (&buffer_cache->cache_lock);

  // Copy sector content to disk
  block_write (block, sector_num, cur_sect);
  cur_meta->dirty = false;
  cur_meta->evicting = false;
  if (evict) cur_meta->populated = false;

  lock_release (&cur_meta->sect_lock);
  lock_acquire (&buffer_cache->cache_lock);
}

/* Returns a loaded cache_sect for use. Initially tries to use cache_get but
  if the sector does not exist in the cache it loads it from disk. This method 
  should implement the replacement policy and evict sectors when necessary. */
struct cache_sect *
cache_load (struct block *block, block_sector_t sector_num) {
  ASSERT (buffer_cache->cache_lock.semaphore.value == 0); // IMPORTANT: Cache should already be locked.

  struct cache_sect *temp_sect = cache_get (sector_num);
  struct metadata *temp_meta = meta_get (sector_num);
  if (temp_sect == NULL) {
    // Get the index of an empty sect
    int empty_index = cache_empty_index ();
    if (empty_index == -1) {
      // Implement replacement policy, evict sector, and set empty_index to the evicted cache index
      empty_index = clock_replacement (block);
    }

    // Set temp_sect to proper cache sector
    temp_sect = sect_with_index (empty_index);
    temp_meta = meta_with_index (empty_index);

    // The lock acquire and release order is important, I think acquire release, release acquire will work
    lock_acquire (&temp_meta->sect_lock);
    temp_meta->populated = true;
    temp_meta->sector_num = sector_num;
    temp_meta->loading = true; //IMPORTANT: change this so that cache_load will wait on this
    lock_release (&buffer_cache->cache_lock);

    // Load sector from disk into cache sector and initialize the meta as though it were new
    block_read (block, sector_num, temp_sect);
    temp_meta->loading = false;

    lock_release (&temp_meta->sect_lock);
    lock_acquire (&buffer_cache->cache_lock);
  } else if (temp_meta->evicting) {
    lock_acquire (&temp_meta->sect_lock);
    lock_release (&temp_meta->sect_lock);
    return cache_load (block, sector_num);
  }
  return temp_sect;
}

/* Function to replace block_read () in p****. 
  These (this function and cache_block_write) are theoretically the only functions you must
  be concered about working with the locks. */
void
cache_block_read (struct block *block, block_sector_t sector_num, void *buffer) {
  lock_acquire (&buffer_cache->cache_lock);

  struct cache_sect * temp_sect = cache_load (block, sector_num);

  // executre read; Locks necessary so that we know the sector is loaded fully from disk
  struct metadata *meta = meta_get (sector_num);
  lock_acquire (&meta->sect_lock);
  memcpy (buffer, temp_sect->data, BLOCK_SECTOR_SIZE);
  lock_release (&meta->sect_lock);


  lock_release (&buffer_cache->cache_lock);
}

/* Function to replace block_write () in p****. 
  These (this function and cache_block_read) are theoretically the only functions you must
  be concered about working with the locks. */
void
cache_block_write (struct block *block, block_sector_t sector_num, const void *buffer) {
  lock_acquire (&buffer_cache->cache_lock);

  struct cache_sect * temp_sect = cache_load (block, sector_num);

  // execute write; Locks necessary so that we know the sector is loaded fully from disk
  struct metadata *meta = meta_get (sector_num);
  lock_acquire (&meta->sect_lock);
  memcpy (temp_sect->data, buffer, BLOCK_SECTOR_SIZE);
  meta->dirty = true;
  lock_release (&meta->sect_lock);


  lock_release (&buffer_cache->cache_lock);
}

/* Function to flush all dirty sectors from cache to disk. */
void
cache_flush (struct block *block) {
  lock_acquire (&buffer_cache->cache_lock);

  int i;
  for (i = 0; i < CACHE_SECTOR_COUNT; i++) {
    struct metadata *cur_meta = meta_with_index (i);
    if (cur_meta->populated && cur_meta->dirty) {
      sector_flush (block, cur_meta->sector_num, false);
    }
  }

  lock_release (&buffer_cache->cache_lock);
}

/* Project 3: End, cache.c */