/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef EREVENTPROCESSOR_H
#define EREVENTPROCESSOR_H

#include <string>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"
#include "daq/rfarm/manager/RFFlowStat.h"

#include "framework/pcore/RingBuffer.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class ERecoEventProcessor : public RFServerBase {
  public:
    explicit ERecoEventProcessor(const std::string& conffile);
    ~ERecoEventProcessor();
    /** Copy constructor (disabled). */
    ERecoEventProcessor(const ERecoEventProcessor&) = delete;

    /** Operator = (disabled). */
    ERecoEventProcessor& operator=(const ERecoEventProcessor&) = delete;

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;
    int Restart(NSMmsg*, NSMcontext*) override;

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    RFSharedMem*       m_shm;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RingBuffer*        m_rbufin;
    RingBuffer*        m_rbufout;
    RFFlowStat*        m_flow;

    int m_pid_receiver = 0;
    int m_pid_basf2 = 0;
    int m_pid_sender = 0;
    int m_pid_hrecv = 0;
    int m_pid_hrelay = 0;
    int m_pid_evs = 0;
    char m_nodename[256 + 4];

  };

}
#endif




