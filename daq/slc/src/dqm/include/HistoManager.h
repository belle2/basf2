#ifndef _Belle2_HistoManager_hh
#define _Belle2_HistoManager_hh

#include "dqm/HistoPackage.h"
#include "dqm/RootPanel.h"

#include <map>

class TH1;

namespace Belle2 {

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
