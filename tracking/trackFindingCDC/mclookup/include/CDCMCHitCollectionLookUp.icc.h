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

#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TDatabasePDG.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class ACDCHitCollection>
    void CDCMCHitCollectionLookUp<ACDCHitCollection>::clear()
    {
      B2DEBUG(100, "Clearing CDCMCHitCollectionLookUp<ACDCHitCollection>");
    }

    template <class ACDCHitCollection>
    std::map<ITrackType, size_t>
    CDCMCHitCollectionLookUp<ACDCHitCollection>::getHitCountByMCTrackId(
      const ACDCHitCollection& hits) const
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

    template <class ACDCHitCollection>
    MCTrackIdPurityPair CDCMCHitCollectionLookUp<ACDCHitCollection>::getHighestPurity(
      const ACDCHitCollection& hits) const
    {
      std::map<ITrackType, size_t> hitCountByMCTrackId = getHitCountByMCTrackId(hits);

      size_t nHits = 0;
      std::pair<ITrackType, size_t> highestHitCountMCTrackId(0, 0);
      std::max_element(hitCountByMCTrackId.begin(), hitCountByMCTrackId.end(), LessOf<Second>());

      for (const std::pair<ITrackType, size_t>& hitCountForMCTrackId : hitCountByMCTrackId) {

        nHits += hitCountForMCTrackId.second;

        if (highestHitCountMCTrackId.second < hitCountForMCTrackId.second) {
          highestHitCountMCTrackId = hitCountForMCTrackId;
        }
      }

      const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

      int correctRLVote = 0;
      for (const auto& recoHit : hits) {
        const CDCHit* hit = recoHit;
        ERightLeft mcRLInfo = mcHitLookUp.getRLInfo(hit);
        ERightLeft rlInfo = recoHit.getRLInfo();
        if (rlInfo == mcRLInfo) {
          ++correctRLVote;
        } else {
          --correctRLVote;
        }
      }

      const float purity = static_cast<float>(highestHitCountMCTrackId.second) / nHits;
      return MCTrackIdPurityPair(highestHitCountMCTrackId.first, purity, correctRLVote);
    }

    template <class ACDCHitCollection>
    ITrackType
    CDCMCHitCollectionLookUp<ACDCHitCollection>::getMCTrackId(const ACDCHitCollection* ptrHits) const
    {
      if (not ptrHits) return INVALID_ITRACK;
      const ACDCHitCollection& hits = *ptrHits;
      MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(hits);
      if (mcTrackIdAndPurity.getPurity() >= m_minimalMatchPurity) {
        return mcTrackIdAndPurity.getMCTrackId();
      } else {
        return INVALID_ITRACK;
      }
    }

    template <class ACDCHitCollection>
    int CDCMCHitCollectionLookUp<ACDCHitCollection>::getCorrectRLVote(
      const ACDCHitCollection* ptrHits) const
    {
      if (not ptrHits) return INVALID_ITRACK;
      const ACDCHitCollection& hits = *ptrHits;
      MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(hits);
      if (mcTrackIdAndPurity.getPurity() >= m_minimalMatchPurity) {
        return mcTrackIdAndPurity.getCorrectRLVote();
      } else {
        return 0;
      }
    }

    template <class ACDCHitCollection>
    double
    CDCMCHitCollectionLookUp<ACDCHitCollection>::getRLPurity(const ACDCHitCollection* ptrHits) const
    {
      EForwardBackward fbInfo = isForwardOrBackwardToMCTrack(ptrHits);
      if (fbInfo == EForwardBackward::c_Invalid) return NAN;

      int correctRLVote = getCorrectRLVote(ptrHits);

      if (fbInfo == EForwardBackward::c_Backward) {
        correctRLVote = -correctRLVote;
      }

      int nCorrectRL = (correctRLVote + ptrHits->size()) / 2;
      float rlPurity = 1.0 * nCorrectRL / ptrHits->size();
      return rlPurity;
    }

    template <class ACDCHitCollection>
    const MCParticle* CDCMCHitCollectionLookUp<ACDCHitCollection>::getMCParticle(
      const ACDCHitCollection* ptrHits) const
    {
      const CDCHit* ptrHit = getFirstHit(ptrHits);
      const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
      return mcHitLookUp.getMCParticle(ptrHit);
    }

    template <class ACDCHitCollection>
    const CDCHit*
    CDCMCHitCollectionLookUp<ACDCHitCollection>::getFirstHit(const ACDCHitCollection* ptrHits) const
    {
      if (not ptrHits) return nullptr;
      const ACDCHitCollection& hits = *ptrHits;

      ITrackType mcTrackId = getMCTrackId(ptrHits);
      if (mcTrackId == INVALID_ITRACK) return nullptr;

      const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

      for (const CDCHit* hit : hits) {
        if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
      }
      return nullptr;
    }

    template <class ACDCHitCollection>
    const CDCHit*
    CDCMCHitCollectionLookUp<ACDCHitCollection>::getLastHit(const ACDCHitCollection* ptrHits) const
    {

      if (not ptrHits) return nullptr;
      const ACDCHitCollection& hits = *ptrHits;

      ITrackType mcTrackId = getMCTrackId(ptrHits);
      if (mcTrackId == INVALID_ITRACK) return nullptr;

      const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

      for (const CDCHit* hit : reversedRange(hits)) {
        if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
      }
      return nullptr;
    }

    template <class ACDCHitCollection>
    EForwardBackward CDCMCHitCollectionLookUp<ACDCHitCollection>::isForwardOrBackwardToMCTrack(
      const ACDCHitCollection* ptrHits) const
    {
      Index firstInTrackId = getFirstInTrackId(ptrHits);
      Index lastInTrackId = getLastInTrackId(ptrHits);
      if (firstInTrackId == c_InvalidIndex or lastInTrackId == c_InvalidIndex) {
        return EForwardBackward::c_Invalid;
      } else if (firstInTrackId < lastInTrackId) {
        return EForwardBackward::c_Forward;
      } else if (firstInTrackId > lastInTrackId) {
        return EForwardBackward::c_Backward;
      } else if (firstInTrackId == lastInTrackId) {
        return EForwardBackward::c_Unknown;
      }
      return EForwardBackward::c_Invalid;
    }

    template <class ACDCHitCollection>
    EForwardBackward CDCMCHitCollectionLookUp<ACDCHitCollection>::areAlignedInMCTrack(
      const ACDCHitCollection* ptrFromHits,
      const ACDCHitCollection* ptrToHits) const
    {
      /// Check if the segments are in the same track
      ITrackType fromMCTrackId = getMCTrackId(ptrFromHits);
      if (fromMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

      ITrackType toMCTrackId = getMCTrackId(ptrToHits);
      if (toMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

      if (fromMCTrackId != toMCTrackId) return EForwardBackward::c_Invalid;

      // Check if the segments are sensable on their own
      EForwardBackward fromFBInfo = isForwardOrBackwardToMCTrack(ptrFromHits);
      if (fromFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

      EForwardBackward toFBInfo = isForwardOrBackwardToMCTrack(ptrToHits);
      if (toFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

      if (fromFBInfo != toFBInfo) return EForwardBackward::c_Invalid;

      {
        // Now check if hits are aligned within their common track
        // Index firstNPassedSuperLayersOfFromHits = getFirstNPassedSuperLayers(ptrFromHits);
        Index lastNPassedSuperLayersOfFromHits = getLastNPassedSuperLayers(ptrFromHits);
        // if (firstNPassedSuperLayersOfFromHits == c_InvalidIndex) return
        // EForwardBackward::c_Invalid;
        if (lastNPassedSuperLayersOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        Index firstNPassedSuperLayersOfToHits = getFirstNPassedSuperLayers(ptrToHits);
        // Index lastNPassedSuperLayersOfToHits = getLastNPassedSuperLayers(ptrToHits);
        if (firstNPassedSuperLayersOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
        // if (lastNPassedSuperLayersOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        if (lastNPassedSuperLayersOfFromHits < firstNPassedSuperLayersOfToHits) {
          if (fromFBInfo == EForwardBackward::c_Forward and
              toFBInfo == EForwardBackward::c_Forward) {
            return EForwardBackward::c_Forward;
          } else {
            return EForwardBackward::c_Invalid;
          }
        } else if (firstNPassedSuperLayersOfToHits < lastNPassedSuperLayersOfFromHits) {
          if (fromFBInfo == EForwardBackward::c_Backward and
              toFBInfo == EForwardBackward::c_Backward) {
            return EForwardBackward::c_Backward;
          } else {
            return EForwardBackward::c_Invalid;
          }
        }
      }

      {
        // Now we are in the same true segment with both segments
        // Index firstInTrackSegmentIdOfFromHits = getFirstInTrackSegmentId(ptrFromHits);
        Index lastInTrackSegmentIdOfFromHits = getLastInTrackSegmentId(ptrFromHits);
        // if (firstInTrackSegmentIdOfFromHits == c_InvalidIndex) return
        // EForwardBackward::c_Invalid;
        if (lastInTrackSegmentIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        Index firstInTrackSegmentIdOfToHits = getFirstInTrackSegmentId(ptrToHits);
        // Index lastInTrackSegmentIdOfToHits = getLastInTrackSegmentId(ptrToHits);
        if (firstInTrackSegmentIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
        // if (lastInTrackSegmentIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        if (lastInTrackSegmentIdOfFromHits < firstInTrackSegmentIdOfToHits) {
          if (fromFBInfo == EForwardBackward::c_Forward and
              toFBInfo == EForwardBackward::c_Forward) {
            return EForwardBackward::c_Forward;
          } else {
            return EForwardBackward::c_Invalid;
          }
        } else if (firstInTrackSegmentIdOfToHits < lastInTrackSegmentIdOfFromHits) {
          // Test if to segment lies before in the mc track
          // Hence the whole pair of segments is reverse to the track direction of flight
          if (fromFBInfo == EForwardBackward::c_Backward and
              toFBInfo == EForwardBackward::c_Backward) {
            return EForwardBackward::c_Backward;
          } else {
            return EForwardBackward::c_Invalid;
          }
        }
      }

      {
        // Now we are in the same true segment with both of the hits
        // Index firstInTrackIdOfFromHits = getFirstInTrackId(ptrFromHits);
        Index lastInTrackIdOfFromHits = getLastInTrackId(ptrFromHits);
        // if (firstInTrackIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
        if (lastInTrackIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        Index firstInTrackIdOfToHits = getFirstInTrackId(ptrToHits);
        // Index lastInTrackIdOfToHits = getLastInTrackId(ptrToHits);
        if (firstInTrackIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
        // if (lastInTrackIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

        // Relax conditions somewhat such that segments may overlap at the borders.

        if (lastInTrackIdOfFromHits - 1 < firstInTrackIdOfToHits + 1) {
          if (fromFBInfo == EForwardBackward::c_Forward and
              toFBInfo == EForwardBackward::c_Forward) {
            return EForwardBackward::c_Forward;
          }
        }

        if (firstInTrackIdOfToHits - 1 < lastInTrackIdOfFromHits + 1) {
          if (fromFBInfo == EForwardBackward::c_Backward and
              toFBInfo == EForwardBackward::c_Backward) {
            return EForwardBackward::c_Backward;
          }
        }
      }
      // FIXME: Handle intertwined hits that are not cleanly consecutive along the track?
      return EForwardBackward::c_Invalid;
    }

    template <class ACDCHitCollection>
    EForwardBackward CDCMCHitCollectionLookUp<ACDCHitCollection>::areAlignedInMCTrackWithRLCheck(
      const ACDCHitCollection* ptrFromHits,
      const ACDCHitCollection* ptrToHits) const
    {
      EForwardBackward result = areAlignedInMCTrack(ptrFromHits, ptrToHits);
      if (result == EForwardBackward::c_Invalid) return result;

      int fromCorrectRLVote = getCorrectRLVote(ptrFromHits);
      int toCorrectRLVote = getCorrectRLVote(ptrToHits);

      if (result == EForwardBackward::c_Backward) {
        fromCorrectRLVote = -fromCorrectRLVote;
        toCorrectRLVote = -toCorrectRLVote;
      }

      int fromNCorrectRL = (fromCorrectRLVote + ptrFromHits->size()) / 2;
      int toNCorrectRL = (toCorrectRLVote + ptrToHits->size()) / 2;

      float fromRLPurity = 1.0 * fromNCorrectRL / ptrFromHits->size();
      float toRLPurity = 1.0 * toNCorrectRL / ptrToHits->size();

      // Require the minimal rl purity and also at least 2.5 correct hits
      // (cut chosen to require all correct in single hit triplet segment)
      if (fromRLPurity > m_minimalRLPurity and toRLPurity > m_minimalRLPurity and
          fromNCorrectRL > 2.5 and toNCorrectRL > 2.5) {
        return result;
      }

      return EForwardBackward::c_Invalid;
    }

    template <class ACDCHitCollection>
    CDCTrajectory3D CDCMCHitCollectionLookUp<ACDCHitCollection>::getTrajectory3D(
      const ACDCHitCollection* ptrHits) const
    {
      if (not ptrHits) {
        B2WARNING("Segment is nullptr. Could not get fit.");
        return CDCTrajectory3D();
      }

      const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

      const CDCHit* ptrFirstHit = getFirstHit(ptrHits);
      const CDCSimHit* ptrPrimarySimHit = mcHitLookUp.getClosestPrimarySimHit(ptrFirstHit);

      if (not ptrPrimarySimHit) {
        // If there is no primary SimHit simply use the secondary simhit as reference
        ptrPrimarySimHit = mcHitLookUp.getSimHit(ptrFirstHit);
        if (not ptrPrimarySimHit) {
          return CDCTrajectory3D();
        }
      }

      const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

      Vector3D mom3D{primarySimHit.getMomentum()};
      Vector3D pos3D{primarySimHit.getPosTrack()};
      double time{primarySimHit.getFlightTime()};

      int pdgCode = primarySimHit.getPDGCode();
      const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);

      if (not ptrTPDGParticle) {
        B2WARNING("No particle for PDG code " << pdgCode << ". Could not get fit");
        return CDCTrajectory3D();
      }

      const TParticlePDG& tPDGParticle = *ptrTPDGParticle;

      double charge = tPDGParticle.Charge() / 3.0;

      ESign chargeSign = sign(charge);

      CDCTrajectory3D trajectory3D(pos3D, time, mom3D, charge);

      ESign settedChargeSign = trajectory3D.getChargeSign();

      if (chargeSign != settedChargeSign) {
        B2WARNING("Charge sign of mc particle is not the same as the one of the fit");
      }

      return trajectory3D;
    }
  }
}
