#ifndef _B2DAQ_HistoFileReader_hh
#define _B2DAQ_HistoFileReader_hh

#include <dqm/HistoManager.hh>

#include "TH1.h"
#include "TMapFile.h"

#include <map>

namespace B2DQM {

  class HistoPackage;

  class HistoFileReader {
    
  public:
    HistoFileReader(const char* file_path);
    ~HistoFileReader();
    
  public:
    bool init(const char* file_path);
    void update(HistoPackage* pack = NULL);
    TH1* getHist(const std::string& name);
    RootHistMap& getHists() { return _hist_m; }
    bool isReady() const { return _ready; }

  private:
    TMapFile* _file;
    RootHistMap _hist_m;
    bool _ready;

  };

}

#endif
