/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRLWireHitTriple.h"

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCRLWireHitTriple)



CDCRLWireHitTriple::CDCRLWireHitTriple():
  m_startRLWireHit(nullptr),
  m_rearRLWireHitPair(nullptr, nullptr)
{
  B2WARNING("CDCRLWireHitTiple initialized with three nullptr");
}



CDCRLWireHitTriple::CDCRLWireHitTriple(
  const CDCRLWireHit* startRLWireHit,
  const CDCRLWireHit* middleRLWireHit,
  const CDCRLWireHit* endRLWireHit
):
  m_startRLWireHit(startRLWireHit),
  m_rearRLWireHitPair(middleRLWireHit, endRLWireHit)
{
  if (startRLWireHit == nullptr) { B2ERROR("CDCRLWireHitPair initialized with nullptr as first oriented wire hit"); }
  if (middleRLWireHit == nullptr) { B2ERROR("CDCRLWireHitPair initialized with nullptr as second oriented wire hit"); }
  if (endRLWireHit == nullptr) { B2ERROR("CDCRLWireHitPair initialized with nullptr as third oriented wire hit"); }
}


CDCRLWireHitTriple CDCRLWireHitTriple::reversed() const
{
  return CDCRLWireHitTriple(
           CDCWireHitTopology::getInstance().getReverseOf(getEndRLWireHit()),
           CDCWireHitTopology::getInstance().getReverseOf(getMiddleRLWireHit()),
           CDCWireHitTopology::getInstance().getReverseOf(getStartRLWireHit())
         );
}



void CDCRLWireHitTriple::reverse()
{

  const CDCRLWireHit* newStartRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getEndRLWireHit());
  const CDCRLWireHit* newMiddleRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getMiddleRLWireHit());
  const CDCRLWireHit* newEndRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getStartRLWireHit());

  setStartRLWireHit(newStartRLWireHit);
  setMiddleRLWireHit(newMiddleRLWireHit);
  setEndRLWireHit(newEndRLWireHit);

}



void CDCRLWireHitTriple::setStartRLInfo(const RightLeftInfo& startRLInfo)
{

  if (startRLInfo != getStartRLInfo()) {
    const CDCRLWireHit* newStartRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getStartRLWireHit());
    setStartRLWireHit(newStartRLWireHit);
  }

}



void CDCRLWireHitTriple::setMiddleRLInfo(const RightLeftInfo& middleRLInfo)
{
  getRearRLWireHitPair().setFromRLInfo(middleRLInfo);
}



void CDCRLWireHitTriple::setEndRLInfo(const RightLeftInfo& endRLInfo)
{
  getRearRLWireHitPair().setToRLInfo(endRLInfo);
}



CDCRLWireHitTriple::Shape CDCRLWireHitTriple::getShape() const
{

  const CDCWire& startWire = getStartWire();
  const CDCWire& middleWire = getMiddleWire();
  const CDCWire& endWire = getEndWire();

  WireNeighborType middleToStartNeighborType = middleWire.isNeighborWith(startWire);
  WireNeighborType middleToEndNeighborType   = middleWire.isNeighborWith(endWire);

  if (middleToStartNeighborType and middleToEndNeighborType) {
    if (startWire.isNeighborWith(endWire)) return ORTHO;
    else if (abs(middleToStartNeighborType - middleToEndNeighborType) == 6) return PARA;
    else return META;
  }
  //else
  return Shape::ILLSHAPED;
}




