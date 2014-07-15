#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>

const char* program_name_in_syslog = "";

int
isasocket(int fd)
{
  struct stat buf;
  int ret;

  ret = fstat(fd, &buf);
  assert(ret == 0);

  if (buf.st_mode & S_IFSOCK) {
    return 1;
  } else {
    return 0;
  }
}

void log(const char* format, ...)
{
  static int use_syslog = -1;

  if (use_syslog == -1) {
    if (isasocket(2)) {
      use_syslog = 1;
      openlog(program_name_in_syslog, LOG_NDELAY | LOG_PID, LOG_DAEMON);
    } else {
      fprintf(stderr, "log from pid %d\n", getpid());
      use_syslog = 0;
    }
  }
  va_list arg;
  va_start(arg, format);
  if (use_syslog) {
    vsyslog(LOG_INFO, format, arg);
  } else {
    vfprintf(stderr, format, arg);
  }
  va_end(arg);
}

#endif
