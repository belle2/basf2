#ifndef _B2DQM_DefaultHistoManager_hh
#define _B2DQM_DefaultHistoManager_hh

#include <dqm/HistoManager.hh>

#include <dqm/MonLabel.hh>
#include <dqm/TimedGraph1.hh>

namespace B2DQM {

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

