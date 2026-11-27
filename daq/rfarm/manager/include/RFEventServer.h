/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFEVENTSERVER_H
#define RFEVENTSERVER_H

#include <string>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFEventServer : public RFServerBase {
  public:
    explicit RFEventServer(const std::string& conffile);
    ~RFEventServer();
    /** Copy constructor (disabled). */
    RFEventServer(const RFEventServer&) = delete;

    /** Operator = (disabled). */
    RFEventServer& operator=(const RFEventServer&) = delete;

    // Instance of singleton
    static RFEventServer& Create(const std::string& conffile);
    static RFEventServer& Instance();

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
    RFSharedMem*       m_shm;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RFFlowStat*        m_flow;
    RingBuffer*        m_rbufin;

    int m_pid_recv;
    int m_pid_sender[MAXNODES];
    int m_nnodes = 0;

  private:
    // cppcheck-suppress duplInheritedMember ; the derived class deliberately provides its own
    static RFEventServer* s_instance;
  };

}
#endif




