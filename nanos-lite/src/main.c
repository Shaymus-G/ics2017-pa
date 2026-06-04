#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

#define USER_STACK_TOP 0x07f00000

#ifndef HAS_PTE
static void enter_user(uint32_t entry, const char *filename) {
  uintptr_t sp = USER_STACK_TOP;

  size_t len = strlen(filename) + 1;
  sp -= len;
  memcpy((void *)sp, filename, len);
  uintptr_t argv0_str = sp;

  sp &= ~0x3;

  sp -= 4;
  *(uintptr_t *)sp = 0;
  uintptr_t envp = sp;

  sp -= 4;
  *(uintptr_t *)sp = 0;

  sp -= 4;
  *(uintptr_t *)sp = argv0_str;
  uintptr_t argv = sp;

  sp -= 4;
  *(uintptr_t *)sp = envp;

  sp -= 4;
  *(uintptr_t *)sp = argv;

  sp -= 4;
  *(uintptr_t *)sp = 1;

  sp -= 4;
  *(uintptr_t *)sp = 0;

  Log("enter user: entry = 0x%x, esp = 0x%x, argv = 0x%x, envp = 0x%x, argv0 = %s", entry, sp, argv, envp, (char *)argv0_str);

  asm volatile(
    "movl %0, %%esp;"
    "jmp *%1"
    :
    : "r"(sp), "r"(entry)
    : "memory"
  );

  panic("Should not reach here");
}
#endif

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);
void load_prog(const char *filename);
void run_first_proc(void);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();

  //uint32_t entry = loader(NULL, "/bin/init");
  //((void (*)(void))entry)();
  const char *filename = "/bin/pal";

#ifdef HAS_PTE
  load_prog(filename);
  run_first_proc();
#else
  uint32_t entry = loader(NULL, filename);
  enter_user(entry, filename);
#endif

  panic("Should not reach here");
}
