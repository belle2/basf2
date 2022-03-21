/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCMCTrackLookUp;
    class CDCMCSegment3DLookUp;
    class CDCMCSegment2DLookUp;
    class CDCMCHitLookUp;

    class CDCSimHitLookUp;
    class CDCMCTrackStore;
    class CDCMCMap;


    ///Class to organize and present the monte carlo hit information
    class CDCMCManager  {

    public:
      /// Getter for the singletone instance
      static CDCMCManager& getInstance();

      /// Singleton: Delete copy constructor and assignment operator
      CDCMCManager(CDCMCManager&) = delete;

      /// Operator =.
      CDCMCManager& operator=(const CDCMCManager&) = delete;

      /// Getter for the singletone instance of the CDCMCMap
      static const CDCMCMap& getMCMap();

      /// Getter for the singletone instance of the CDCMCTrackStore
      static const CDCMCTrackStore& getMCTrackStore();

      /// Getter for the singletone instance of the CDCSimHitLookUp
      static const CDCSimHitLookUp& getSimHitLookUp();

      /// Getter for the singletone instance of the CDCMCHitLookUp
      static const CDCMCHitLookUp& getMCHitLookUp();

      /// Getter for the singletone instance of the CDCMCSegment2DLookUp
      static const CDCMCSegment2DLookUp& getMCSegment2DLookUp();

      /// Getter for the singletone instance of the CDCMCSegment3DLookUp
      static const CDCMCSegment3DLookUp& getMCSegment3DLookUp();

      /// Getter for the singletone instance of the CDCMCTrackLookUp
      static const CDCMCTrackLookUp& getMCTrackLookUp();

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
    };
  }
}
