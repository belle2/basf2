#ifndef _B2DAQ_HistoManager_hh
#define _B2DAQ_HistoManager_hh

#include <dqm/HistoPackage.hh>
#include <dqm/RootPanel.hh>

namespace B2DQM {

  class HistoManager {
    
  public:
    HistoManager() {}
    virtual ~HistoManager() throw() {}
    
  public:
    virtual HistoPackage* createPackage() = 0;
    virtual RootPanel* createRootPanel() = 0;
    virtual void analyze() {}

  };

}

#endif
