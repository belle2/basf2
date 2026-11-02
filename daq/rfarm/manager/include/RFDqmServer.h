/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RF_DQMSERVER_H
#define RF_DQMSERVER_H

#include <string>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFDqmServer : public RFServerBase {
  public:
    explicit RFDqmServer(const std::string& conffile);
    ~RFDqmServer();
    /** Copy constructor (disabled). */
    RFDqmServer(const RFDqmServer&) = delete;

    /** Operator = (disabled). */
    RFDqmServer& operator=(const RFDqmServer&) = delete;

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
    RFConf*            m_conf;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;

    int m_pid_dqm = 0;
    int m_pid_relay = 0;

    int m_expno = 0;
    int m_runno = 0;

  };
}
#endif

