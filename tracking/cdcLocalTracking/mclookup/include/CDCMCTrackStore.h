/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCTRACKSTORE_H
#define CDCMCTRACKSTORE_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCMap.h>

#include <cdc/dataobjects/CDCHit.h>
#include <map>
#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information
    class CDCMCTrackStore {

    public:
      /// Type for an ordered sequence of pointers to the CDCHit
      typedef std::vector<const CDCHit*> CDCHitVector;

    public:
      /// Default constructor
      CDCMCTrackStore();

      /// Empty destructor
      ~CDCMCTrackStore();

    public:
      /// Getter for the singletone instance
      static CDCMCTrackStore& getInstance();

    public:
      /// Clear all Monte Carlo hits
      void clear();

      /** Fill the store with the tracks from Monte Carlo information.
       *  It uses the CDCMCMap to construct the Monte Carlo tracks.
       */
      void fill(const CDCMCMap* ptrMCMap);


    public:
      /// Getter for the stored Monte Carlo tracks ordered by their Monte Carlo Id
      const std::map<int, Belle2::CDCLocalTracking::CDCMCTrackStore::CDCHitVector>& getMCTracksByMCParticleIdx() const
      { return  m_mcTracksByMCParticleIdx;}

      /// Getter for the stored Monte Carlo segments ordered by their Monte Carlo Id
      const std::map<int, std::vector<Belle2::CDCLocalTracking::CDCMCTrackStore::CDCHitVector> >& getMCSegmentsByMCParticleIdx() const
      { return  m_mcSegmentsByMCParticleIdx;}

    private:

      /// Construct the tracks by grouping the hits by the mc particle id and sorted them for the FlightTime of the related CDCSimHits.
      void fillMCTracks();

      /// Construct the segments by dividing the mc tracks in to disconnected parts and sorted them for the FlightTime of the related CDCSimHits.
      void fillMCSegments();

      /// Sorts the given track for the FlightTime of the assoziated CDCSimHits
      void arrangeMCTrack(CDCHitVector& mcTrack) const;

      /// Fill the look up table for the in track index of each hit
      void fillInTrackId();

      /// Fill the look up table for the in track segment index of each hit
      void fillInTrackSegmentId();

      /// Fill the look up table of the number of traversed super layers until each hit
      void fillNPassedSuperLayers();

    public:
      /// Getter for the index of the hit within its track.
      int getInTrackId(const CDCHit* hit) const;

      /// Getter for the index of the segment of the hit within its track.
      int getInTrackSegmentId(const CDCHit* hit) const;

      /// Getter for the number of super layers traversed until this hit.
      int getNPassedSuperLayers(const CDCHit* hit) const;

    private:
      /// Reference to the MC map of the current event
      const CDCMCMap* m_ptrMCMap;

      /// The memory for the tracks made of CDCHits sorted for the time of flight and assoziated to the Monte Carlo particle id
      std::map<int, CDCHitVector> m_mcTracksByMCParticleIdx;

      /// The memory for the segments made of CDCHits sorted for the time of flight and assoziated to the Monte Carlo particle id
      std::map<int, std::vector<CDCHitVector>> m_mcSegmentsByMCParticleIdx;

      /// Look up table for index of the hit within its track
      std::map<const CDCHit*, int> m_inTrackIds;

      /// Look up table for index of the segment of the hits within their respective tracks
      std::map<const CDCHit*, int> m_inTrackSegmentIds;

      /// Look up table for the number of super layers the particle traversed before making the individual hit
      std::map<const CDCHit*, int> m_nPassedSuperLayers;

    }; //class CDCMCTrackStore
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCTRACKSTORE
