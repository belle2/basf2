/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFNODEMANAGER_H
#define RFNODEMANAGER_H

#include
namespace Belle2 {

  class RFNodeManager : public RFSharedMem, public RFNSM, public RFConf {
  public:
    RFNodeManager(string& nodename);
    virtual ~RFNodeManager();

    static void signal_handler(int num);

    // Utility functions to fork worker process
    int fork(char* script, int nargs, char** args);

  private:
    int piperec[2];
    int pipesend[2];
  };
}
#endif




