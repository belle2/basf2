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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <TDatabasePDG.h>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


template<class CDCHitCollection>
void
CDCMCHitCollectionLookUp<CDCHitCollection>
::clear()
{
  B2DEBUG(100, "Clearing CDCMCHitCollectionLookUp<CDCHitCollection>");
  m_mcTrackIds.clear();
  B2DEBUG(100, "m_mcTrackIds.size(): " <<  m_mcTrackIds.size());
}




template<class CDCHitCollection>
const float
CDCMCHitCollectionLookUp<CDCHitCollection>
::s_minimalMatchPurity = 0.5;



template<class CDCHitCollection>
ITrackType
CDCMCHitCollectionLookUp<CDCHitCollection>
::getMCTrackId(const CDCHitCollection* ptrHits) const
{

  if (not ptrHits) return INVALID_ITRACK;

  const CDCHitCollection& hits = *ptrHits;
  auto itFound = m_mcTrackIds.find(ptrHits);

  if (itFound == m_mcTrackIds.end()) {
    MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(hits);
    if (mcTrackIdAndPurity.getPurity() >= s_minimalMatchPurity) {
      m_mcTrackIds[ptrHits] = mcTrackIdAndPurity.getMCTrackId();
      return mcTrackIdAndPurity.getMCTrackId();
    } else {
      m_mcTrackIds[ptrHits] = INVALID_ITRACK;
      return INVALID_ITRACK;
    }

  } else {
    return itFound->second;
  }
}


template<class CDCHitCollection>
const MCParticle*
CDCMCHitCollectionLookUp<CDCHitCollection>
::getMCParticle(const CDCHitCollection* ptrHits) const
{
  const CDCHit* ptrHit = getFirstHit(ptrHits);
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  return mcHitLookUp.getMCParticle(ptrHit);
}


template<class CDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<CDCHitCollection>
::getFirstHit(const CDCHitCollection* ptrHits) const
{
  if (not ptrHits) return nullptr;
  const CDCHitCollection& hits = *ptrHits;

  ITrackType mcTrackId = getMCTrackId(ptrHits);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCHit* hit : hits) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
  }
  return nullptr;

}




template<class CDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<CDCHitCollection>
::getLastHit(const CDCHitCollection* ptrHits) const
{

  if (not ptrHits) return nullptr;
  const CDCHitCollection& hits = *ptrHits;

  ITrackType mcTrackId = getMCTrackId(ptrHits);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCHit* hit : boost::adaptors::reverse(hits)) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
  }
  return nullptr;

}




template<class CDCHitCollection>
ForwardBackwardInfo
CDCMCHitCollectionLookUp<CDCHitCollection>
::isForwardOrBackwardToMCTrack(const CDCHitCollection* ptrHits) const
{
  Index firstInTrackId = getFirstInTrackId(ptrHits);
  Index lastInTrackId = getLastInTrackId(ptrHits);
  if (firstInTrackId == INVALID_INDEX or lastInTrackId == INVALID_INDEX) return INVALID_INFO;
  else if (firstInTrackId < lastInTrackId) return FORWARD;
  else if (firstInTrackId > lastInTrackId) return BACKWARD;
  else if (firstInTrackId == lastInTrackId) return INVALID_INFO;
  return INVALID_ITRACK;
}




