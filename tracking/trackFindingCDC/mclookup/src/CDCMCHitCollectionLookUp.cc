/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <TDatabasePDG.h>
#include <boost/range/adaptor/reversed.hpp>

using namespace Belle2;
using namespace TrackFindingCDC;


template<class ACDCHitCollection>
void CDCMCHitCollectionLookUp<ACDCHitCollection>::clear()
{
  B2DEBUG(100, "Clearing CDCMCHitCollectionLookUp<ACDCHitCollection>");
}




template<class ACDCHitCollection>
const float
CDCMCHitCollectionLookUp<ACDCHitCollection>
::s_minimalMatchPurity = 0.5;



template<class ACDCHitCollection>
ITrackType
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getMCTrackId(const ACDCHitCollection* ptrHits) const
{
  if (not ptrHits) return INVALID_ITRACK;
  const ACDCHitCollection& hits = *ptrHits;
  MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(hits);
  if (mcTrackIdAndPurity.getPurity() >= s_minimalMatchPurity) {
    return mcTrackIdAndPurity.getMCTrackId();
  } else {
    return INVALID_ITRACK;
  }
}


template<class ACDCHitCollection>
const MCParticle*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getMCParticle(const ACDCHitCollection* ptrHits) const
{
  const CDCHit* ptrHit = getFirstHit(ptrHits);
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  return mcHitLookUp.getMCParticle(ptrHit);
}


template<class ACDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getFirstHit(const ACDCHitCollection* ptrHits) const
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




template<class ACDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getLastHit(const ACDCHitCollection* ptrHits) const
{

  if (not ptrHits) return nullptr;
  const ACDCHitCollection& hits = *ptrHits;

  ITrackType mcTrackId = getMCTrackId(ptrHits);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCHit* hit : boost::adaptors::reverse(hits)) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
  }
  return nullptr;

}




template<class ACDCHitCollection>
EForwardBackward
CDCMCHitCollectionLookUp<ACDCHitCollection>
::isForwardOrBackwardToMCTrack(const ACDCHitCollection* ptrHits) const
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

template<class ACDCHitCollection>
EForwardBackward
CDCMCHitCollectionLookUp<ACDCHitCollection>
::areAlignedInMCTrack(const ACDCHitCollection* ptrFromHits,
                      const ACDCHitCollection* ptrToHits) const
{
  // Check if the segments are sensable on their own
  EForwardBackward fromFBInfo = isForwardOrBackwardToMCTrack(ptrFromHits);
  if (fromFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

  EForwardBackward toFBInfo = isForwardOrBackwardToMCTrack(ptrToHits);
  if (toFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

  if (fromFBInfo != toFBInfo) return EForwardBackward::c_Invalid;

  /// Check if the segments are in the same track
  ITrackType fromMCTrackId = getMCTrackId(ptrFromHits);
  if (fromMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

  ITrackType toMCTrackId = getMCTrackId(ptrToHits);
  if (toMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

  if (fromMCTrackId != toMCTrackId) return EForwardBackward::c_Invalid;

  // Now check are aligned with their common track
  // Examine if they are in the same super layer

  Index firstNPassedSuperLayersOfFromHits = getFirstNPassedSuperLayers(ptrFromHits);
  Index lastNPassedSuperLayersOfFromHits = getLastNPassedSuperLayers(ptrFromHits);
  if (firstNPassedSuperLayersOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastNPassedSuperLayersOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  Index firstNPassedSuperLayersOfToHits = getFirstNPassedSuperLayers(ptrToHits);
  Index lastNPassedSuperLayersOfToHits = getLastNPassedSuperLayers(ptrToHits);
  if (firstNPassedSuperLayersOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastNPassedSuperLayersOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  if (lastNPassedSuperLayersOfFromHits < firstNPassedSuperLayersOfToHits) {
    if (fromFBInfo == EForwardBackward::c_Forward and toFBInfo == EForwardBackward::c_Forward) {
      return EForwardBackward::c_Forward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  } else if (lastNPassedSuperLayersOfToHits < firstNPassedSuperLayersOfFromHits) {
    if (fromFBInfo == EForwardBackward::c_Backward and toFBInfo == EForwardBackward::c_Backward) {
      return EForwardBackward::c_Backward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  }

  // Now we are in the same true segment with both segments
  Index firstInTrackSegmentIdOfFromHits = getFirstInTrackSegmentId(ptrFromHits);
  Index lastInTrackSegmentIdOfFromHits = getLastInTrackSegmentId(ptrFromHits);
  if (firstInTrackSegmentIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastInTrackSegmentIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  Index firstInTrackSegmentIdOfToHits = getFirstInTrackSegmentId(ptrToHits);
  Index lastInTrackSegmentIdOfToHits = getLastInTrackSegmentId(ptrToHits);
  if (firstInTrackSegmentIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastInTrackSegmentIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  if (lastInTrackSegmentIdOfFromHits < firstInTrackSegmentIdOfToHits) {
    if (fromFBInfo == EForwardBackward::c_Forward and toFBInfo == EForwardBackward::c_Forward) {
      return EForwardBackward::c_Forward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  } else if (lastInTrackSegmentIdOfToHits < firstInTrackSegmentIdOfFromHits) {
    // Test if to segment lies before in the mc track
    // Hence the whole pair of segments is reverse to the track direction of flight
    if (fromFBInfo == EForwardBackward::c_Backward and toFBInfo == EForwardBackward::c_Backward) {
      return EForwardBackward::c_Backward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  }

  // Now we are in the same true segment with both of the hits
  Index firstInTrackIdOfFromHits = getFirstInTrackId(ptrFromHits);
  Index lastInTrackIdOfFromHits = getLastInTrackId(ptrFromHits);
  if (firstInTrackIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastInTrackIdOfFromHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  Index firstInTrackIdOfToHits = getFirstInTrackId(ptrToHits);
  Index lastInTrackIdOfToHits = getLastInTrackId(ptrToHits);
  if (firstInTrackIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;
  if (lastInTrackIdOfToHits == c_InvalidIndex) return EForwardBackward::c_Invalid;

  if (fromFBInfo == EForwardBackward::c_Forward and toFBInfo == EForwardBackward::c_Forward) {
    if (lastInTrackIdOfFromHits < firstInTrackIdOfToHits) {
      return EForwardBackward::c_Forward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  } else if (fromFBInfo == EForwardBackward::c_Backward and toFBInfo == EForwardBackward::c_Backward) {
    // Test if to segment lies before in the mc track
    // Hence the whole pair of segments is reverse to the track direction of flight
    if (lastInTrackIdOfToHits < firstInTrackIdOfFromHits) {
      return EForwardBackward::c_Backward;
    } else {
      return EForwardBackward::c_Invalid;
    }
  }

  // FIXME: Handle intertwined hits that are not cleanly consecutive along the track?
  return EForwardBackward::c_Invalid;
}




template<class ACDCHitCollection>
CDCTrajectory3D
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getTrajectory3D(const ACDCHitCollection* ptrHits) const
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


// Explicit instantiation for the relevant cases.
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCRecoSegment2D>;
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCTrack>;
