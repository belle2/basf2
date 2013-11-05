#ifndef _B2DQM_CDC_TESTHistoManager_hh
#define _B2DQM_CDC_TESTHistoManager_hh

#include <dqm/HistoManager.hh>
#include <dqm/MonLabel.hh>
#include <dqm/TimedGraph1.hh>

namespace B2DQM {

  class CDC_TESTHistoManager : public HistoManager {

  public:
    CDC_TESTHistoManager();
    virtual ~CDC_TESTHistoManager() throw() {};

  public:
    virtual HistoPackage* createPackage(RootHistMap& hist_m);
    virtual RootPanel* createRootPanel(RootHistMap& hist_m);
    virtual void analyze(RootHistMap& hist_m);

  private:
    HistoPackage* _pack;
    Histo* _h_ncpr;
    Histo* _h_nevt;
    Histo* _h_size;
    Histo* _h_size2d;
    Histo* _h_ncpr_cpy;
    Histo* _h_nevt_cpy;
    Histo* _h_size_cpy;
    MonLabel* _label_state;
    MonLabel* _label_nevt;
    MonLabel* _label_nevt_rate;
    TimedGraph1* _gr_nevt;
    double _time;
  };

}

#endif

