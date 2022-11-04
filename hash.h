#ifndef HASH_H
#define HASH_H



#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <cstdbool>
#include <iostream>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#endif // __cplusplus

// Typ funkcji hashującej.
typedef uint64_t (*hash_function_t) (uint64_t const *, size_t);

#ifdef __cplusplus
namespace jnp1 {
    extern "C" {
#endif // __cplusplus

unsigned long hash_create(hash_function_t hash_function);

void hash_delete(unsigned long id);

size_t hash_size(unsigned long id);

bool hash_insert(unsigned long id, uint64_t const * seq, size_t size);

bool hash_remove(unsigned long id, uint64_t const * seq, size_t size);

void hash_clear(unsigned long id);

bool hash_test(unsigned long id, uint64_t const * seq, size_t size);

#ifdef __cplusplus
    }}
#endif // __cplusplus

#endif // HASH_H