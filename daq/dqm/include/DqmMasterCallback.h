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

namespace Belle2 {

  class DqmMasterCallback : public RCCallback {

  public:
    explicit DqmMasterCallback(ConfigFile&);
    virtual ~DqmMasterCallback() override;
    /** Copy constructor (disabled). */
    DqmMasterCallback(const DqmMasterCallback&) = delete;

    /** Operator = (disabled). */
    DqmMasterCallback& operator=(const DqmMasterCallback&) = delete;

  public:
    void load(const DBObject& obj, const std::string& runtype) override final;
    void start(int, int) override final;
    void stop(void) override final;
    void abort(void) override final;

    static int m_running;
  private:
    int m_expno = 0;
    int m_runno = 0;
    std::string m_runtype;
    std::string m_histodir;
    std::string m_tmpdir;
    std::string m_instance;
    EvtSocketSend* m_sock;
  };

}
