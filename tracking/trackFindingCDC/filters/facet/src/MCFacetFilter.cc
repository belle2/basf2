/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCFacetFilter.h"

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CellWeight MCFacetFilter::isGoodFacet(const CDCRecoFacet& facet)
{
  bool isCorrectFacet = isCorrect(facet, 3);
  bool isCorrectReverseFacet = m_allowReverse and isCorrect(facet.reversed(), 3);

  if (isCorrectFacet or isCorrectReverseFacet) {
    facet.adjustLines();
    return 3.0;
  } else {
    return NOT_A_CELL;
  }
}

bool MCFacetFilter::isCorrect(const CDCRLWireHitTriple& rlWireHitTriple, int inTrackHitDistanceTolerance) const
{

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCWireHit& startWireHit = rlWireHitTriple.getStartWireHit();
  const CDCWireHit& middleWireHit = rlWireHitTriple.getMiddleWireHit();
  const CDCWireHit& endWireHit = rlWireHitTriple.getEndWireHit();

  //First check if the track ids are in agreement
  ITrackType startMCTrackId = mcHitLookUp.getMCTrackId(startWireHit);
  ITrackType middleMCTrackId = mcHitLookUp.getMCTrackId(middleWireHit);
  ITrackType endMCTrackId = mcHitLookUp.getMCTrackId(endWireHit);

  if (not(startMCTrackId == middleMCTrackId and middleMCTrackId == endMCTrackId) or startMCTrackId == INVALID_ITRACK) {
    return false;
  }



  //Maybe be a bit more permissive for reassigned hits
  bool startIsReassigned = mcHitLookUp.isReassignedSecondary(startWireHit);
  bool middleIsReassigned = mcHitLookUp.isReassignedSecondary(middleWireHit);
  bool endIsReassigned = mcHitLookUp.isReassignedSecondary(endWireHit);



  //Now check the alignement in track
  bool distanceInTrackIsSufficientlyLow = true;

  int startInTrackId = mcHitLookUp.getInTrackId(startWireHit);
  int middleInTrackId = mcHitLookUp.getInTrackId(middleWireHit);
  int endInTrackId = mcHitLookUp.getInTrackId(endWireHit);

  int startToMiddleInTrackDistance =  middleInTrackId - startInTrackId;
  int middleToEndInTrackDistance =  endInTrackId - middleInTrackId;
  int startToEndInTrackDistance =  endInTrackId - startInTrackId;

  if (true or (not startIsReassigned and not middleIsReassigned and not endIsReassigned)) {

    distanceInTrackIsSufficientlyLow =
      0 < startToMiddleInTrackDistance and startToMiddleInTrackDistance <= inTrackHitDistanceTolerance and
      0 < middleToEndInTrackDistance and middleToEndInTrackDistance <= inTrackHitDistanceTolerance;

    // if (not distanceInTrackIsSufficientlyLow){
    //   B2WARNING("Disagreement here");
    //   B2WARNING("startToMiddleInTrackDistance: " <<  startToMiddleInTrackDistance);
    //   B2WARNING("middleToEndInTrackDistance: " <<  middleToEndInTrackDistance);
    // }

  } else if (not startIsReassigned and not middleIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < startToMiddleInTrackDistance and startToMiddleInTrackDistance <= inTrackHitDistanceTolerance;

  } else if (not middleIsReassigned and not endIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < middleToEndInTrackDistance and middleToEndInTrackDistance <= inTrackHitDistanceTolerance;

  } else if (not startIsReassigned and not endIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < startToEndInTrackDistance and  startToEndInTrackDistance <= 2 * inTrackHitDistanceTolerance;

  } else {

    // can not say anything about the in

  }

  if (not distanceInTrackIsSufficientlyLow) return false;


  //Now check the right left information in track
  RightLeftInfo startRLInfo = rlWireHitTriple.getStartRLInfo();
  RightLeftInfo middleRLInfo = rlWireHitTriple.getMiddleRLInfo();
  RightLeftInfo endRLInfo = rlWireHitTriple.getEndRLInfo();


  RightLeftInfo mcStartRLInfo = mcHitLookUp.getRLInfo(startWireHit);
  RightLeftInfo mcMiddleRLInfo = mcHitLookUp.getRLInfo(middleWireHit);
  RightLeftInfo mcEndRLInfo = mcHitLookUp.getRLInfo(endWireHit);



  if (startRLInfo == mcStartRLInfo and middleRLInfo == mcMiddleRLInfo and endRLInfo == mcEndRLInfo) {
    //return true;
  } else {
    return false;
  }



  const CDCSimHit* startSimHit = mcHitLookUp.getSimHit(startWireHit);
  const CDCSimHit* middleSimHit = mcHitLookUp.getSimHit(middleWireHit);
  const CDCSimHit* endSimHit = mcHitLookUp.getSimHit(endWireHit);

  if (not startSimHit) {
    B2WARNING("Start wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }
  if (not middleSimHit) {
    B2WARNING("Middle wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }
  if (not startSimHit) {
    B2WARNING("End wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }

  CDCRecoHit2D startMCHit =  CDCRecoHit2D::fromSimHit(&startWireHit, *startSimHit);
  CDCRecoHit2D middleMCHit = CDCRecoHit2D::fromSimHit(&middleWireHit, *middleSimHit);
  CDCRecoHit2D endMCHit = CDCRecoHit2D::fromSimHit(&endWireHit, *endSimHit);

  CDCRecoTangent startToMiddleMCTangent(startMCHit, middleMCHit);
  CDCRecoTangent startToEndMCTangent(startMCHit, endMCHit);
  CDCRecoTangent middleToEndMCTangent(middleMCHit, endMCHit);

  //the right left passage info from mc often is not correct, because the simhits offer poor momentum information (therefor limites flight direction information)
  startToMiddleMCTangent.adjustRLInfo();
  startToEndMCTangent.adjustRLInfo();
  middleToEndMCTangent.adjustRLInfo();

  //check if the tangents have matching right left passage information in their own recohits
  if (startToEndMCTangent.getFromRLInfo() !=
      startToMiddleMCTangent.getFromRLInfo() or

      startToMiddleMCTangent.getToRLInfo() !=
      middleToEndMCTangent.getFromRLInfo() or

      startToEndMCTangent.getToRLInfo() !=
      middleToEndMCTangent.getToRLInfo()) {

    return false;
  }

  if (startWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getStartRLInfo() or
        startToEndMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getStartRLInfo()) return false;
  }

  if (middleWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getToRLInfo() !=
        rlWireHitTriple.getMiddleRLInfo() or

        middleToEndMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getMiddleRLInfo()) return false;
  }

  if (endWireHit->getRefDriftLength() != 0) {
    if (startToEndMCTangent.getToRLInfo() !=
        rlWireHitTriple.getEndRLInfo() or

        middleToEndMCTangent.getToRLInfo() !=
        rlWireHitTriple.getEndRLInfo()) return false;
  }

  return true;

}
