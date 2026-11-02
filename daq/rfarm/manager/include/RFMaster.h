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
    explicit RFMaster(const std::string& conf);
    ~RFMaster();
    /** Copy constructor (disabled). */
    RFMaster(const RFMaster&) = delete;

    /** Operator = (disabled). */
    RFMaster& operator=(const RFMaster&) = delete;

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;
    int Restart(NSMmsg*, NSMcontext*) override;

    // Function to hook message funnctions
    static void Hook_Message_Handlers();

    // Message functions
    static void Log_Handler(NSMmsg* msg, NSMcontext* ctx);

    // Server function
    static void monitor_loop();


  private:
    RFConf*            m_conf;
    // RFSharedMem*       m_shm;
    RFLogManager*      m_log;

  };

}
#endif



