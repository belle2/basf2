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

#include <tracking/trackFindingCDC/mclookup/ITrackType.h>
#include <tracking/trackFindingCDC/numerics/Index.h>

#include <map>
#include <vector>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCMCMap;
    class CDCSimHitLookUp;

    ///Class to organize and present the monte carlo hit information
    class CDCMCTrackStore {

    public:
      /// Type for an ordered sequence of pointers to the CDCHit
      using CDCHitVector = std::vector<const CDCHit*>;

    public:
      /// Getter for the singletone instance
      static const CDCMCTrackStore& getInstance();

    public:
      /// Default constructor - for cppcheck
      CDCMCTrackStore() = default;

    public:
      /// Clear all Monte Carlo hits
      void clear();

      /** Fill the store with the tracks from Monte Carlo information.
       *  It uses the CDCMCMap to construct the Monte Carlo tracks.
       */
      void fill(const CDCMCMap* ptrMCMap, const CDCSimHitLookUp* ptrSimHitLookUp);

    public:
      /// Getter for the stored Monte Carlo tracks ordered by their Monte Carlo Id
      const std::map<ITrackType, Belle2::TrackFindingCDC::CDCMCTrackStore::CDCHitVector>& getMCTracksByMCParticleIdx() const
      { return  m_mcTracksByMCParticleIdx;}

      /// Getter for the stored Monte Carlo segments ordered by their Monte Carlo Id
      const std::map<ITrackType, std::vector<Belle2::TrackFindingCDC::CDCMCTrackStore::CDCHitVector> >& getMCSegmentsByMCParticleIdx()
      const
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
      void fillNLoopsAndNPassedSuperLayers();

      /// Helper function to decide whether the number of passed superlayers changed from one segment to the next
      /** The number of passed superlayers is easily detectable in a switch of the superlayer number form one segment to the next.
       *  An exception to this rule happens were particles are curling back exiting the super layer 0 and reentering the CDC super layer 0.
       *  There is no change of super layer number in this case since consecutive segments (only looking at the CDC) have the super layer number 0.
       *  Since this case would screw the evaluation of segment pair or triple selection we have to treat it seperatly.
       */
      bool changedSuperLayer(const CDCHitVector& mcSegment, const CDCHitVector& nextMCSegment) const;

    public:
      /// Getter for the index of the hit within its track.
      Index getInTrackId(const CDCHit* ptrHit) const;

      /// Getter for the index of the segment of the hit within its track.
      Index getInTrackSegmentId(const CDCHit* ptrHit) const;

      /// Getter for the number of super layers traversed until this hit.
      Index getNPassedSuperLayers(const CDCHit* ptrHit) const;

      /// Getter for the number of traversed loops until this hit.
      Index getNLoops(const CDCHit* ptrHit) const;

    private:
      /// Reference to the MC map of the current event
      const CDCMCMap* m_ptrMCMap;

      /// Reference to the CDCSimHit look up for additional information about related primary sim hits
      const CDCSimHitLookUp* m_ptrSimHitLookUp;

      /// The memory for the tracks made of CDCHits sorted for the time of flight and assoziated to the Monte Carlo particle id
      std::map<ITrackType, CDCHitVector> m_mcTracksByMCParticleIdx;

      /// The memory for the segments made of CDCHits sorted for the time of flight and assoziated to the Monte Carlo particle id
      std::map<ITrackType, std::vector<CDCHitVector> > m_mcSegmentsByMCParticleIdx;

      /// Look up table for index of the hit within its track
      std::map<const CDCHit*, int> m_inTrackIds;

      /// Look up table for index of the segment of the hits within their respective tracks
      std::map<const CDCHit*, int> m_inTrackSegmentIds;

      /// Look up table for the number of super layers the particle traversed before making the individual hit
      std::map<const CDCHit*, int> m_nPassedSuperLayers;

      /// Look up table for the number of loops the particle traversed before making the individual hit
      std::map<const CDCHit*, int> m_nLoops;
    };
  }
}
