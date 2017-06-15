#ifndef RFPROCESSMANAGER_H
#define RFPROCESSMANAGER_H
//+
// File : RFProcessManager.h
// Description : Manages process forking for RFARM control clients
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 20 - June - 2013
//-

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <vector>


namespace Belle2 {

  class RFProcessManager {
  public:
    RFProcessManager(char* nodename);
    virtual ~RFProcessManager();

    // Signal handler
    static void signal_handler(int num);

    // Execute script in a child process
    int Execute(char* script, int nargs, char** args);
    int Execute(char* scr, char* arg0 = NULL, char* arg1 = NULL, char* arg2 = NULL, char* arg3 = NULL, char* arg4 = NULL,
                char* arg5 = NULL, char* arg6 = NULL, char* arg7 = NULL);

    // Check Output Request from child
    int CheckOutput();

    int GetFd();

    // Check process status
    pid_t CheckProcess();

  private:
    int m_iopipe[2];
    int m_fdout;
    std::vector<pid_t> m_pidlist;

  };
}
#endif




