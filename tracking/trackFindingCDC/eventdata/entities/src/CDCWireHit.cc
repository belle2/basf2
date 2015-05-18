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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

TDCCountTranslatorBase& CDCWireHit::getTDCCountTranslator()
{
  return CDCWireHitTopology::getInstance().getTDCCountTranslator();
}




CDCWireHit::CDCWireHit() :
  CDCWireHit(&(CDCWire::getLowest()))
{;}

CDCWireHit::CDCWireHit(const CDCWire* wire):
  m_automatonCell(1),
  m_refDriftLength(0.0),
  m_refDriftLengthVariance(0.0),
  m_wire(wire),
  m_hit(nullptr)
{;}

CDCWireHit::CDCWireHit(const CDCHit* ptrHit, TDCCountTranslatorBase* ptrTranslator):
  m_automatonCell(1),
  m_refDriftLength(0.0),
  m_refDriftLengthVariance(0.0),
  m_wire(ptrHit ? CDCWire::getInstance(*ptrHit) : nullptr),
  m_hit(ptrHit)
{
  if (not ptrHit) {
    B2ERROR("CDCWireHit constructor invoked with nullptr CDCHit");
    return;
  }
  const CDCHit& hit = *ptrHit;

  TDCCountTranslatorBase& translator = ptrTranslator ? *ptrTranslator : getTDCCountTranslator();

  float initialTOFEstimate = 0;

  // TODO: check left right correspondence to bool
  float refDriftLengthRight = translator.getDriftLength(hit.getTDCCount(),
                                                        getWireID(),
                                                        initialTOFEstimate,
                                                        false, //bool leftRight
                                                        getWire().getRefZ());

  float refDriftLengthLeft = translator.getDriftLength(hit.getTDCCount(),
                                                       getWireID(),
                                                       initialTOFEstimate,
                                                       true, //bool leftRight
                                                       getWire().getRefZ());


  m_refDriftLength = (refDriftLengthLeft + refDriftLengthRight) / 2.0;

  m_refDriftLengthVariance = translator.getDriftLengthResolution(m_refDriftLength,
                             getWireID(),
                             false, //bool leftRight ?
                             getWire().getRefZ());


}

CDCWireHit::CDCWireHit(const WireID& wireID, const FloatType& driftLength):
  m_automatonCell(1),
  m_refDriftLength(driftLength),
  m_refDriftLengthVariance(getTDCCountTranslator().getDriftLengthResolution(driftLength, wireID, false, NAN, NAN)),
  m_wire(CDCWire::getInstance(wireID)),
  m_hit(nullptr)
{;}
