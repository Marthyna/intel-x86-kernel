#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

#include "hash.h"
#include "stddef.h"
#include "mem.h"

#define PAGE_SIZE 0x1000

struct shared_page {
    void *add;
    int references;
};

void shm_config();
void *shm_create(const char *key);
void *shm_acquire(const char *key);
void shm_release(const char *key);

#endif