template<class CDCHitCollection>
ForwardBackwardInfo
CDCMCHitCollectionLookUp<CDCHitCollection>
::areAlignedInMCTrack(const CDCHitCollection* ptrStartSegment2D,
                      const CDCHitCollection* ptrEndSegment2D) const
{

  // Check if the segments are sensable on their own
  ForwardBackwardInfo startFBInfo = isForwardOrBackwardToMCTrack(ptrStartSegment2D);
  if (startFBInfo == INVALID_INFO) return INVALID_INFO;

  ForwardBackwardInfo endFBInfo = isForwardOrBackwardToMCTrack(ptrEndSegment2D);
  if (endFBInfo == INVALID_INFO) return INVALID_INFO;

  if (startFBInfo != endFBInfo) return INVALID_INFO;


  /// Check if the segments are in the same track
  ITrackType startMCTrackId = getMCTrackId(ptrStartSegment2D);
  if (startMCTrackId == INVALID_ITRACK) return INVALID_INFO;

  ITrackType endMCTrackId = getMCTrackId(ptrEndSegment2D);
  if (endMCTrackId == INVALID_ITRACK) return INVALID_INFO;

  if (startMCTrackId != endMCTrackId) return INVALID_INFO;


  // Now check are aligned with their common track
  // Examine if they are in the same super layer

  Index lastNPassedSuperLayersOfStartSegment = getLastNPassedSuperLayers(ptrStartSegment2D);
  if (lastNPassedSuperLayersOfStartSegment == INVALID_INDEX) return INVALID_INFO;

  Index firstNPassedSuperLayersOfEndSegment = getFirstNPassedSuperLayers(ptrEndSegment2D);
  if (firstNPassedSuperLayersOfEndSegment == INVALID_INDEX) return INVALID_INFO;

  if (lastNPassedSuperLayersOfStartSegment < firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == FORWARD and  endFBInfo == FORWARD) return FORWARD;
    else return INVALID_INFO;
  } else if (lastNPassedSuperLayersOfStartSegment > firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == BACKWARD and endFBInfo == BACKWARD) return BACKWARD;
    else return INVALID_INFO;
  }

  // Now we are in the same passed super layer with both segments
  Index lastInTrackIdOfStartSegment = getLastInTrackId(ptrStartSegment2D);
  if (lastInTrackIdOfStartSegment == INVALID_INDEX) return INVALID_INFO;

  Index firstInTrackIdOfEndSegment = getFirstInTrackId(ptrEndSegment2D);
  if (firstInTrackIdOfEndSegment == INVALID_INDEX) return INVALID_INFO;

  if (startFBInfo == FORWARD and endFBInfo == FORWARD) {
    if (lastInTrackIdOfStartSegment < firstInTrackIdOfEndSegment) return FORWARD;
    else return INVALID_INFO;
  } else if (startFBInfo == BACKWARD and endFBInfo == BACKWARD) {
    // Test if end segment lies before in the mc track
    // Hence the whole pair of segments is reverse to the track direction of flight
    if (lastInTrackIdOfStartSegment > firstInTrackIdOfEndSegment) return BACKWARD;
    else return INVALID_INFO;
  }
  return INVALID_INFO;

}




template<class CDCHitCollection>
CDCTrajectory3D
CDCMCHitCollectionLookUp<CDCHitCollection>
::getTrajectory3D(const CDCHitCollection* ptrHits) const
{
  CDCTrajectory3D trajectory3D;

  if (not ptrHits) {
    B2WARNING("Segment is nullptr. Could not get fit.");
    return trajectory3D;
  }

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCHit* ptrFirstHit = getFirstHit(ptrHits);
  const CDCSimHit* ptrPrimarySimHit = mcHitLookUp.getClosestPrimarySimHit(ptrFirstHit);

  if (not ptrPrimarySimHit) {
    // If there is no primary SimHit simply use the secondary simhit as reference
    ptrPrimarySimHit = mcHitLookUp.getSimHit(ptrFirstHit);
    if (not ptrPrimarySimHit) {
      B2WARNING("First simhit of CDCRecoSegment is nullptr. Could not get fit.");
      return trajectory3D;
    }
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  Vector3D mom3D{primarySimHit.getMomentum()};
  Vector3D pos3D{primarySimHit.getPosTrack()};

  int pdgCode = primarySimHit.getPDGCode();
  const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);

  if (not ptrTPDGParticle) {
    B2WARNING("No particle for PDG code " << pdgCode << ". Could not get fit");
    return trajectory3D;
  }

  const TParticlePDG& tPDGParticle = *ptrTPDGParticle;

  double charge = tPDGParticle.Charge() / 3.0;

  SignType chargeSign = sign(charge);

  trajectory3D.setPosMom3D(pos3D, mom3D, charge);

  SignType settedChargeSign = trajectory3D.getChargeSign();

  if (chargeSign != settedChargeSign) {
    B2WARNING("Charge sign of mc particle is not the same as the one of the fit");
  }

  return trajectory3D;
}


// Explicit instantiation for the relevant cases.
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCRecoSegment2D>;
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCTrack>;
