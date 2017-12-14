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

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/Index.h>

#include <map>

namespace Belle2 {
  class MCParticle;
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCTrajectory3D;

    /// Structure representing a matched Monte Carlo track id with the corresponding purity
    struct MCTrackIdPurityPair {

      /// Constructor taking the Monte Carlo track id and the purity to be stored
      MCTrackIdPurityPair(ITrackType mcTrackId, float purity, int correctRLVote)
        : m_mcTrackId(mcTrackId)
        , m_purity(purity)
        , m_correctRLVote(correctRLVote)
      {
      }

      /// Getter for the Monte Carlo track Id
      ITrackType getMCTrackId() const
      {
        return m_mcTrackId;
      }

      /// Getter for the purity
      float getPurity() const
      {
        return m_purity;
      }

      /// Getter for the rl vote
      int getCorrectRLVote() const
      {
        return m_correctRLVote;
      }

    private:
      // Id of the mc track
      ITrackType m_mcTrackId;

      // Purity wrt mc track
      float m_purity;

      // Number of correct rl information in the track
      int m_correctRLVote;
    };

    /// Interface class to the Monte Carlo information for collections of hits
    template <typename ACDCHitCollection>
    class CDCMCHitCollectionLookUp  {

    public:
      /// Provide a user-defined constructor because the Intel compiler complains (wrongfully)
      /// that a const-instance of a class w/o user-defined ctor is useless. But in C++11 we have
      /// per-member initialization ...
      CDCMCHitCollectionLookUp() = default;

      /// Clears all Monte Carlo information left from the last event
      void clear();

    private:
      /// Fill a map with the number of hits for each track id contained in the given hit range.
      std::map<ITrackType, size_t> getHitCountByMCTrackId(const ACDCHitCollection& hits) const;

    public:
      /// Get the track id with the highest corresponding purity.
      MCTrackIdPurityPair getHighestPurity(const ACDCHitCollection& hits) const;

      /// Getter for the Monte Carlo track id matched to this collection of hits
      /** On first encounter of a collection of hits this evaluates the purities for the contained track ids.
       *  The match is valid if the highest purity exceeds the s_minimalMatchPurity threshold.
       *  In case the highest purity is to low to result will be INVALID_ITRACK. To result is stored
       *  in member map object for fast look up for repeated calls.*/
      ITrackType getMCTrackId(const ACDCHitCollection* ptrHits) const;

      /// Getter for the difference of correct versus incorrect right left passage informations
      int getCorrectRLVote(const ACDCHitCollection* ptrHits) const;

      /// Getter for the right left passge purity which respects the forward backward reconstruction
      double getRLPurity(const ACDCHitCollection* ptrHits) const;

      /// Getter for the mc particle matched to this collection of hits
      const MCParticle* getMCParticle(const ACDCHitCollection* ptrHits) const;


      /// Getter for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      const CDCHit* getFirstHit(const ACDCHitCollection* ptrHits) const;

      /// Getter for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      const CDCHit* getLastHit(const ACDCHitCollection* ptrHits) const;



      /// Getter for in track id for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstInTrackId(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getFirstHit(ptrHits)); }

      /// Getter for in track id for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastInTrackId(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackId(getLastHit(ptrHits)); }



      /// Getter for in track collection of hits id for the first hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstInTrackSegmentId(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getFirstHit(ptrHits)); }

      /// Getter for in track collection of hits id for the last hit in the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastInTrackSegmentId(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getInTrackSegmentId(getLastHit(ptrHits)); }



      /// Getter for number of passed superlayer till the first hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstNPassedSuperLayers(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getFirstHit(ptrHits)); }

      /// Getter for number of passed superlayer till the last hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastNPassedSuperLayers(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNPassedSuperLayers(getLastHit(ptrHits)); }

      /// Getter for number of loops till the first hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getFirstNLoops(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNLoops(getFirstHit(ptrHits)); }

      /// Getter for number of loops till the last hit the collection of hits which has the Monte Carlo track id matched to this collection of hits
      Index getLastNLoops(const ACDCHitCollection* ptrHits) const
      { return CDCMCHitLookUp::getInstance().getNLoops(getLastHit(ptrHits)); }

      /**
       *  Returns the orientation of the collection of hits relative to its matched track.
       *  * Returns EForwardBackward::c_Invalid if the collection of hits is not matched to any track.
       *  * Returns EForwardBackward::c_Forward if the collection of hits is coaligned with the matched track.
       *  * Returns EForwardBackward::c_Backward if the collection of hits is coaligned with the matched track interpreted in reverse.
       *  * Returns EForwardBackward::c_Unknown if the matter cannot be decided.
       */
      EForwardBackward isForwardOrBackwardToMCTrack(const ACDCHitCollection* ptrHits) const;

      /**
       *  Returns if the second collection of hits follows the first collection of hits in their common Monte Carlo track.
       *
       *  * Returns EForwardBackward::c_Invalid if the collection of hits is not matched in the same Monte Carlo track.
       *  * Returns EForwardBackward::c_Forward  if both collections of hits are coaligned
       *                     with the Monte Carlo track and the second collection of hits
       *                     comes after the first.
       *  * Returns EForwardBackward::c_Backward if both collections of hits are anticoaligned
       *                     with the Monte Carlo track and the second collection of hits
       *                     comes after the first in the reversed sense.
       */
      EForwardBackward areAlignedInMCTrack(const ACDCHitCollection* ptrFromHits,
                                           const ACDCHitCollection* ptrToHits) const;

      /**
       *  Returns if the second collection of hits follows the first collection of hits in their common Monte Carlo track.
       *
       *  Also checks that the majority of the right left passage informations agrees with the forward backward information
       */
      EForwardBackward areAlignedInMCTrackWithRLCheck(const ACDCHitCollection* ptrFromHits,
                                                      const ACDCHitCollection* ptrToHits) const;

      /** Returns the trajectory of the collection of hits */
      CDCTrajectory3D getTrajectory3D(const ACDCHitCollection* ptrHits) const;

    private:
      /// Threshold for the purity that must be exceeded to be considered as a match.
      float m_minimalMatchPurity = 0.5;

      /// Threshold for the correct fraction of right left passage informations to be considered a match
      float m_minimalRLPurity = 0.5;
    };
  }
}
