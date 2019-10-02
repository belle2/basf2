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

  //! DQM module for TRGCDCT3D
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
    //! dz of T3D in each module
    TH1D* h_dz = nullptr;
    //! phi of T3D in each module
    TH1D* h_phi = nullptr;
    //! tanlambda of T3D in each module
    TH1D* h_tanlambda = nullptr;
    //! pt of T3D in each module
    TH1D* h_pt = nullptr;
    //! phi (from 2D) of T3D in each module
    TH1D* h_phi_2D = nullptr;
    //! pt (from 2D) of T3D in each module
    TH1D* h_pt_2D = nullptr;


    //! TDirectories for DQM histograms
    TDirectory* oldDir = nullptr;
    //! TDirectories for DQM histograms
    TDirectory* dirDQM = nullptr;

    /// flag to save ps file
    bool m_generatePostscript;

    /// name of ps file
    std::string m_postScriptName;

    /// experiment number
    unsigned _exp = 0;

    /// run number
    unsigned _run = 0;

    /// T3D module number
    int m_T3DMOD = 0;

    /// 3D data store
    StoreArray<CDCTriggerTrack> entAry;

    /// 2D data store
    StoreArray<CDCTriggerTrack> entAry_2D;
  };

}

#endif
