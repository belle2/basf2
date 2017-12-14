#ifndef TRCECLDQMMODULE_h
#define TRCECLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
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
    //! TCId histogram
    TH1* h_TCId;
    //! Total  Energy
    TH1* h_TotalEnergy;
    //! Hit TCId
    std::vector<int> TCId;
    //! Hit TC Energy
    std::vector<double> TCEnergy;
    //! Hit TC Timing
    std::vector<double> TCTiming;

    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore; /**< output for TRGECLUnpackerStore */

  };

}

#endif
