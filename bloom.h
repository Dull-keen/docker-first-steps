#ifndef BLOOM_H
#define BLOOM_H

void bloom_init();
void bloom_free();
void bloom_add(const char *key);
int  bloom_check(const char *key);
int  bloom_configure(const char *hash_list); // Новый: настройка хешей

#endif
