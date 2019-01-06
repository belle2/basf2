#ifndef TRCCDCT3DDQMMODULE_h
#define TRCCDCT3DDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/cdc/dataobjects/TRGCDCT3DUnpackerStore.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/modules/trgcdct3dUnpacker/TRGCDCT3DUnpackerModule.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <TH2I.h>
#include <TH1I.h>

namespace Belle2 {

  class TRGCDCT3DDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGCDCT3DDQMModule();
    //! Destrunctor
    virtual ~TRGCDCT3DDQMModule() {}

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
    virtual void terminate() {}
    //! Define Histogram
    virtual void defineHisto();

  protected:
    //dz of T3D in each module
    TH1D* h_dz;
    TH1D* h_phi;
    TH1D* h_tanlambda;
    TH1D* h_pt;
    TH1D* h_phi_2D;
    TH1D* h_pt_2D;


    //TDirectories
    TDirectory* oldDir;
    TDirectory* dirDQM;

    //flag to save ps file
    bool m_generatePostscript;

    //name of ps file
    std::string m_postScriptName;

    //experiment number
    unsigned _exp;

    //run number
    unsigned _run;

    //T3D module number
    int m_T3DMOD;

    //3D data store
    StoreArray<CDCTriggerTrack> entAry;

    //2D data store
    StoreArray<CDCTriggerTrack> entAry_2D;
  };

}

#endif
