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

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCHitLookUp.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

MCFacetFilter::MCFacetFilter() {;}

MCFacetFilter::~MCFacetFilter() {;}


CellState MCFacetFilter::isGoodFacet(const CDCRecoFacet& facet) const
{
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  bool isCorrectFacet = mcHitLookUp.isCorrect(facet, 3);

  //CellState oldCellState = oldIsGoodFacet(facet);

  //if ( (oldCellState == 3.0) xor isCorrectFacet){
  //  B2WARNING("New defintion disagrees");
  //}

  if (isCorrectFacet) {
    //if (oldCellState == 3.0) {
    facet.adjustLines();
    return 3;
  } else {
    return NOT_A_CELL;
  }

}







/*
CellState MCFacetFilter::oldIsGoodFacet(const CDCRecoFacet& facet) const
{
  const CDCWireHit* startWireHit  = &(facet.getStartWireHit());
  const CDCWireHit* middleWireHit = &(facet.getMiddleWireHit());
  const CDCWireHit* endWireHit    = &(facet.getEndWireHit());

  //the three hits should be in the right order tofwise
  //const Belle2::CDCSimHit * simStart = m_mcLookUp.getSimHit(startWireHit);
  //const Belle2::CDCSimHit * simMiddle = m_mcLookUp.getSimHit(middleWireHit);
  //const Belle2::CDCSimHit * simEnd = m_mcLookUp.getSimHit(endWireHit);

  {
    ITrackType iTrack = m_mcLookUp.getMajorMCTrackId(startWireHit, middleWireHit, endWireHit);
    if (iTrack == INVALID_ITRACK) {
      //B2DEBUG(200,"    Rejected facet because no major track id");
      return NOT_A_CELL;
    }
  }

  if (not m_mcLookUp.isTOFAligned(startWireHit, middleWireHit, endWireHit)) {

    //B2DEBUG(200,"    Rejected facet because wrong tof alignement");
    return NOT_A_CELL;

  }

  if (not m_mcLookUp.isAlignedInMCTrack(startWireHit, middleWireHit, endWireHit)) {

    //B2DEBUG(200,"    Rejected facet because wrong track id alignement");
    return NOT_A_CELL;

  }

  //check if the tangents match to the tangents containing the mc hits
  CDCRecoHit2D startMCHit = m_mcLookUp.getMCHit2D(startWireHit);
  CDCRecoHit2D middleMCHit = m_mcLookUp.getMCHit2D(middleWireHit);
  CDCRecoHit2D endMCHit = m_mcLookUp.getMCHit2D(endWireHit);

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

    //B2DEBUG("    Recofacet has no agreed passage info");

    return NOT_A_CELL;
  }

  //B2DEBUG(200,"MC Right left signature " <<
  //             startToEndMCTangent.getFromRecoHit().getRightLeftInfo() << " " <<
  //             startToMiddleMCTangent.getToRecoHit().getRightLeftInfo() << " " <<
  //             middleToEndMCTangent.getToRecoHit().getRightLeftInfo() );

  //B2DEBUG(200,"Right left signature " <<
  //             startToEndTangent.getFromRecoHit().getRightLeftInfo() << " " <<
  //             startToMiddleTangent.getToRecoHit().getRightLeftInfo() << " " <<
  //             middleToEndTangent.getToRecoHit().getRightLeftInfo() );


  if (startWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getFromRLInfo() !=
        facet.getStartRLInfo() or

        startToEndMCTangent.getFromRLInfo() !=
        facet.getStartRLInfo()) return NOT_A_CELL;
  }

  if (middleWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getToRLInfo() !=
        facet.getMiddleRLInfo() or

        middleToEndMCTangent.getFromRLInfo() !=
        facet.getMiddleRLInfo()) return NOT_A_CELL;
  }

  if (endWireHit->getRefDriftLength() != 0) {
    if (startToEndMCTangent.getToRLInfo() !=
        facet.getEndRLInfo() or

        middleToEndMCTangent.getToRLInfo() !=
        facet.getEndRLInfo()) return NOT_A_CELL;

  }

  return 3;

}
*/
void MCFacetFilter::clear() const {;}
