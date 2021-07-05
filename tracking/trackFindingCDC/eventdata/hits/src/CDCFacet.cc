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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

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

Vector2D CDCFacet::getStartRecoPos2D() const
{
  return getFitLine()->closest(getStartWire().getRefPos2D());
}

Vector2D CDCFacet::getMiddleRecoPos2D() const
{
  return getFitLine()->closest(getMiddleWire().getRefPos2D());
}

Vector2D CDCFacet::getEndRecoPos2D() const
{
  return getFitLine()->closest(getEndWire().getRefPos2D());
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

CDCRecoHit2D CDCFacet::getStartRecoHit2D() const
{
  return CDCRecoHit2D::fromRecoPos2D(getStartRLWireHit(), getStartRecoPos2D());
}

CDCRecoHit2D CDCFacet::getMiddleRecoHit2D() const
{
  return CDCRecoHit2D::fromRecoPos2D(getMiddleRLWireHit(), getMiddleRecoPos2D());
}

CDCRecoHit2D CDCFacet::getEndRecoHit2D() const
{
  return CDCRecoHit2D::fromRecoPos2D(getEndRLWireHit(), getEndRecoPos2D());
}

CDCTangent CDCFacet::getStartToMiddle() const
{
  return CDCTangent(getStartRLWireHit(), getEndRLWireHit(), getStartToMiddleLine());
}

CDCTangent CDCFacet::getStartToEnd() const
{
  return CDCTangent(getStartRLWireHit(), getEndRLWireHit(), getStartToEndLine());
}

CDCTangent CDCFacet::getMiddleToEnd() const
{
  return CDCTangent(getMiddleRLWireHit(), getEndRLWireHit(), getMiddleToEndLine());
}

void CDCFacet::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  getStartWireHit().getAutomatonCell().unsetMaskedFlag();
  getMiddleWireHit().getAutomatonCell().unsetMaskedFlag();
  getEndWireHit().getAutomatonCell().unsetMaskedFlag();
}

void CDCFacet::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  getStartWireHit().getAutomatonCell().setMaskedFlag();
  getMiddleWireHit().getAutomatonCell().setMaskedFlag();
  getEndWireHit().getAutomatonCell().setMaskedFlag();
}

void CDCFacet::receiveMaskedFlag() const
{
  if (getStartWireHit().getAutomatonCell().hasMaskedFlag() or
      getMiddleWireHit().getAutomatonCell().hasMaskedFlag() or
      getEndWireHit().getAutomatonCell().hasMaskedFlag()) {

    getAutomatonCell().setMaskedFlag();
  }
}
