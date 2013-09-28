#ifndef _B2DAQ_HistoFileReader_hh
#define _B2DAQ_HistoFileReader_hh

#include "TH1.h"
#include "TMapFile.h"

#include <map>

namespace B2DQM {

  class HistoPackage;

  typedef std::map<std::string, TH1*> TH1List;

  class HistoFileReader {
    
  public:
    HistoFileReader(const char* file_path);
    ~HistoFileReader();
    
  public:
    bool init(const char* file_path);
    void update(HistoPackage* pack = NULL);
    TH1* getHist(const std::string& name);
    TH1List& getHists() { return _hist_m; }
    bool isReady() const { return _ready; }

  private:
    TMapFile* _file;
    TH1List _hist_m;
    bool _ready;

  };

}

#endif
