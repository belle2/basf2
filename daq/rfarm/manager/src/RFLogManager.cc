//+
// File : RFLogManager.cc
// Description : Variout logging functions
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Jul - 2013
//-

#include "daq/rfarm/manager/RFLogManager.h"
#include <iostream>
#include <time.h>

using namespace Belle2;
using namespace std;

#define VSNPRINTF(s,l,f,a) va_start(a,f);vsnprintf(s,l,f,a);va_end(a)

// Constructor/Destructor

RFLogManager::RFLogManager(char* id, char* lognode, char* logdir)
{
  strcpy(m_id, id);

  if (lognode != NULL)
    strcpy(m_lognode, lognode);
  else
    strcpy(m_lognode, "LOGC");

  if (logdir != NULL)
    strcpy(m_logdir, logdir);
  else
    m_logdir[0] = 0;

  m_logdate = today();
  OpenLogFile(m_logdate);
}

RFLogManager::~RFLogManager()
{
  close(m_fd);
}

// Basic Function

int RFLogManager::today()
{
  time_t today = time(NULL);
  struct tm* tmtoday = gmtime(&today);
  int day = tmtoday->tm_mday;
  int month = tmtoday->tm_mon + 1;
  int year = tmtoday->tm_year - 100;
  //  printf ( "RFLogManager: day = %d, month = %d, year = %d\n", day, month, year );
  return day + month * 100 + year * 10000;
}

int RFLogManager::OpenLogFile(int today)
{
  char filename[1024];
  if (m_logdir[0] != 0)
    sprintf(filename, "%s/%s_d%6.6d.log", m_logdir, m_id, today);
  else
    sprintf(filename, "%s_d%6.6d.log", m_id, today);

  m_fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
  if (m_fd < 0) {
    fprintf(stderr, "RfLogManager(%s) : error to open file %s\n",
            m_id, filename);
    perror("RFLogManager");
  } else {
    // Make symbolic link to "latest.log"
    char slinkname[1024];
    sprintf(slinkname, "latest.log");
    unlink(slinkname);
    symlink(filename, slinkname);
    printf("RFLogManager: symbolic link to %s\n", slinkname);
  }
  return m_fd;
}

int RFLogManager::SwitchLogFile()
{
  int myday = today();
  if (myday != m_logdate) {
    close(m_fd);
    OpenLogFile(myday);
    m_logdate = myday;
  }
  return 0;
}


int RFLogManager::WriteLog(char* prefix, char* msg)
{
  SwitchLogFile();
  char wbuf[1024];
  timestamp(wbuf);
  sprintf(&wbuf[strlen(wbuf) - 1], "%s %s\n", prefix, msg);
  int st = write(m_fd, wbuf, strlen(wbuf));
  return st;
}

void RFLogManager::timestamp(char* buf)
{
  struct timeval tb;
  struct tm*     tp;
  //  struct tm      result;
  gettimeofday(&tb, NULL);
  tp = localtime(&tb.tv_sec);
  sprintf(buf, "%02d:%02d:%02d.%03d ",
          tp->tm_hour, tp->tm_min, tp->tm_sec, (int)(tb.tv_usec / 1000));
  return;
}

char* RFLogManager::BuildMessage(char* fmt, ...)
{
  va_list arg;

  m_strbuf[sizeof(m_strbuf) - 1] = 0;

  VSNPRINTF(m_strbuf, sizeof(m_strbuf), fmt, arg);

  return m_strbuf;

  /*
  int len = strlen(m_strbuf);
  while(m_strbuf[len-1] == '\n' ) m_strbuf[--len] = 0;
  m_strbuf[len++] = '\n';
  m_strbuf[len] = 0;
  return m_strbuf;
  */
}

void RFLogManager::Log(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("", msg);
}

void RFLogManager::Info(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("[info]", msg);
  int pars[2];
  pars[0] = 2;
  pars[1] = (int)time(NULL);
  b2nsm_sendany(m_lognode, "LOG", 2, pars, strlen(msg) + 1, msg, NULL);
}

