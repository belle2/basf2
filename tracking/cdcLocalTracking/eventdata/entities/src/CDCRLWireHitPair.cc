/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRLWireHitPair.h"

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRLWireHitPair)

CDCRLWireHitPair::CDCRLWireHitPair():
  m_fromRLWireHit(nullptr),
  m_toRLWireHit(nullptr)
{
  B2ERROR("CDCRLWireHitPair initialized with two nullptr");
}



CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLWireHitPair& rlWireHitPair):
  CDCRLWireHitPair(rlWireHitPair.m_fromRLWireHit, rlWireHitPair.m_toRLWireHit)
{

}



CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit):
  m_fromRLWireHit(fromRLWireHit),
  m_toRLWireHit(toRLWireHit)
{
  if (fromRLWireHit == nullptr) { B2ERROR("CDCRLWireHitPair initialized with nullptr as fromRLWireHit"); }
  if (toRLWireHit == nullptr) { B2ERROR("CDCRLWireHitPair initialized with nullptr as toRLWireHit"); }
}



CDCRLWireHitPair::~CDCRLWireHitPair()
{

}



CDCRLWireHitPair CDCRLWireHitPair::reversed() const
{

  const CDCRLWireHit& newFromRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getToRLWireHit());
  const CDCRLWireHit& newToRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getFromRLWireHit());

  return CDCRLWireHitPair(&newFromRLWireHit, &newToRLWireHit);

}



void CDCRLWireHitPair::reverse()
{

  const CDCRLWireHit& newFromRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getToRLWireHit());
  const CDCRLWireHit& newToRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getFromRLWireHit());

  setFromRLWireHit(&newFromRLWireHit);
  setToRLWireHit(&newToRLWireHit);

}


const CDCWireHit* CDCRLWireHitPair::commonWireHit(const CDCRLWireHitPair& rlWireHitPair) const
{

  if (rlWireHitPair.hasWireHit(getFromWireHit())) return &(getFromWireHit());
  else if (rlWireHitPair.hasWireHit(getToWireHit())) return &(getToWireHit());
  else return nullptr;

}



void CDCRLWireHitPair::setFromRLInfo(const RightLeftInfo& fromRLInfo)
{

  if (fromRLInfo != getFromRLInfo()) {
    const CDCRLWireHit& newFromRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getFromRLWireHit());
    m_fromRLWireHit = &newFromRLWireHit;
  }

}



void CDCRLWireHitPair::setToRLInfo(const RightLeftInfo& toRLInfo)
{

  if (toRLInfo != getToRLInfo()) {
    const CDCRLWireHit& newToRLWireHit = CDCWireHitTopology::getInstance().getReverseOf(getToRLWireHit());
    m_toRLWireHit = &newToRLWireHit;
  }

}



