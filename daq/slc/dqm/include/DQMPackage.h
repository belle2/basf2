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
      : MonitorPackage(name), _file_name(file_name) {}
    virtual ~DQMPackage() throw() {}

  public:
    const std::string& getFileName() { return _file_name; }
    void setHistMap(DQMHistMap* hist_m);
    DQMHistMap* getHistMap() { return _hist_m; }

  private:
    Histo* makeHisto(TH1* h) throw();
    MonColor makeColor(int num) throw();

  private:
    std::string _file_name;
    DQMHistMap* _hist_m;

  };

}

#endif


