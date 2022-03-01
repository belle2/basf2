/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RF_MASTER_H
#define RF_MASTER_H

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFLogManager.h"

#include "daq/rfarm/manager/RFServerBase.h"

namespace Belle2 {
  class RFMaster : public RFServerBase {
  public:
    RFMaster(std::string conf);
    ~RFMaster();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;
    int Restart(NSMmsg*, NSMcontext*) override;

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



