#ifndef Belle2_DAQDummyGaus_h
#define Belle2_DAQDummyGaus_h

#include <framework/core/HistoModule.h>

#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  class DAQDummyGausModule : public HistoModule {

  public:

    DAQDummyGausModule();
    virtual ~DAQDummyGausModule();

  public:
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();
    virtual void defineHisto();

  private:
    TH1* h_Gaus[5];
    TH1* h_GausSum;
    TH1* h_Gaus2D;

  };

}

#endif
