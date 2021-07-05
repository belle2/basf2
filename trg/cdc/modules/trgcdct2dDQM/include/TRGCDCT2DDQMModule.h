/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRCCDCT2DDQMMODULE_h
#define TRCCDCT2DDQMMODULE_h

#include <framework/datastore/StoreArray.h>
#include <framework/core/HistoModule.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <string>

#include <TH1I.h>
#include <TH1D.h>

namespace Belle2 {

  //! DQM module of TRGCDCT2D
  class TRGCDCT2DDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGCDCT2DDQMModule();
    //! Destrunctor
    virtual ~TRGCDCT2DDQMModule() {}

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
    virtual void terminate() override {}
    //! Define Histogram
    virtual void defineHisto() override;

  protected:
    //! TSF hit distribution as a function of tsfid
    TH1I* h_tsfhit = nullptr;

    //! 2D phi distribution
    TH1D* h_phi = nullptr;

    //! 2D pt distribution
    TH1D* h_pt = nullptr;

    //! 2D omega distribution
    TH1D* h_omega = nullptr;

    //! 2D foundtime distribution
    TH1D* h_time = nullptr;

    //! TDirectories
    TDirectory* oldDir = nullptr;

    //! TDirectories
    TDirectory* dirDQM = nullptr;

    //! flag to save ps file
    bool m_generatePostscript;

    //! name of ps file
    std::string m_postScriptName;

    //! experiment number
    unsigned _exp = 0;

    //! run number
    unsigned _run = 0;

    //! TSF data store
    StoreArray<CDCTriggerSegmentHit> entAry_tsf;

    //! T2D data store
    StoreArray<CDCTriggerTrack> entAry_t2d;

  };

}

#endif
