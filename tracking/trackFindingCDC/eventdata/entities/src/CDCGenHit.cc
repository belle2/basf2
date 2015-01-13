/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCGenHit.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCGenHit)

CDCGenHit::CDCGenHit():
  m_wire(&(CDCWire::getLowest())),
  m_dummyPos(0.0, 0.0)
{;}

CDCGenHit::CDCGenHit(const WireID& wireID, const Vector2D& dummyPos):
  m_wire(CDCWire::getInstance(wireID)),
  m_dummyPos(dummyPos)
{;}


CDCGenHit::~CDCGenHit()
{

}

