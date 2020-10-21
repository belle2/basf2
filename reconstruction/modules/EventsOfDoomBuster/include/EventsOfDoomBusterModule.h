/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

/* Reconstruction headers. */
#include <reconstruction/dbobjects/EventsOfDoomParameters.h>

/* Belle 2 headers. */
#include <cdc/dataobjects/CDCHit.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDShaperDigit.h>

/* C++ headers. */
#include <cstdint>

namespace Belle2 {
  /**
   * Module that flags an event destined for doom at reconstruction,
   * based on the size of selected hits/digits containers after the unpacking.
   * This is meant to be registered in the path *after* the unpacking, but *before* reconstruction.
   */
  class EventsOfDoomBusterModule final : public Module {

  public:
    /// Module constructor.
    EventsOfDoomBusterModule();

    /// Default Destructor.
    virtual ~EventsOfDoomBusterModule() final;

    /// Module initializer.
    void initialize() final;

    /// Called when entering a new run.
    void beginRun() final;

    /**
     * Flag each event.
     * True if event exceeds `nCDCHitsMax or nSVDShaperDigitsMax`.
     * In that case, the event should be skipped for reco.
     */
    void event() final;

  private:
    /**
     * The max number of CDC hits for an event to be kept for reconstruction.
     * By default, no events are skipped based upon this requirement.
     */
    uint32_t m_nCDCHitsMax = 1e9;
    /**
     * The max number of SVD shaper digits for an event to be kept for reconstruction.
     * By default, no events are skipped based upon this requirement.
     */
    uint32_t m_nSVDShaperDigitsMax = 1e9;

    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;

    /// CDCHits StoreArray
    StoreArray<CDCHit> m_cdcHits;

    /// SVDShaperDigits StoreArray
    StoreArray<SVDShaperDigit> m_svdShaperDigits;

    /// EventsOfDoomParameters Database OjbPtr
    DBObjPtr<EventsOfDoomParameters> m_eventsOfDoomParameters;

  };
} // Belle2 namespace
