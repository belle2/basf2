/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCFacet::CDCFacet(const CDCRLTaggedWireHit& startRLWireHit,
                   const CDCRLTaggedWireHit& middleRLWireHit,
                   const CDCRLTaggedWireHit& endRLWireHit) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  adjustLines();
}

CDCFacet::CDCFacet(const CDCRLTaggedWireHit& startRLWireHit,
                   const CDCRLTaggedWireHit& middleRLWireHit,
                   const CDCRLTaggedWireHit& endRLWireHit,
                   const ParameterLine2D& startToMiddle,
                   const ParameterLine2D& startToEnd,
                   const ParameterLine2D& middleToEnd) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(startToMiddle),
  m_startToEnd(startToEnd),
  m_middleToEnd(middleToEnd),
  m_automatonCell()
{
}

CDCFacet CDCFacet::reversed() const
{
  return CDCFacet(getEndRLWireHit().reversed(),
                  getMiddleRLWireHit().reversed(),
                  getStartRLWireHit().reversed(),
                  ParameterLine2D::throughPoints(getEndRecoPos2D(), getMiddleRecoPos2D()),
                  ParameterLine2D::throughPoints(getEndRecoPos2D(), getStartRecoPos2D()),
                  ParameterLine2D::throughPoints(getMiddleRecoPos2D(), getStartRecoPos2D()));
}

void CDCFacet::adjustLines() const
{
  m_startToMiddle =
    CDCTangent::constructTouchingLine(getStartWireHit().getRefPos2D(),
                                      getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                                      getMiddleWireHit().getRefPos2D(),
                                      getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength());

  m_startToEnd =
    CDCTangent::constructTouchingLine(getStartWireHit().getRefPos2D(),
                                      getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                                      getEndWireHit().getRefPos2D(),
                                      getEndRLInfo() * getEndWireHit().getRefDriftLength());

  m_middleToEnd =
    CDCTangent::constructTouchingLine(getMiddleWireHit().getRefPos2D(),
                                      getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength() ,
                                      getEndWireHit().getRefPos2D(),
                                      getEndRLInfo() * getEndWireHit().getRefDriftLength());
}

void CDCFacet::invalidateLines()
{
  m_startToMiddle.invalidate();
  m_startToEnd.invalidate();
  m_middleToEnd.invalidate();
}
