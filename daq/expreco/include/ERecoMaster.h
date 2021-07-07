/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef ER_MASTER_H
#define ER_MASTER_H

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFLogManager.h"

#include "daq/rfarm/manager/RFServerBase.h"

namespace Belle2 {
  class ERecoMaster : public RFServerBase {
  public:
    ERecoMaster(std::string conf);
    ~ERecoMaster();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*);
    int UnConfigure(NSMmsg*, NSMcontext*);
    int Start(NSMmsg*, NSMcontext*);
    int Stop(NSMmsg*, NSMcontext*);
    int Restart(NSMmsg*, NSMcontext*);

    // Function to hook message funnctions
    void Hook_Message_Handlers();

    // Message functions
    static void Log_Handler(NSMmsg* msg, NSMcontext* ctx);

    // Server function
    void monitor_loop();


  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFLogManager*      m_log;

  };

}
#endif



