#ifndef RFLOGMANAGER_H
#define RFLOGMANAGER_H
//+
// File : RFLogManager.h
// Description : Various logging functions
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 12 - Jul - 2013
//-

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <nsm2/nsm2.h>
#include <nsm2/belle2nsm.h>

namespace Belle2 {
  class RFLogManager {
  public:
    RFLogManager(char* prefix, char* lognode = NULL, char* logdir = NULL);
    ~RFLogManager();

    // Read messages from fd and log them
    int ProcessLog(int fd);

    // Logger functions
    void Log(char* fmt, ...);
    void Info(char* fmt, ...);
    void Warning(char* fmt, ...);
    void Error(char* fmt, ...);
    void Fatal(char* fmt, ...);
    void Abort(char* fmt, ...);

    // Basic interface
    void Write(char* msg);
    char* BuildMessage(char* fmt, ...);

  private:
    int today();
    void timestamp(char*);
    int WriteLog(char* type, char* msg);
    int OpenLogFile(int today);
    int SwitchLogFile();

  private:
    int m_logdate;
    char m_logdir[1024];
    char m_id[256];
    char m_strbuf[1024];
    int m_fd;
    char m_lognode[1024];

  };

}
#endif

