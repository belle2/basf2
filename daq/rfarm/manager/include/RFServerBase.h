/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef SERVER_BASE_H
#define SERVER_BASE_H

#include "daq/rfarm/manager/RfNodeInfo.h"
#include <nsm2/nsm2.h>

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

    virtual void SetNodeInfo(RfNodeInfo* ptr)
    {
      m_nsmmem = ptr;
    };

    virtual RfNodeInfo* GetNodeInfo()
    {
      return m_nsmmem;
    };

  public:
    static RFServerBase* s_instance;

  private:
    RfNodeInfo* m_nsmmem;

  };
}
#endif
