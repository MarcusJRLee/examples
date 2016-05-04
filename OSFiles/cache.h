#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <stdbool.h>
#include "devices/block.h" // for BLOCK_SECTOR_SIZE
#include "threads/synch.h" // for struct lock


/* Proj 3: Cache.h */


/* Number of sectors to keep in cache */
#define CACHE_SECTOR_COUNT 64

struct cache_sect {
	char data[BLOCK_SECTOR_SIZE];
};

struct cache_sectors {
	struct cache_sect sector_array[CACHE_SECTOR_COUNT];
};

struct metadata {
	bool populated; // Indicates whether this cache block is in use
	bool loading;
	bool evicting;
	bool dirty;
	int clock_count; // 1 or 0 for eviction
	block_sector_t sector_num;
	struct lock sect_lock; // Lock to acquire when loading in this sector from disk
};

struct cache {
	struct metadata metadata[CACHE_SECTOR_COUNT];
	bool free_list[CACHE_SECTOR_COUNT];
	struct cache_sectors * cache_sectors;
	struct lock cache_lock;
	int hand; //index for clock hand position
};

/*Proj 3: End adding structs */

// External functions
void cache_init (void);
void cache_block_read (struct block *block, block_sector_t sector_num, void *buffer);
void cache_block_write (struct block *block, block_sector_t sector_num, const void *buffer);
void cache_flush (struct block *block);

// Internatl functions
void metadata_init (struct metadata *md);
int get_index (block_sector_t sector_num);
struct cache_sect * sect_with_index (int cache_index);
struct metadata * meta_with_index (int cache_index);
struct cache_sect * cache_get (block_sector_t sector_number);
struct metadata * meta_get (block_sector_t sector_num);
int cache_empty_index (void);
int clock_replacement(struct block *block);
void sector_flush (struct block *block, block_sector_t sector_num, bool evict);
struct cache_sect * cache_load (struct block *block, block_sector_t sector_num);

/* Proj 3: End, Cache.h */

#endif /* filesys/cache.h */