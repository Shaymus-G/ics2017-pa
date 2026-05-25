#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      r->eax = 1;
      break;

    case SYS_exit:
      Log("SYS_exit, status = %d", a[1]);
      _halt(a[1]);
      break;

    case SYS_open:
      r->eax = fs_open((const char *)a[1], a[2], a[3]);
      break;

    case SYS_read:
      r->eax = fs_read(a[1], (void *)a[2], a[3]);
      break;

    case SYS_write:
      r->eax = fs_write(a[1], (const void *)a[2], a[3]);
      break;

    case SYS_lseek:
      r->eax = fs_lseek(a[1], a[2], a[3]);
      break;

    case SYS_close:
      r->eax = fs_close(a[1]);
      break;

    case SYS_brk: {
      uintptr_t new_brk = a[1];
      //Log("SYS_brk: new_brk = 0x%x", new_brk);

      if (new_brk >= 0x04000000 && new_brk < 0x08000000) {
        r->eax = 0;
      } else {
	//Log("SYS_brk failed: new_brk = 0x%x", new_brk);
	r->eax = -1;
      }

      break;
		  }

    case SYS_execve: {
      const char *filename = (const char *)a[1];
      Log("SYS_execve called: filename = %s", filename);
      r->eax = -1;
      break;
		     }

    default:
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
