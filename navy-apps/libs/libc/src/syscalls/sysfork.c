/* connector for fork */

#include <reent.h>

int _fork(void);

int
fork ()
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _fork_r (_REENT);
#else
  return _fork ();
#endif
}
