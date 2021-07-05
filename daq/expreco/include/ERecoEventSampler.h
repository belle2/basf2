/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef ERECO_EVENT_SAMPLER_H
#define ERECO_EVENT_SAMPLER_H

#include <string>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFLogManager.h>
#include <daq/rfarm/manager/RFServerBase.h>

#include <framework/pcore/RingBuffer.h>

#include <nsm2/nsm2.h>

#define MAXEVTSIZE 80000000

namespace Belle2 {
  class ERecoEventSampler : public RFServerBase {
  public:
    ERecoEventSampler(std::string conffile);
    ~ERecoEventSampler();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*);
    int UnConfigure(NSMmsg*, NSMcontext*);
    int Start(NSMmsg*, NSMcontext*);
    int Stop(NSMmsg*, NSMcontext*);
    int Restart(NSMmsg*, NSMcontext*);

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    std::string        m_conffile;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RingBuffer*        m_rbufout;

    int m_pid_sampler;
    int m_pid_server;

  };

}
#endif
