/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCMANAGER_H
#define CDCMCMANAGER_H

#include "CDCMCMap.h"
#include "CDCMCTrackStore.h"
#include "CDCSimHitLookUp.h"

#include "CDCMCHitLookUp.h"
#include "CDCMCSegmentLookUp.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information

    class CDCMCManager : public UsedTObject {

    public:
      /// Empty constructor
      CDCMCManager();

      /// Empty deconstructor
      ~CDCMCManager();

    public:
      ///Getter for the singletone instance
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


    public:
      /// Clears all Monte Carlo information present in the last event
      void clear();

      /// Fill Monte Carlo look up maps from the DataStore
      void fill();

    private:
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

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCHITLOOKUP
