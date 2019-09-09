/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/dataobjects/EventMetaData.h>
#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

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

    /// Module initializer
    void initialize() final;

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
    unsigned int m_nCDCHitsMax = 1e9;
    /**
     * The max number of SVD shaper digits for an event to be kept for reconstruction.
     * By default, no events are skipped based upon this requirement.
     */
    unsigned int m_nSVDShaperDigitsMax = 1e9;

    /// Event Meta Data Store ObjPtr
    StoreObjPtr<EventMetaData> m_eventInfo;
    /// CDCHits StoreArray
    StoreArray<CDCHit> m_cdcHits;
    /// SVDShaperDigits StoreArray
    StoreArray<SVDShaperDigit> m_svdShaperDigits;
  };
} // Belle2 namespace
