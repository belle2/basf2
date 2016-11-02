/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Class to organize and present the monte carlo hit information

    class CDCMCManager  {

    public:
      /// Getter for the singletone instance
      static CDCMCManager& getInstance();

      /// Getter for the singletone instance of the CDCMCMap
      static const CDCMCMap& getMCMap()
      { return getInstance().m_mcMap; }

      /// Getter for the singletone instance of the CDCMCTrackStore
      static const CDCMCTrackStore& getMCTrackStore()
      { return getInstance().m_mcTrackStore; }

      /// Getter for the singletone instance of the CDCSimHitLookUp
      static const CDCSimHitLookUp& getSimHitLookUp()
      { return getInstance().m_simHitLookUp; }

      /// Getter for the singletone instance of the CDCMCHitLookUp
      static const CDCMCHitLookUp& getMCHitLookUp()
      { return getInstance().m_mcHitLookUp; }

      /// Getter for the singletone instance of the CDCMCSegmentLookUp
      static const CDCMCSegmentLookUp& getMCSegmentLookUp()
      { return getInstance().m_mcSegmentLookUp; }

      /// Getter for the singletone instance of the CDCMCTrackLookUp
      static const CDCMCTrackLookUp& getMCTrackLookUp()
      { return getInstance().m_mcTrackLookUp; }

    private:
      /// Default constructor only accessable for getInstance()
      CDCMCManager() = default;

    public:
      /// Require the mc information store arrays.
      void requireTruthInformation();

      /// Clears all Monte Carlo information present in the last event
      void clear();

      /// Fill Monte Carlo look up maps from the DataStore
      void fill();

    private:
      /// Memory for the event, run and experminent number corresponding to the currently stored data.
      EventMetaData m_eventMetaData = EventMetaData(-999, -999, -999);

      /// Memory for the singletone CDCMCMap
      CDCMCMap m_mcMap;

      /// Memory for the singletone CDCMCTrackStore
      CDCMCTrackStore m_mcTrackStore;

      /// Memory for the singletone CDCSimHitLookUp
      CDCSimHitLookUp m_simHitLookUp;

      /// Memory for the singletone CDCMCHitLookUp
      CDCMCHitLookUp m_mcHitLookUp;

      /// Memory for the singletone CDCMCSegmentLookUp
      CDCMCSegmentLookUp m_mcSegmentLookUp;

      /// Memory for the singletone CDCMCTrackLookUp
      CDCMCTrackLookUp m_mcTrackLookUp;

    };
  }
}
