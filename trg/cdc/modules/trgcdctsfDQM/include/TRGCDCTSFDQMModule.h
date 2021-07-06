/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRCCDCTSFDQMMODULE_h
#define TRCCDCTSFDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/TRGCDCTSFUnpackerStore.h>
#include <string>

#include <TH1I.h>

namespace Belle2 {

  //! DQM module of TRGCDCTSF
  class TRGCDCTSFDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGCDCTSFDQMModule();
    //! Destrunctor
    virtual ~TRGCDCTSFDQMModule() {}

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
    //! Total number of TSF hits per event in each superlayer
    TH1I* h_nhit = nullptr;

    //! Total number of hits in each TSF
    TH1I* h_nhit_tsf = nullptr;

    //! Valid type of TSF hits in each superlayer
    TH1I* h_valid = nullptr;

    //! Timing of TSF hits in each superlayer
    TH1I* h_timing = nullptr;

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

    //! TSF module number
    int m_TSFMOD = 0;

    //! TSF data store
    StoreArray<TRGCDCTSFUnpackerStore> entAry;

  };

}

#endif
