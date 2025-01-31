/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_EventServerCallback_hh
#define _Belle2_EventServerCallback_hh

#include <pthread.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFSharedMem.h>
#include <daq/rfarm/manager/RFLogManager.h>

namespace Belle2 {

  class EventServerCallback : public RCCallback {

  public:
    EventServerCallback();
    virtual ~EventServerCallback() noexcept;

  public:
    virtual void load(const DBObject&, const std::string&) override;
    void abort() override;

    void EventServerLogger();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    RFLogManager* m_log;
    int m_pid_server;
    pthread_t m_logthread;

  };

}

#endif
