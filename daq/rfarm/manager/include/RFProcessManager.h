/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <stdio.h>
#include <sys/types.h>

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



