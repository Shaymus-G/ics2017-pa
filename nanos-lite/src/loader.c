#include "common.h"
#include <unistd.h>

#define DEFAULT_ENTRY ((void *)0x8048000)

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
//extern size_t fs_lseek(int fd, size_t offset, int whence);
extern size_t fs_filesz(int fd);
extern int fs_close(int fd);
void* new_page(void);

uintptr_t loader_brk = 0;

uintptr_t loader(_Protect *as, const char *filename) {
  if (filename == NULL) {
    filename = "/bin/init";
  }

  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);

  loader_brk = PGROUNDUP((uintptr_t)DEFAULT_ENTRY + size);

  if (as == NULL) {
    fs_read(fd, DEFAULT_ENTRY, size);
  } else {
    uintptr_t va = (uintptr_t)DEFAULT_ENTRY;
    size_t offset = 0;

    while (offset < size) {
      void *pa = new_page();
      memset(pa, 0, PGSIZE);

      _map(as, (void *)va, pa);

      size_t len = size - offset;
      if (len > PGSIZE) {
        len = PGSIZE;
      }

      fs_read(fd, pa, len);

      va += PGSIZE;
      offset += len;
    }
  }

  fs_close(fd);

  Log("load program: filename = %s, size = %d, entry = %p", filename, size, DEFAULT_ENTRY);

  return (uintptr_t)DEFAULT_ENTRY;
}
