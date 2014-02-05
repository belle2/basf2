/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoFacet.h"

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRecoFacet)

CDCRecoFacet::CDCRecoFacet() :
  CDCRLWireHitTriple(nullptr, nullptr, nullptr),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  B2ERROR("CDCRecoFacet initialized with nullptr for all oriented wire hit");
}



CDCRecoFacet::CDCRecoFacet(
  const CDCRLWireHit* startRLWireHit,
  const CDCRLWireHit* middleRLWireHit,
  const CDCRLWireHit* endRLWireHit
) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  if (startRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as first oriented wire hit");
  if (middleRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as second oriented wire hit");
  if (endRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as third oriented wire hit");
  adjustLines();
}



CDCRecoFacet::CDCRecoFacet(
  const CDCRLWireHit* startRLWireHit,
  const CDCRLWireHit* middleRLWireHit,
  const CDCRLWireHit* endRLWireHit,
  const ParameterLine2D& startToMiddle,
  const ParameterLine2D& startToEnd,
  const ParameterLine2D& middleToEnd
) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(startToMiddle),
  m_startToEnd(startToEnd),
  m_middleToEnd(middleToEnd),
  m_automatonCell()
{
  if (startRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as first oriented wire hit");
  if (middleRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as second oriented wire hit");
  if (endRLWireHit == nullptr) B2ERROR("CDCRecoFacet initialized with nullptr as third oriented wire hit");
}



CDCRecoFacet CDCRecoFacet::reversed() const
{
  return CDCRecoFacet(
           &(CDCWireHitTopology::getInstance().getReverseOf(getEndRLWireHit())),
           &(CDCWireHitTopology::getInstance().getReverseOf(getMiddleRLWireHit())),
           &(CDCWireHitTopology::getInstance().getReverseOf(getStartRLWireHit()))
         );

}



void CDCRecoFacet::adjustLines() const
{

  m_startToMiddle = CDCRecoTangent::constructTouchingLine(
                      getStartWireHit().getRefPos2D(),
                      getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                      getMiddleWireHit().getRefPos2D(),
                      getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength()
                    );

  m_startToEnd = CDCRecoTangent::constructTouchingLine(
                   getStartWireHit().getRefPos2D(),
                   getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                   getEndWireHit().getRefPos2D(),
                   getEndRLInfo() * getEndWireHit().getRefDriftLength()
                 );

  m_middleToEnd = CDCRecoTangent::constructTouchingLine(
                    getMiddleWireHit().getRefPos2D(),
                    getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength() ,
                    getEndWireHit().getRefPos2D(),
                    getEndRLInfo() * getEndWireHit().getRefDriftLength()
                  );

}

FloatType CDCRecoFacet::getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
{

  FloatType distance = trajectory2D.getDist2D(getStartRefPos2D());
  FloatType result = distance * distance;

  distance = trajectory2D.getDist2D(getMiddleRefPos2D());
  result += distance * distance;

  distance = trajectory2D.getDist2D(getEndRefPos2D());
  result += distance * distance;

  return result;

}

