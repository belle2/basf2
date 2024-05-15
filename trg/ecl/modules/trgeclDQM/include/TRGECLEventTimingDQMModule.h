/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <trg/ecl/dataobjects/TRGECLUnpackerStore.h>
#include <trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h>
#include <trg/ecl/TrgEclMapping.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventT0.h>
#include <TH1F.h>
#include <TDirectory.h>
#include <iostream>
#include <iomanip>
#include <string>
namespace Belle2 {
  //!
  //! This module is for ecl trigger DQM related to L1 event timing
  //!
  class TRGECLEventTimingDQMModule : public HistoModule {
  public:
    /** Default constructor */
    TRGECLEventTimingDQMModule();
    /** Destructor */
    virtual ~TRGECLEventTimingDQMModule();
    /** Defination of histograms */
    virtual void defineHisto() override;
    /** Initialize the module */
    virtual void initialize() override;
    /** This method is called for each run */
    virtual void beginRun() override;
    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

  private:

    //! Trigger Summary data object
    StoreObjPtr<TRGSummary> m_objTrgSummary;
    //! EventT0 data object
    StoreObjPtr<EventT0> m_eventT0;
    //! Trg ECL UnpakcerStore
    StoreArray<TRGECLUnpackerStore> m_trgeclUnpackerStores;
    //! Trg ECL UnpakcerEvtStore
    StoreArray<TRGECLUnpackerEvtStore> m_trgeclUnpackerEvtStores;
    //! ecl trigger map
    TrgEclMapping* trgecl_map;
    //! Max TC E
    TH1F* m_histMaxTCE{nullptr};
    //! Max TC ID
    TH1F* m_histMaxTCId{nullptr};
    //! Max TC theta ID
    TH1F* m_histMaxTCThetaId{nullptr};
    //! Event Timing Quality from TRGSummary
    TH1F* m_histEventTimingQuality{nullptr};
    //! EventT0 with coarse event timing
    TH1F* m_histEventT0Coarse{nullptr};
    //! EventT0 with fine event timing
    TH1F* m_histEventT0Fine{nullptr};
    //! The number of EventT0 histogram for different max TC E region
    static const int NBinEventT0 = 15;
    //! EventT0 for different max TC E region
    TH1F* m_histEventT0[NBinEventT0] {nullptr};

  };
} // Belle2 namespace
