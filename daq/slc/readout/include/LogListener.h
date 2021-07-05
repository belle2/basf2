/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_LogListener_hh
#define _Belle2_LogListener_hh

namespace Belle2 {

  class ProcessController;

  class LogListener {

  public:
    LogListener(ProcessController* con, int pipe[2], bool enableUnescapeNewlines = true)
      : m_con(con), m_enableUnescapeNewlines(enableUnescapeNewlines)
    {
      m_pipe[0] = pipe[0];
      m_pipe[1] = pipe[1];
    }
    ~LogListener() {}

  public:
    void run();

  private:
    ProcessController* m_con;
    int m_pipe[2];

    bool m_enableUnescapeNewlines;

  };

}

#endif

