#ifndef _Belle2_DQMFileReader_hh
#define _Belle2_DQMFileReader_hh

#include <daq/slc/dqm/DQMHistMap.h>
#include <daq/slc/dqm/MonColor.h>
#include <daq/slc/dqm/Histo.h>

#include <TH1.h>
#include <TMapFile.h>

#include <map>

namespace Belle2 {

  class HistoPackage;

  class DQMFileReader {

  public:
    DQMFileReader();
    ~DQMFileReader();

  public:
    bool init(const char* file_path, HistoPackage* pack);
    void update(HistoPackage* pack = NULL);
    TH1* getHist(const std::string& name);
    DQMHistMap& getHists() { return _hist_m; }
    bool isReady() const { return _ready; }

  private:
    Histo* makeHisto(TH1* h) throw();
    MonColor makeColor(int num) throw();

  private:
    TMapFile* _file;
    DQMHistMap _hist_m;
    bool _ready;

  };

}

#endif
