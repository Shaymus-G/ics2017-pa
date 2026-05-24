/* connector for getpid */

#include <reent.h>
#include <sys/types.h>

int _getpid(void);

int
getpid ()
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _getpid_r (_REENT);
#else
  return _getpid ();
#endif
}
