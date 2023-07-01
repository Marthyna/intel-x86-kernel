#include "shared_memory.h"

hash_t mem_table;

void shm_config() {
    hash_init_string(&mem_table);
}

void *shm_create(const char *key) {

    //verify if the key already exists 
    if(key == NULL || hash_isset(&mem_table, (void *)key))
        return NULL;
    
    void *page_adress = mem_alloc(PAGE_SIZE);

    struct shared_page *shared_page = mem_alloc(sizeof(struct shared_page));
    
    shared_page->add = page_adress;
    shared_page->references = 0;

    hash_set(&mem_table, (void *)key, shared_page);

    return page_adress;
}

void *shm_acquire(const char *key) {

    struct shared_page *shared_page = hash_get(&mem_table, (void *)key, (void *)NULL);
    
    shared_page->references++;
    
    return shared_page->add;
}

void shm_release(const char *key) {

    struct shared_page *shared_page = hash_get(&mem_table, (void *)key, (void *)NULL);

    // decrement the number of references and test if there are still references
    shared_page->references--;
    if(shared_page->references <= 0) {
        mem_free(shared_page->add, PAGE_SIZE);
    }

}