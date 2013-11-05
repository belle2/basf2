#ifndef _B2DQM_TestHistoManager_hh
#define _B2DQM_TestHistoManager_hh

#include <dqm/HistoManager.hh>

#include <dqm/MonLabel.hh>
#include <dqm/TimedGraph1.hh>

namespace B2DQM {

  class TestHistoManager : public HistoManager {

  public:
    TestHistoManager();
    virtual ~TestHistoManager() throw() {};

  public:
    virtual HistoPackage* createPackage(RootHistMap& hist_m);
    virtual RootPanel* createRootPanel(RootHistMap& hist_m);
    virtual void analyze(RootHistMap& hist_m);

  private:
    HistoPackage* pack;
    Histo* h1;
    Histo* h1_2;
    Histo* h2;
    Histo* h3;
    Histo* h3_2;
    Histo* h4;
    MonLabel* label_state;
    MonLabel* label_rate1;
    MonLabel* label_rate1_2;
    MonLabel* label_rate2;
    TimedGraph1* g1;
    TimedGraph1* g1_2;

  };

}

#endif

