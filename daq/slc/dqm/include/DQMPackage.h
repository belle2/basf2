#ifndef _Belle2_DQMPackage_h
#define _Belle2_DQMPackage_h

#include "daq/slc/dqm/MonitorPackage.h"
#include "daq/slc/dqm/DQMHistMap.h"
#include <daq/slc/dqm/MonColor.h>
#include <daq/slc/dqm/Histo.h>

#include <string>
#include <map>

namespace Belle2 {

  class DQMPackage : public MonitorPackage {

  public:
    DQMPackage(const std::string& name,
               const std::string file_name)
      : MonitorPackage(name), m_file_name(file_name) {}
    virtual ~DQMPackage() throw() {}

  public:
    const std::string& getFileName() { return m_file_name; }
    void setHistMap(DQMHistMap* hist_m);
    DQMHistMap* getHistMap() { return m_hist_m; }

  private:
    Histo* makeHisto(TH1* h) throw();
    MonColor makeColor(int num) throw();

  private:
    std::string m_file_name;
    DQMHistMap* m_hist_m;

  };

}

#define REGISTER_DQM_PACKAGE(classname) \
  extern "C" void* create##classname(const char* name, const char* filename)\
  {\
    return new Belle2::classname(name, filename);\
  }

#endif


