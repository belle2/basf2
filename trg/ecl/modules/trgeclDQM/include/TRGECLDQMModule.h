#ifndef TRCECLDQMMODULE_h
#define TRCECLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include <framework/datastore/StoreArray.h>
#include <trg/ecl/TrgEclCluster.h>
#include <stdlib.h>
#include <iostream>


#include <TH1F.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  class TRGECLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGECLDQMModule();
    //! Destrunctor
    virtual ~TRGECLDQMModule();

  public:
    //! initialize
    virtual void initialize();
    //! begin Run
    virtual void beginRun();
    //! Event
    virtual void event();
    //! End Run
    virtual void endRun();
    //! terminate
    virtual void terminate();
    //! Define Histogram
    virtual void defineHisto();

  private:
    /** Object of TC Clustering */
    TrgEclCluster* _TCCluster;
    //! TCId histogram
    TH2* h_TC2d;
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

  };

}

#endif
