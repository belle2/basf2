/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/base/ConfigFile.h>

#include <daq/dataflow/EvtSocket.h>

#include "daq/dqm/DqmMemFile.h"

namespace Belle2 {

  class DqmMasterCallback : public RCCallback {

  public:
    DqmMasterCallback(ConfigFile&);
    virtual ~DqmMasterCallback();

  public:
    virtual void load(const DBObject& obj, const std::string& runtype);
    virtual void start(int, int);
    virtual void stop();
    virtual void abort();

  private:
    int m_expno;
    int m_runno;
    std::string m_runtype;
    DqmMemFile* m_hltdqm;
    DqmMemFile* m_erecodqm;
    std::string m_hltdir;
    std::string m_erecodir;
    //    static int m_running;
    EvtSocketSend* m_sock_hlt;
    EvtSocketSend* m_sock_reco;
  };

}
