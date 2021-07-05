/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ProcessListener_hh
#define _Belle2_ProcessListener_hh

namespace Belle2 {

  class ProcessController;

  class ProcessListener {

  public:
    ProcessListener(ProcessController* con)
      : m_con(con) {}
    ~ProcessListener() {}

  public:
    void run();

  private:
    ProcessController* m_con;

  };

}

#endif

