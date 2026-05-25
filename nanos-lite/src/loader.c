#include "common.h"
#include <unistd.h>

#define DEFAULT_ENTRY ((void *)0x4000000)

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
//extern size_t fs_lseek(int fd, size_t offset, int whence);
extern size_t fs_filesz(int fd);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;

  if (filename == NULL) {
    filename = "/bin/init";
  }

  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);

  fs_read(fd, DEFAULT_ENTRY, size);
  fs_close(fd);

  Log("load program: filename = %s, size = %d, entry = %p", filename, size, DEFAULT_ENTRY);

  return (uintptr_t)DEFAULT_ENTRY;
}
