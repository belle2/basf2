#ifndef _Belle2_HistoFileReader_hh
#define _Belle2_HistoFileReader_hh

#include <dqm/HistoManager.h>

#include "TH1.h"
#include "TMapFile.h"

#include <map>

namespace Belle2 {

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
