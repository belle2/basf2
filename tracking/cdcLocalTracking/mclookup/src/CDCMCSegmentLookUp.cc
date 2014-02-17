/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCSegmentLookUp.h"
#include "../include/CDCMCManager.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCMCSegmentLookUp)


CDCMCSegmentLookUp::CDCMCSegmentLookUp()
{
}


/** Destructor. */
CDCMCSegmentLookUp::~CDCMCSegmentLookUp()
{
}

const CDCMCSegmentLookUp& CDCMCSegmentLookUp::getInstance()
{
  return CDCMCManager::getMCSegmentLookUp();
}


void CDCMCSegmentLookUp::clear()
{
  m_mcTrackIds.clear();
}



ITrackType CDCMCSegmentLookUp::getMCTrackId(const CDCRecoSegment2D* ptrSegment2D) const
{
  if (not ptrSegment2D) return INVALID_ITRACK;
  const CDCRecoSegment2D& segment2D = *ptrSegment2D;
  auto itFound = m_mcTrackIds.find(ptrSegment2D);

  if (itFound == m_mcTrackIds.end()) {
    MCTrackIdEfficiencyPair mcTrackIdAndEfficiency = getHighestEfficieny(segment2D);
    if (mcTrackIdAndEfficiency.getEfficiency() < 0.5) {
      m_mcTrackIds[ptrSegment2D] = mcTrackIdAndEfficiency.getMCTrackId();
      return mcTrackIdAndEfficiency.getMCTrackId();
    } else {
      m_mcTrackIds[ptrSegment2D] = INVALID_ITRACK;
      return INVALID_ITRACK;
    }

  } else {
    return itFound->second;
  }
}



const CDCHit* CDCMCSegmentLookUp::getFirstHit(const CDCRecoSegment2D* ptrSegment2D) const
{
  if (not ptrSegment2D) return nullptr;
  const CDCRecoSegment2D& segment2D = *ptrSegment2D;

  ITrackType mcTrackId = getMCTrackId(ptrSegment2D);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCRecoHit2D & recoHit2D : segment2D) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(recoHit2D)) return recoHit2D;
  }
  return nullptr;

}


const CDCHit* CDCMCSegmentLookUp::getLastHit(const CDCRecoSegment2D* ptrSegment2D) const
{

  if (not ptrSegment2D) return nullptr;
  const CDCRecoSegment2D& segment2D = *ptrSegment2D;

  ITrackType mcTrackId = getMCTrackId(ptrSegment2D);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCRecoHit2D & recoHit2D : segment2D.reverseRange()) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(recoHit2D)) return recoHit2D;
  }
  return nullptr;

}




ForwardBackwardInfo CDCMCSegmentLookUp::isForwardOrBackwardToMCTrack(const CDCRecoSegment2D* ptrSegment2D) const
{
  Index firstInTrackId = getFirstInTrackId(ptrSegment2D);
  Index lastInTrackId = getLastInTrackId(ptrSegment2D);
  if (firstInTrackId == INVALID_INDEX or lastInTrackId == INVALID_INDEX) return INVALID_INFO;
  else if (firstInTrackId < lastInTrackId) return FORWARD;
  else if (firstInTrackId > lastInTrackId) return BACKWARD;
  else if (firstInTrackId == lastInTrackId) return INVALID_INFO;
  return INVALID_ITRACK;
}


ForwardBackwardInfo CDCMCSegmentLookUp::areAlignedInMCTrack(const CDCRecoSegment2D* ptrStartSegment2D, const CDCRecoSegment2D* ptrEndSegment2D) const
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
  Index lastInTrackIdOfStartSegment = getLastInTrackId(ptrStartSegment2D);
  if (lastInTrackIdOfStartSegment == INVALID_INDEX) return INVALID_INFO;

  Index firstInTrackIdOfEndSegment = getFirstInTrackId(ptrEndSegment2D);
  if (firstInTrackIdOfEndSegment == INVALID_INDEX) return INVALID_INFO;

  if (startFBInfo == FORWARD and  endFBInfo == FORWARD) {

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



