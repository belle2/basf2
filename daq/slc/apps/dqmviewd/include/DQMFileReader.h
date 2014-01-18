#ifndef _Belle2_DQMFileReader_hh
#define _Belle2_DQMFileReader_hh

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/dqm/DQMHistMap.h>

#include <TH1.h>
#include <TMapFile.h>

#include <map>

namespace Belle2 {

  class HistoPackage;

  class DQMFileReader {

  public:
    DQMFileReader(const std::string& name);
    ~DQMFileReader();

  public:
    bool init(const char* file_path);
    void update(HistoPackage* pack);
    TH1* getHist(const std::string& name);
    DQMHistMap* getHistMap() { return _hist_m; }
    bool isReady() const { return _ready; }
    bool dump(const std::string& dir,
              unsigned int expno, unsigned int runno);

  private:
    std::string _name;
    TMapFile* _file;
    DQMHistMap* _hist_m;
    bool _ready;

  };

}

#endif
