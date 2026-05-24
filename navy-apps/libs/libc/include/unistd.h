#ifndef _UNISTD_H_
#define _UNISTD_H_

# include <sys/unistd.h>
# include <sys/types.h>
# include <stddef.h>

int _open(const char *pathname, int flags, int mode);
int _close(int fd);
int _read(int fd, void *buf, size_t count);
int _write(int fd, const void *buf, size_t count);
off_t _lseek(int fd, off_t offset, int whence);

#endif /* _UNISTD_H_ */
