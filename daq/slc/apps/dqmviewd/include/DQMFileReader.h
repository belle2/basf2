#ifndef _Belle2_DQMFileReader_hh
#define _Belle2_DQMFileReader_hh

#include <daq/slc/dqm/DQMHistMap.h>

#include <TH1.h>
#include <TMapFile.h>

#include <map>

namespace Belle2 {

  class HistoPackage;

  class DQMFileReader {

  public:
    DQMFileReader(const char* file_path);
    ~DQMFileReader();

  public:
    bool init(const char* file_path);
    void update(HistoPackage* pack = NULL);
    TH1* getHist(const std::string& name);
    DQMHistMap& getHists() { return _hist_m; }
    bool isReady() const { return _ready; }

  private:
    TMapFile* _file;
    DQMHistMap _hist_m;
    bool _ready;

  };

}

#endif
