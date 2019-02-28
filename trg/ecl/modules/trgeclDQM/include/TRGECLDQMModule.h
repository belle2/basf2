#ifndef TRCECLDQMMODULE_h
#define TRCECLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include <framework/datastore/StoreArray.h>
#include <trg/ecl/TrgEclCluster.h>
#include <stdlib.h>
#include <iostream>


#include <TH1F.h>
#include <TH2F.h>

//using namespace std;

namespace Belle2 {

  class TRGECLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGECLDQMModule();
    //! Destrunctor
    virtual ~TRGECLDQMModule();

  public:
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override;
    //! Define Histogram
    virtual void defineHisto() override;

  private:
    //! TCId histogram
    TH1* h_TCId;
    //! TCthetaId histogram
    TH1* h_TCthetaId;
    //! TCphiId histogram
    TH1* h_TCphiId_BWD;
    //! TCphiId histogram
    TH1* h_TCphiId_BR;
    //! TCphiId histogram
    TH1* h_TCphiId_FWD;
    //! TCphiId histogram
    TH1* h_TCEnergy;
    //! Total  Energy
    TH1* h_TotalEnergy;
    //! N of TC Hit / event
    TH1* h_n_TChit_event;
    //! N of Cluster / event
    TH1* h_Cluster;
    //! TC Timing / event
    TH1* h_TCTiming;
    //! Event Timing / event
    TH1* h_TRGTiming;



    //! Hit TCId
    std::vector<int> TCId;
    //! Hit TC Energy
    std::vector<double> TCEnergy;
    //! Hit TC Timing
    std::vector<double> TCTiming;
    //! FAM Revolution Clk
    std::vector<double> RevoFAM;
    //! Event Timing
    std::vector<double> FineTiming;
    //! GDL Revolution Clk
    std::vector<double> RevoTrg;



    //!
    StoreArray<TRGECLUnpackerStore> trgeclHitArray;
    StoreArray<TRGECLUnpackerEvtStore> trgeclEvtArray;
    StoreArray<TRGECLCluster> trgeclCluster;
  };

}

#endif
