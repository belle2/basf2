/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* HLT headers */
#include <hlt/dbobjects/HLTPrefilterParameters.h>
#include <hlt/softwaretrigger/core/utilities.h>
#include <hlt/utilities/HLTPrefilter.h>

/* BASF2 headers */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>

/* C++ headers */
#include <cstdint>
#include <map>

namespace Belle2 {

  class EventMetaData;
  class HLTPrefilterParameters;
  class TRGSummary;

  /**
   * Prefilter module to suppress the injection background
   */
  class HLTPrefilterModule final : public Module {

  public:
    /// Module constructor.
    HLTPrefilterModule();

    /// Default Destructor.
    virtual ~HLTPrefilterModule() final;

    /// Module initializer.
    void initialize() final;

    /// Called when entering a new run.
    void beginRun() final;

    /**
     * Flag each event.
     * True if event is inside injection background.
     * In that case, the event should be skipped from HLT processing.
     */
    void event() final;

  private:
    // enumeration for HLTPrefilter state
    enum HLTPrefilterState { TimingCut = 0, CDCECLCut = 1 };

    // Helper instance for timing based prefilter
    HLTPrefilter::TimingCutState m_timingPrefilter;

    // Helper instance for CDC-ECL occupancy based prefilter
    HLTPrefilter::CDCECLCutState m_cdceclPrefilter;

    // Decision results
    std::map<HLTPrefilterState, bool> m_decisions;

    // Instance for prefilter cut state
    HLTPrefilterState m_HLTPrefilterState;

    // BASF2 objects
    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

    /// Trigger summary
    StoreObjPtr<TRGSummary> m_trgSummary;

    /// HLTprefilterParameters Database OjbPtr
    DBObjPtr<HLTPrefilterParameters> m_hltPrefilterParameters;

  };
}

