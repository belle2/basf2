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

#include <tracking/trackFindingCDC/typedefs/InfoTypes.h>
#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface class to the Monte Carlo information for segments
    /** This class provides a stable interface for the underlying implementation for look ups
     *  into the Monte Carlo informations for the second stage of the algorithm.*/
    class CDCMCSegmentLookUp : public SwitchableRootificationBase {

    public:
      /// Structure representing a matched Monte Carlo track id with the corresponding purity
      struct MCTrackIdPurityPair : public std::pair<ITrackType, float> {

        /// Constructor taking the Monte Carlo track id and the purity to be stored
        MCTrackIdPurityPair(ITrackType mcTrackId, float purity):
          std::pair<ITrackType, float>(mcTrackId, purity) {;}

        /// Getter for the Monte Carlo track Id
        ITrackType getMCTrackId() const
        { return this->first; }

        /// Getter for the purity
        float getPurity() const
        { return this->second; }

      };

    public:
      /// Empty constructor
      CDCMCSegmentLookUp();

      /// Empty deconstructor
      ~CDCMCSegmentLookUp();

    public:
      /// Getter for the singletone instance
      static const CDCMCSegmentLookUp& getInstance();

      /// Clears all Monte Carlo information left from the last event
      void clear();


    private:
      /// Threshold for the purity of the segments must exceed to be considered as a match.
      static const float s_minimalMatchPurity;

    private:
      /// Fill a map with the number of hits for each track id contained in the given hit range.
      template<class CDCHitPtrRange>
      std::map<ITrackType, size_t> getHitCountByMCTrackId(const CDCHitPtrRange& ptrHits) const
      {

        const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

        std::map<ITrackType, size_t> hitCountByMCTrackId;
        for (const auto& convertableToPtrHit : ptrHits) {
          const CDCHit* ptrHit = convertableToPtrHit;
          ITrackType mcTrackId = mcHitLookUp.getMCTrackId(ptrHit);
          if (hitCountByMCTrackId.count(mcTrackId) == 0) hitCountByMCTrackId[mcTrackId] = 0;
          ++(hitCountByMCTrackId[mcTrackId]);
        }
        return hitCountByMCTrackId;

      }

      /// Get the track id with the highest corresponding purity.
      template<class CDCHitPtrRange>
      MCTrackIdPurityPair getHighestPurity(const CDCHitPtrRange& ptrHits) const
      {

        std::map<ITrackType, size_t>&& hitCountByMCTrackId = getHitCountByMCTrackId(ptrHits);

        size_t nHits = 0;
        std::pair<ITrackType, size_t> highestHitCountMCTrackId(0, 0);

        for (const std::pair<ITrackType, size_t>& hitCountForMCTrackId : hitCountByMCTrackId) {

          nHits += hitCountForMCTrackId.second;

          if (highestHitCountMCTrackId.second < hitCountForMCTrackId.second) {
            highestHitCountMCTrackId = hitCountForMCTrackId;
          }

        }

        return MCTrackIdPurityPair(highestHitCountMCTrackId.first, (float(highestHitCountMCTrackId.second)) / nHits);
      }

      /*

      /// Getter for the CDCSimHit which is related to the CDCHit contained in the given wire hit
      const Belle2::CDCSimHit* getSimHit(const CDCHit* ptrHit) const;

      /// Getter for the MCParticle which is related to the CDCHit contained in the given wire hit
      const Belle2::MCParticle* getMCParticle(const CDCHit* ptrHit) const;

      public:
      /// Getter for the closest simulated hit of a primary particle to the given hit - may return nullptr of no closest is found
      const CDCSimHit* getClosestPrimarySimHit(const CDCHit* ptrHit) const;
      */

    public:
      /// Getter for the Monte Carlo track id matched to this segment
      /** On first encounter of a segment this evaluates the purities for the contained track ids.
       *  The match is valid if the highest purity exceeds the s_minimalMatchPurity threshold.
       *  In case the highest purity is to low to result will be INVALID_ITRACK. To result is stored
       *  in member map object for fast look up for repeated calls.*/
      ITrackType getMCTrackId(const CDCRecoSegment2D* ptrSegment2D) const;

      /// Getter for the mc particle matched to this segment
      const MCParticle* getMCParticle(const CDCRecoSegment2D* ptrSegment2D) const;


      /// Getter for the first hit in the segment which has the Monte Carlo track id matched to this segment
      const CDCHit* getFirstHit(const CDCRecoSegment2D* ptrSegment2D) const;
      /// Getter for the last hit in the segment which has the Monte Carlo track id matched to this segment
      const CDCHit* getLastHit(const CDCRecoSegment2D* ptrSegment2D) const;



      /// Getter for in track id for the first hit in the segment which has the Monte Carlo track id matched to this segment
      Index getFirstInTrackId(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getFirstHit(ptrSegment2D)); }

      /// Getter for in track id for the last hit in the segment which has the Monte Carlo track id matched to this segment
      Index getLastInTrackId(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getLastHit(ptrSegment2D)); }



      /// Getter for in track segment id for the first hit in the segment which has the Monte Carlo track id matched to this segment
      Index getFirstInTrackSegmentId(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getFirstHit(ptrSegment2D)); }

      /// Getter for in track segment id for the last hit in the segment which has the Monte Carlo track id matched to this segment
      Index getLastInTrackSegmentId(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getLastHit(ptrSegment2D)); }



      /// Getter for number of passed superlayer till the first hit the segment which has the Monte Carlo track id matched to this segment
      Index getFirstNPassedSuperLayers(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getFirstHit(ptrSegment2D)); }

      /// Getter for number of passed superlayer till the last hit the segment which has the Monte Carlo track id matched to this segment
      Index getLastNPassedSuperLayers(const CDCRecoSegment2D* ptrSegment2D) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getLastHit(ptrSegment2D)); }



      /** Returns the orientation of the segment relative to its matched track.
       *  * Returns INVALID_INFO if the segment is not matched to anz hit.
       *  * Returns FORWARD if the segment is coaligned with the matched track.
       *  * Returns BACKWARD if the segment is coaligned with the matched track interpreted in reverse.
       *  * Returns INVALID_INFO if the segment has only a single hit.
       */
      ForwardBackwardInfo isForwardOrBackwardToMCTrack(const CDCRecoSegment2D* ptrSegment2D) const;

      /** Returns if the second segments follows the first segment in their common Monte Carlo track
       *  * Returns INVALID_INFO if the segment is not matched in the same Monte Carlo track
       *  * Returns FORWARD if both segments are coaligned with the Monte Carlo track and the second segment comes after the first
       *  * Returns BBACKWARD if both segments are anticoaligned with the Monte Carlo track and the second segment comes after the first in the reversed sense
       */
      ForwardBackwardInfo areAlignedInMCTrack(const CDCRecoSegment2D* ptrStartSegment2D, const CDCRecoSegment2D* ptrendSegment2D) const;



      /** Returns the trajectory of the segment */
      CDCTrajectory3D getTrajectory3D(const CDCRecoSegment2D* ptrSegment2D) const;

    private:
      /// Storage for the map from reconstructed segments to their matched Monte Carlo Track Id
      mutable std::map<const CDCRecoSegment2D*, ITrackType> m_mcTrackIds;



      /// ROOT Macro to make CDCMCSegmentLookUp a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCMCSegmentLookUp, 1);

    }; //class
  } // end namespace TrackFindingCDC
} // namespace Belle2
