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

#include <tracking/trackFindingCDC/numerics/InfoTypes.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface class to the Monte Carlo information for collections of hits
    template<class CDCHitCollection>
    class CDCMCHitCollectionLookUp  {

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
      /// Clears all Monte Carlo information left from the last event
      void clear();

    private:
      /// Threshold for the purity of the segments must exceed to be considered as a match.
      static const float s_minimalMatchPurity;

    private:
      /// Fill a map with the number of hits for each track id contained in the given hit range.
      std::map<ITrackType, size_t> getHitCountByMCTrackId(const CDCHitCollection& hits) const
      {
        const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

        std::map<ITrackType, size_t> hitCountByMCTrackId;
        for (const CDCHit* ptrHit : hits) {
          ITrackType mcTrackId = mcHitLookUp.getMCTrackId(ptrHit);
          if (hitCountByMCTrackId.count(mcTrackId) == 0) hitCountByMCTrackId[mcTrackId] = 0;
          ++(hitCountByMCTrackId[mcTrackId]);
        }
        return hitCountByMCTrackId;
      }

      /// Get the track id with the highest corresponding purity.
      MCTrackIdPurityPair getHighestPurity(const CDCHitCollection& hits) const
      {
        std::map<ITrackType, size_t> hitCountByMCTrackId = getHitCountByMCTrackId(hits);

        size_t nHits = 0;
        std::pair<ITrackType, size_t> highestHitCountMCTrackId(0, 0);

        for (const std::pair<ITrackType, size_t>& hitCountForMCTrackId : hitCountByMCTrackId) {

          nHits += hitCountForMCTrackId.second;

          if (highestHitCountMCTrackId.second < hitCountForMCTrackId.second) {
            highestHitCountMCTrackId = hitCountForMCTrackId;
          }

        }

        const float purity = float(highestHitCountMCTrackId.second) / nHits;
        return MCTrackIdPurityPair(highestHitCountMCTrackId.first, purity);
      }

    public:
      /// Getter for the Monte Carlo track id matched to this collection of hits
      /** On first encounter of a collection of hits this evaluates the purities for the contained track ids.
       *  The match is valid if the highest purity exceeds the s_minimalMatchPurity threshold.
       *  In case the highest purity is to low to result will be INVALID_ITRACK. To result is stored
       *  in member map object for fast look up for repeated calls.*/
      ITrackType getMCTrackId(const CDCHitCollection* ptrHits) const;

      /// Getter for the mc particle matched to this collection of hits
      const MCParticle* getMCParticle(const CDCHitCollection* ptrHits) const;


      /// Getter for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      const CDCHit* getFirstHit(const CDCHitCollection* ptrHits) const;

      /// Getter for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      const CDCHit* getLastHit(const CDCHitCollection* ptrHits) const;



      /// Getter for in track id for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstInTrackId(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getFirstHit(ptrHits)); }

      /// Getter for in track id for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastInTrackId(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getLastHit(ptrHits)); }



      /// Getter for in track collection of hits id for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstInTrackSegmentId(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getFirstHit(ptrHits)); }

      /// Getter for in track collection of hits id for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastInTrackSegmentId(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getLastHit(ptrHits)); }



      /// Getter for number of passed superlayer till the first hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstNPassedSuperLayers(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getFirstHit(ptrHits)); }

      /// Getter for number of passed superlayer till the last hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastNPassedSuperLayers(const CDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getLastHit(ptrHits)); }



      /** Returns the orientation of the collection of hits relative to its matched track.
       *  * Returns INVALID_INFO if the collection of hits is not matched to anz hit.
       *  * Returns FORWARD if the collection of hits is coaligned with the matched track.
       *  * Returns BACKWARD if the collection of hits is coaligned with the matched track interpreted in reverse.
       *  * Returns INVALID_INFO if the collection of hits has only a single hit.
       */
      ForwardBackwardInfo isForwardOrBackwardToMCTrack(const CDCHitCollection* ptrHits) const;

      /** Returns if the second collection of hits
       *  follows the first collection of hits in their common Monte Carlo track.
       *  * Returns INVALID_INFO if the collection of hits is not matched in the same Monte Carlo track.
       *  * Returns FORWARD  if both collections of hits are coaligned
       *                     with the Monte Carlo track and the second collection of hits
       *                     comes after the first.
       *  * Returns BACKWARD if both collections of hits are anticoaligned
       *                     with the Monte Carlo track and the second collection of hits
       *                     comes after the first in the reversed sense.
       */
      ForwardBackwardInfo areAlignedInMCTrack(const CDCHitCollection* ptrFromHits,
                                              const CDCHitCollection* ptrToHits) const;



      /** Returns the trajectory of the collection of hits */
      CDCTrajectory3D getTrajectory3D(const CDCHitCollection* ptrHits) const;

    private:
      /// Storage for the map from reconstructed collections of hits to their matched Monte Carlo Track Id
      mutable std::map<const CDCHitCollection*, ITrackType> m_mcTrackIds;


    }; //class




  } // end namespace TrackFindingCDC
} // namespace Belle2
