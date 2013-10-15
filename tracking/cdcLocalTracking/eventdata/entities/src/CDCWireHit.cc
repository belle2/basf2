/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCWireHit.h"


#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCWireHit)

CDCWireHit::CDCWireHit():
  AutomatonCell(1),
  m_wire(&(CDCWire::getLowest())),
  m_hit(nullptr), m_iHit(-1),
  m_refDriftLength(0)
{;}

CDCWireHit::CDCWireHit(const CDCWire* wire):
  AutomatonCell(1),
  m_wire(wire),
  m_hit(nullptr),
  m_iHit(-1),
  m_refDriftLength(0.0)
{;}

CDCWireHit::CDCWireHit(const CDCHit* hit, int iHit):
  AutomatonCell(1),
  m_wire(CDCWire::getInstance(*hit)),
  m_hit(hit),
  m_iHit(iHit),
  m_refDriftLength(0.0)
{
  m_refDriftLength = CDCWireHit::TDCCountTranslatorInstance().getDriftLength(hit->getTDCCount());
  //m_refDriftLength = CDCWireHit::TDCCountTranslatorInstance().getDriftLength(hit->getTDCCount(),
  //                                                                           getWire().getWireID(),
  //                                                                           0,
  //                                                                           false,
  //                                                                           getWire().getRefZ() ) ;
}

CDCWireHit::CDCWireHit(const WireID& wireID, const FloatType& driftLength):
  AutomatonCell(1),
  m_wire(CDCWire::getInstance(wireID)),
  m_hit(nullptr),
  m_iHit(-1),
  m_refDriftLength(driftLength)
{;}

/** Destructor. */
CDCWireHit::~CDCWireHit() {;}

const CDCWireHit* CDCWireHit::s_lowest = nullptr;


CDCWireHit::TDCCountTranslator* CDCWireHit::m_tdcCountTranslator = nullptr;



