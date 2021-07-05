/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.icc.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MCSymmetric<BaseFacetFilter>;

MCFacetFilter::MCFacetFilter(bool allowReverse)
  : Super(allowReverse)
{
}

Weight MCFacetFilter::operator()(const CDCFacet& facet)
{
  const int maxInTrackHitIdDifference = 3;
  bool isCorrectFacet = operator()(facet, maxInTrackHitIdDifference);

  bool isCorrectReverseFacet =
    this->getAllowReverse() and operator()(facet.reversed(), maxInTrackHitIdDifference);

  if (isCorrectFacet) {
    if (facet.getFitLine()->isInvalid()) {
      facet.adjustFitLine();
    }
    return 3.0;
  } else if (isCorrectReverseFacet) {
    if (facet.getFitLine()->isInvalid()) {
      facet.adjustFitLine();
    }
    return -3.0;
  } else {
    return NAN;
  }
}

bool MCFacetFilter::operator()(const CDCRLWireHitTriple& rlWireHitTriple,
                               int maxInTrackHitIdDifference)
{
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCWireHit& startWireHit = rlWireHitTriple.getStartWireHit();
  const CDCWireHit& middleWireHit = rlWireHitTriple.getMiddleWireHit();
  const CDCWireHit& endWireHit = rlWireHitTriple.getEndWireHit();

  // First check if the track ids are in agreement
  ITrackType startMCTrackId = mcHitLookUp.getMCTrackId(startWireHit.getHit());
  ITrackType middleMCTrackId = mcHitLookUp.getMCTrackId(middleWireHit.getHit());
  ITrackType endMCTrackId = mcHitLookUp.getMCTrackId(endWireHit.getHit());

  if (startMCTrackId == INVALID_ITRACK or
      middleMCTrackId == INVALID_ITRACK or
      endMCTrackId == INVALID_ITRACK) {
    return false;
  }

  if (not(startMCTrackId == middleMCTrackId and middleMCTrackId == endMCTrackId)) {
    return false;
  }

  int startInTrackId = mcHitLookUp.getInTrackId(startWireHit.getHit());
  int middleInTrackId = mcHitLookUp.getInTrackId(middleWireHit.getHit());
  int endInTrackId = mcHitLookUp.getInTrackId(endWireHit.getHit());

  int startToMiddleInTrackDistance =  middleInTrackId - startInTrackId;
  int middleToEndInTrackDistance =  endInTrackId - middleInTrackId;

  // Now check the alignement in track
  bool distanceInTrackIsSufficientlyLow =
    0 < startToMiddleInTrackDistance and startToMiddleInTrackDistance <= maxInTrackHitIdDifference and
    0 < middleToEndInTrackDistance and middleToEndInTrackDistance <= maxInTrackHitIdDifference;


  if (not distanceInTrackIsSufficientlyLow) return false;

  // Now check the right left information in track
  ERightLeft startRLInfo = rlWireHitTriple.getStartRLInfo();
  ERightLeft middleRLInfo = rlWireHitTriple.getMiddleRLInfo();
  ERightLeft endRLInfo = rlWireHitTriple.getEndRLInfo();

  ERightLeft mcStartRLInfo = mcHitLookUp.getRLInfo(startWireHit.getHit());
  ERightLeft mcMiddleRLInfo = mcHitLookUp.getRLInfo(middleWireHit.getHit());
  ERightLeft mcEndRLInfo = mcHitLookUp.getRLInfo(endWireHit.getHit());

  if (startRLInfo == mcStartRLInfo and middleRLInfo == mcMiddleRLInfo and endRLInfo == mcEndRLInfo) {
    return true;
  } else {
    return false;
  }
}
