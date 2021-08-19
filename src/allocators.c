#include <stdio.h>
#include <pico/stdlib.h>

// idk what to include to get implicit warnings to go away for free/memcpy/memalign
//#include <pico/malloc.h>
#include <stdlib.h>
#include <memory.h>

#include "walker.h"
#include "allocators.h"

uint8_t* pico_pw_alloc(void* ud,
		size_t len, uint32_t ptr_align, uint32_t len_align, size_t ret_addr) {

	uint8_t *ptr = NULL;
	uint32_t align_offset = len_align - (len%len_align);
	int err = memalign(&ptr, ptr_align, len+align_offset);
	if( err < 0 );	// Should probably handle this


	return ptr;
}

size_t pico_pw_realloc(void* ud,
		uint8_t *buf, size_t buf_len, uint32_t buf_align,
		size_t new_len, uint32_t len_align, size_t ret_addr) {

	uint8_t *ptr = NULL;
	uint32_t align_offset = len_align - (new_len%len_align);
	int err = memalign(&ptr, buf_align, new_len+align_offset);
	if( err < 0 );	// Should probably handle this

	// Should i do somehting with buf_align here?
	memcpy(ptr, buf, buf_len);
	free(buf);

	return new_len+align_offset;

}
