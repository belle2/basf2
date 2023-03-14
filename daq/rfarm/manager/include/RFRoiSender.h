/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFFlowStat.h"
#include "daq/rfarm/manager/RFServerBase.h"

namespace Belle2 {

  class RFRoiSender : public RFServerBase {
  public:
    RFRoiSender(std::string conffile);
    ~RFRoiSender();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;
    int Restart(NSMmsg*, NSMcontext*) override;

    // Server function
    void server();

    // Cleanup
    void cleanup();

  private:
    RFConf*            m_conf{};
    RFProcessManager*  m_proc{};
    RFLogManager*      m_log{};
    RFSharedMem*       m_shm{};
    RFFlowStat*        m_flow{};

    int m_pid_merger{};
    int m_pid_sender{};

  };
}

