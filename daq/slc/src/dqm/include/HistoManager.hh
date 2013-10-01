#ifndef _B2DAQ_HistoManager_hh
#define _B2DAQ_HistoManager_hh

#include <dqm/HistoPackage.hh>
#include <dqm/RootPanel.hh>

#include <map>

class TH1;

namespace B2DQM {

  typedef std::map<std::string, TH1*> RootHistMap;

  class HistoManager {
    
  public:
    HistoManager() {}
    virtual ~HistoManager() throw() {}
    
  public:
    virtual HistoPackage* createPackage(RootHistMap& hito_m) = 0;
    virtual RootPanel* createRootPanel(RootHistMap& hito_m) = 0;
    virtual void analyze(RootHistMap& hito_m) {}

  };

}

#endif
