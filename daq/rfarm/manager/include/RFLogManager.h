/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFLOGMANAGER_H
#define RFLOGMANAGER_H

#include <stdio.h>

namespace Belle2 {
  class RFLogManager {
  public:
    RFLogManager(const char* prefix, const char* lognode = NULL, const char* logdir = NULL);
    ~RFLogManager();

    // Read messages from fd and log them
    int ProcessLog(int fd);

    // Logger functions
    void Log(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Warning(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Fatal(const char* fmt, ...);
    void Abort(const char* fmt, ...);

    // Basic interface
    void Write(const char* msg);
    char* BuildMessage(const char* fmt, ...);

  private:
    int today();
    void timestamp(char*);
    int WriteLog(const char* type, const char* msg);
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

