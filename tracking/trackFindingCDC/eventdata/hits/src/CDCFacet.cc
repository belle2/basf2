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

using namespace Belle2;
using namespace TrackFindingCDC;

CDCFacet::CDCFacet(const CDCRLWireHit& startRLWireHit,
                   const CDCRLWireHit& middleRLWireHit,
                   const CDCRLWireHit& endRLWireHit)
  : CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
    m_fitLine(),
    m_automatonCell()
{
  adjustFitLine();
}

CDCFacet::CDCFacet(const CDCRLWireHit& startRLWireHit,
                   const CDCRLWireHit& middleRLWireHit,
                   const CDCRLWireHit& endRLWireHit,
                   const UncertainParameterLine2D& fitLine)
  : CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
    m_fitLine(fitLine),
    m_automatonCell()
{
}

void CDCFacet::reverse()
{
  CDCRLWireHitTriple::reverse();
  m_fitLine.reverse();
}

CDCFacet CDCFacet::reversed() const
{
  return CDCFacet(getEndRLWireHit().reversed(),
                  getMiddleRLWireHit().reversed(),
                  getStartRLWireHit().reversed(),
                  m_fitLine.reversed());
}

void CDCFacet::adjustFitLine() const
{
  m_fitLine = UncertainParameterLine2D(getStartToEndLine());
}

void CDCFacet::invalidateFitLine()
{
  m_fitLine.invalidate();
}

ParameterLine2D CDCFacet::getStartToMiddleLine() const
{
  return ParameterLine2D::touchingCircles(getStartRLWireHit().getRefPos2D(),
                                          getStartRLWireHit().getSignedRefDriftLength() ,
                                          getMiddleRLWireHit().getRefPos2D(),
                                          getMiddleRLWireHit().getSignedRefDriftLength());
}

ParameterLine2D CDCFacet::getStartToEndLine() const
{
  return ParameterLine2D::touchingCircles(getStartRLWireHit().getRefPos2D(),
                                          getStartRLWireHit().getSignedRefDriftLength() ,
                                          getEndRLWireHit().getRefPos2D(),
                                          getEndRLWireHit().getSignedRefDriftLength());
}

ParameterLine2D CDCFacet::getMiddleToEndLine() const
{
  return ParameterLine2D::touchingCircles(getMiddleRLWireHit().getRefPos2D(),
                                          getMiddleRLWireHit().getSignedRefDriftLength() ,
                                          getEndRLWireHit().getRefPos2D(),
                                          getEndRLWireHit().getSignedRefDriftLength());
}
