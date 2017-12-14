#ifndef RFNODEMANAGER_H
#define RFNODEMANAGER_H
//+
// File : RFNodeManager.h
// Description : Base class for various RFARM node control
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 20 - June - 2013
//-

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




