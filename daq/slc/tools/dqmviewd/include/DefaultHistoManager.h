#ifndef _Belle2_DefaultHistoManager_hh
#define _Belle2_DefaultHistoManager_hh

#include <dqm/HistoManager.h>

#include <dqm/MonLabel.h>
#include <dqm/TimedGraph1.h>

namespace Belle2 {

  class DefaultHistoManager : public HistoManager {

  public:
    DefaultHistoManager(const std::string& name);
    virtual ~DefaultHistoManager() throw() {};

  public:
    virtual HistoPackage* createPackage(RootHistMap& hito_m);
    virtual RootPanel* createRootPanel(RootHistMap& hito_m);

  private:
    std::string _name;
    HistoPackage* _pack;

  };

}

#endif

