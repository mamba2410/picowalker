#ifndef ALLOCATORS_H
#define ALLOCATORS_H

#include <pico/stdlib.h>

uint8_t* pico_pw_alloc(void* ud,
		size_t len, uint32_t ptr_align, uint32_t len_align, size_t ret_addr);

size_t pico_pw_realloc(void* ud,
		uint8_t *buf, size_t buf_len, uint32_t buf_align,
		size_t new_len, uint32_t len_align, size_t ret_addr);

#endif /* ALLOCATORS_H */
