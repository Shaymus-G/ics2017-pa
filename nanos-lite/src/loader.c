#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern size_t get_ramdisk_size();
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;

  size_t size = get_ramdisk_size();

  ramdisk_read(DEFAULT_ENTRY, 0, size);

  Log("load program from ramdisk: filename = %s, size = %d, entry = %p", filename, size, DEFAULT_ENTRY);

  return (uintptr_t)DEFAULT_ENTRY;
}
