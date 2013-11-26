#ifndef SERVER_BASE_H
#define SERVER_BASE_H
//+
// File : RFServerBase.h
// Description : Base class to construct RFARM server applications
//               The class can be fed to RFNSM to hook up functions
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - Jul - 2013
//-

#include "daq/rfarm/manager/RfNodeInfo.h"

namespace Belle2 {
  class RFServerBase {
  public:
    RFServerBase() {};
    ~RFServerBase() {};

    virtual int Configure(NSMmsg*, NSMcontext*) { return 0; };
    virtual int UnConfigure(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Start(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Stop(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Pause(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Resume(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Restart(NSMmsg*, NSMcontext*) { return 0; };
    virtual int Status(NSMmsg*, NSMcontext*) { return 0; };

    virtual void SetNodeInfo(RfNodeInfo* ptr) {
      m_nsmmem = ptr;
    };

    virtual RfNodeInfo* GetNodeInfo() {
      return m_nsmmem;
    };

  public:
    static RFServerBase* s_instance;

  private:
    RfNodeInfo* m_nsmmem;

  };
}
#endif
