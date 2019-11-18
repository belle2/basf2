#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include "daq/dqm/DqmMemFile.h"
#include "TH1.h"
#include "TFile.h"
#include "TMemFile.h"

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
    DqmMemFile* m_hltdqm;
    DqmMemFile* m_erecodqm;
    std::string m_hltdir;
    std::string m_erecodir;
    //    static int m_running;
  };

}
