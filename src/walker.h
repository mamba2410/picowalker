
#ifndef PWEMU_WALKER_H_
#define PWEMU_WALKER_H_

#include <stddef.h>
#include <stdint.h>

// walker.zig C interface to communicate with the outside world

typedef uint8_t* (*pw_alloc_fn)(void* ud, size_t len, uint32_t ptr_align,
		uint32_t len_align, size_t ret_addr);
typedef size_t (*pw_resize_fn)(void* ud, uint8_t* buf, size_t buf_len,
		uint32_t buf_align, size_t new_len, uint32_t len_align, size_t ret_addr);

struct pw_allocator {
	pw_alloc_fn alloc;
	pw_resize_fn resize;
	void* ud;
};

#ifndef PWEMU_NO_PWLIB_EXPORTS
typedef void* pw_walker_t;

size_t pw_walker_size(void);
void pw_walker_init(pw_walker_t walker, uint8_t* flashrom, uint8_t* eeprom,
		const struct pw_allocator* alloc_sched_event,
		const struct pw_allocator* alloc_ram);
void pw_walker_reset(pw_walker_t walker);
void pw_walker_run(pw_walker_t walker, uint64_t inc);

bool pw_walker_sched_get_interactive(pw_walker_t walker);
void pw_walker_sched_set_interactive(pw_walker_t walker, bool interactive);
bool pw_walker_sched_had_breakpoint(pw_walker_t walker);

void pw_walker_stat(pw_walker_t walker);
#endif

#endif

