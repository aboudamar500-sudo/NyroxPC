#ifndef NYROX_MEMORY_MANAGER_H
#define NYROX_MEMORY_MANAGER_H

#include "boot_info.h"

int mm_init(const NyroxBootInfo *info);
u64 mm_alloc_page(void);
int mm_free_page(u64 address);
u64 mm_free_pages_count(void);

#endif
