#ifndef DIRECTORY
#define DIRECTORY

#include<stdio.h>
#include"Cache.h"

struct directory{
	struct L1_cache *cache0;
	struct L1_cache *cache1;
	struct L1_cache *cache2;
	struct L1_cache *cache3;
}

struct directory* directory_init(struct L1_cache C0, struct L1_cache C1, struct L1_cache C2, struct L1_cache C3){
	struct directory *dir = malloc(sizeof(struct directory));
	dir -> cache0 = C0;
	dir -> cache0 = C1;
	dir -> cache0 = C2;
	dir -> cache0 = C3;

	return dir;
}

int check_cache_block_in_all(struct directory *dir,struct cache_block *block){ //Returns 1 if the block exists somewhere else in the cache hierarchy else returns 0
	if(lookup_cache(dir->cache0, block->addr) != NULL){
		return 1; //Means the block exists 
	}
	else if( lookup_cache(dir->cache1, block->addr) != NULL ){
		return 1;
	}
	else if( lookup_cache(dir->cache2, block->addr) != NULL ){
		return 1;
	}
	else if( lookup_cache(dir->cache3, block->addr) != NULL ){
		return 1;
	}	

	return 0; //Not found 
}


