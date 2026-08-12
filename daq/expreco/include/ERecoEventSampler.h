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
    explicit ERecoEventSampler(const std::string& conffile);
    ~ERecoEventSampler();
    /** Copy constructor (disabled). */
    ERecoEventSampler(const ERecoEventSampler&) = delete;

    /** Operator = (disabled). */
    ERecoEventSampler& operator=(const ERecoEventSampler&) = delete;

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;

    // Server function
    void server();

  private:
    RFConf*            m_conf;
    std::string        m_conffile;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;
    RingBuffer*        m_rbufout;

    int m_pid_sampler;
    int m_pid_server = 0;

  };

}
#endif
