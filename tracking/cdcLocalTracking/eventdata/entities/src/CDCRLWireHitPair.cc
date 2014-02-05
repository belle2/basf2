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