void RFLogManager::Warning(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("[warning]", msg);
  int pars[2];
  pars[0] = 4;
  pars[1] = (int)time(NULL);
  b2nsm_sendany(m_lognode, "LOG", 2, pars, strlen(msg) + 1, msg, NULL);
}

void RFLogManager::Error(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("[error]", msg);
  int pars[2];
  pars[0] = 5;
  pars[1] = (int)time(NULL);
  b2nsm_sendany(m_lognode, "LOG", 2, pars, strlen(msg) + 1, msg, NULL);
}

void RFLogManager::Fatal(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("[fatal]", msg);
  int pars[2];
  pars[0] = 6;
  pars[1] = (int)time(NULL);
  b2nsm_sendany(m_lognode, "LOG", 2, pars, strlen(msg) + 1, msg, NULL);
}

void RFLogManager::Abort(char* fmt, ...)
{
  va_list ap;
  char msg[1000];

  msg[sizeof(msg) - 1] = 0;
  VSNPRINTF(msg, sizeof(msg), fmt, ap);
  WriteLog("[abort]", msg);
  b2nsm_sendany(m_lognode, "LOG", 0, NULL, strlen(msg), msg, NULL);
  abort();
}

// Process Log

int RFLogManager::ProcessLog(int fd)
{
  char* p, *q, buf[4000000];
  int len;
  int siz = sizeof(buf) - 1;
  int toolong = 0;

  p = buf;
  fd_set fdset;
  do {
    FD_ZERO(&fdset);
    if (fd <= 0) {
      printf("Pipe for log is not availablle\n");
      Abort("No pipe available");
    }
    FD_SET(fd, &fdset);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (select(fd + 1, &fdset, NULL, NULL, &tv) < 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("select");
          return -1;
      }
      return -1;
    } else if (!(fd > 0 && FD_ISSET(fd, &fdset))) {
      return 0;
    }
    if ((len = read(fd, p, siz)) < 0) {
      if (errno == EINTR) continue;
      if (errno == EPIPE) {
        Log("broken pipe fd=%d", fd);
        close(fd);
        return -1;
      } else if (errno == EBADF) {
        Log("bad fd=%d", fd);
        return -1;
      }
      perror("read");
      Abort("read");
    } else if (len == 0) {
      close(fd);
      return -1;
    } else {
      p[len] = 0;
      len += p - buf;
      p = buf;
      do {
        if (!(q = strchr(p, '\n'))) {
          if (p == buf && len == siz) {
            if (! toolong)
              Warning("too long message from fd=%d", fd);
            toolong = 1;
          } else {
            len -= p - buf;
            memmove(buf, p, len);
            siz = sizeof(buf) - len;
            p = buf + len;
          }
          break;
        }
        *q++ = 0;
        if (! toolong) {
          if (strlen(p) > 13 &&
              p[2] == ':' && p[5] == ':' && p[8] == '.' && p[12] == ' ') {
            p += 13;
          }
          if (strncmp(p, "[FATAL] ", 8) == 0) {
            Fatal("%s", p + 8);
          } else if (strncmp(p, "[ERROR] ", 8) == 0) {
            Error("%s", p + 8);
          } else if (strncmp(p, "[WARNING] ", 10) == 0) {
            Warning("%s", p + 10);
          } else if (strncmp(p, "[INFO] ", 7) == 0) {
            Info("%s", p + 7);
          } else if (strncmp(p, "[ABORT] ", 8) == 0) {
            Fatal("abort - %s", p + 8);
          } else if (strncmp(p, "[sysexit] ", 10) == 0) {
            Fatal("sysexit - %s", p + 10);
          } else {
            Log("%s", p);
          }
        }
        toolong = 0;
        p = q;
        if (! *p) return 0;
      } while (*p);
    }
  } while (1);
}






