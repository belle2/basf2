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

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRecoFacet)

CDCRecoFacet::CDCRecoFacet() :
  m_start(&(CDCWireHit::getLowest()) , LEFT),
  m_middle(&(CDCWireHit::getLowest()) , LEFT),
  m_end(&(CDCWireHit::getLowest()) , LEFT),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{;}

CDCRecoFacet::CDCRecoFacet(
  const CDCWireHit* startWireHit,  const RightLeftInfo& startRLInfo,
  const CDCWireHit* middleWireHit, const RightLeftInfo& middleRLInfo,
  const CDCWireHit* endWireHit,    const RightLeftInfo& endRLInfo
) :
  m_start(startWireHit, startRLInfo),
  m_middle(middleWireHit, middleRLInfo),
  m_end(endWireHit, endRLInfo),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  if (startWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
  if (middleWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
  if (endWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
  adjustLines();
}

CDCRecoFacet::CDCRecoFacet(
  const CDCWireHit* startWireHit,  const RightLeftInfo& startRLInfo,
  const CDCWireHit* middleWireHit, const RightLeftInfo& middleRLInfo,
  const CDCWireHit* endWireHit,    const RightLeftInfo& endRLInfo,
  const ParameterLine2D& startToMiddle,
  const ParameterLine2D& startToEnd,
  const ParameterLine2D& middleToEnd
) :
  m_start(startWireHit, startRLInfo),
  m_middle(middleWireHit, middleRLInfo),
  m_end(endWireHit, endRLInfo),
  m_startToMiddle(startToMiddle),
  m_startToEnd(startToEnd),
  m_middleToEnd(middleToEnd),
  m_automatonCell()
{
  if (startWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
  if (middleWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
  if (endWireHit == nullptr) B2WARNING("Recohit with nullptr as wire hit");
}


CDCRecoFacet::Shape
CDCRecoFacet::getShape(
  const CDCWire& startWire,
  const CDCWire& middleWire,
  const CDCWire& endWire
)
{

  WireNeighborType middleToStartNeighborType = middleWire.isNeighborWith(startWire);
  WireNeighborType middleToEndNeighborType   = middleWire.isNeighborWith(endWire);

  if (middleToStartNeighborType and middleToEndNeighborType) {
    if (startWire.isNeighborWith(endWire)) return ORTHO;
    else if (abs(middleToStartNeighborType - middleToEndNeighborType) == 6) return PARA;
    else return META;
  }
  //else
  return ILLSHAPED;
}


ILayerType CDCRecoFacet::getISuperLayer() const
{

  const ILayerType iSuperLayer = getStartWireHit()->getISuperLayer();

  return (iSuperLayer == getMiddleWireHit()->getISuperLayer() and
          iSuperLayer == getEndWireHit()->getISuperLayer()) ?
         iSuperLayer :
         INVALIDSUPERLAYER;

}


void CDCRecoFacet::adjustLines() const
{

  m_startToMiddle = CDCRecoTangent::constructTouchingLine(
                      getStartWireHit()->getRefPos2D(),
                      getStartRLInfo() * getStartWireHit()->getRefDriftLength() ,
                      getMiddleWireHit()->getRefPos2D(),
                      getMiddleRLInfo() * getMiddleWireHit()->getRefDriftLength()
                    );

  m_startToEnd = CDCRecoTangent::constructTouchingLine(
                   getStartWireHit()->getRefPos2D(),
                   getStartRLInfo() * getStartWireHit()->getRefDriftLength() ,
                   getEndWireHit()->getRefPos2D(),
                   getEndRLInfo() * getEndWireHit()->getRefDriftLength()
                 );

  m_middleToEnd = CDCRecoTangent::constructTouchingLine(
                    getMiddleWireHit()->getRefPos2D(),
                    getMiddleRLInfo() * getMiddleWireHit()->getRefDriftLength() ,
                    getEndWireHit()->getRefPos2D(),
                    getEndRLInfo() * getEndWireHit()->getRefDriftLength()
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




/*
CDCRecoFacet::CDCRecoFacet() {
}


CDCRecoFacet::CDCRecoFacet(const CDCRecoTangent & startToMiddle,
                           const CDCRecoTangent & startToEnd,
                           const CDCRecoTangent & middleToEnd, bool checked) : m_cellState(0){

  m_recotangents[START  + MIDDLE -1] = startToMiddle;
  m_recotangents[START  + END -1]    = startToEnd;
  m_recotangents[MIDDLE + END -1]    = middleToEnd;


  //check if the tangents are alright
  if (checked) {
    if ( not CDCWireHit::ptrIsEqual(startToMiddle.getFromWireHit(),startToEnd.getFromWireHit()) ){
      B2ERROR("startToMiddle and startToEnd tangent do not have the same wirehit on start" );
      m_type = ILLSHAPED;
    } else if (  not CDCWireHit::ptrIsEqual(startToMiddle.getToWireHit(),middleToEnd.getFromWireHit()) ){
      B2ERROR("startToMiddle and middleToEnd tangent do not have the same wirehit on middle" );
      m_type = ILLSHAPED;
    } else if (  not CDCWireHit::ptrIsEqual(startToEnd.getToWireHit(),middleToEnd.getToWireHit()) ){
      B2ERROR("startToEnd and middleToEnd tangent do not have the same wirehit on end" );
      m_type = ILLSHAPED;
    } else {
      m_type = resultingType(startToMiddle.getFromRecoHit(),
                       middleToEnd.getFromRecoHit(),
                       startToEnd.getToRecoHit() );
  } else {
    //uncheck Facets are illshaped be default
    m_type = ILLSHAPED;
  }

  //m_recohits[START]  = CDCRecoHit2D::average(startToMiddle.getFromRecoHit(),startToEnd.getFromRecoHit() );
  //m_recohits[MIDDLE] = CDCRecoHit2D::average(startToMiddle.getToRecoHit()  ,middleToEnd.getFromRecoHit() );
  //m_recohits[END]    = CDCRecoHit2D::average(startToEnd.getToRecoHit()     ,middleToEnd.getToRecoHit() );

}

// Destructor.
CDCRecoFacet::~CDCRecoFacet() {
}


CDCRecoFacet::Type CDCRecoFacet::resultingType( const CDCWire & startWire, const CDCWire & middleWire, const CDCWire & endWire ){

  CDCWire::NeighborType middleToStartNeighborType = middleWire.hasNeighbor(startWire);
  CDCWire::NeighborType middleToEndNeighborType   = middleWire.hasNeighbor(endWire);

  if ( middleToStartNeighborType and middleToEndNeighborType ){
    if ( startWire.hasNeighbor(endWire) ) return ORTHO;
    else if ( abs(middleToStartNeighborType - middleToEndNeighborType) == 6 ) return PARA;
    else return META;
  }
  //else
  return ILLSHAPED;
}

CDCRecoFacet::Type CDCRecoFacet::resultingType( const CDCWireHit & startWireHit,
                                                const CDCWireHit & middleWireHit,
                                                const CDCWireHit & endWireHit ){

  const CDCWire * startWire = startWireHit.getWire();
  const CDCWire * middleWire = middleWireHit.getWire();
  const CDCWire * endWire = endWireHit.getWire();
  return ( startWire == nullptr or middleWire == nullptr or endWire == nullptr) ?
           ILLSHAPED : resultingType(*startWire,*middleWire,*endWire);
}

CDCRecoFacet::Type CDCRecoFacet::resultingType( const CDCRecoHit2D & startRecoHit,
                                                const CDCRecoHit2D & middleRecoHit,
                                                const CDCRecoHit2D & endRecoHit ){

  const CDCWire * startWire = startRecoHit.getWire();
  const CDCWire * middleWire = middleRecoHit.getWire();
  const CDCWire * endWire = endRecoHit.getWire();
  return ( startWire == nullptr or middleWire == nullptr or endWire == nullptr) ?
           ILLSHAPED : resultingType(*startWire,*middleWire,*endWire);
}

*/

// Implement all functions

