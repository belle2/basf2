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

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCWireHit)

namespace {
  // Setup instance for the tdc count translation

  SimpleTDCCountTranslator* s_simpleTDCCountTranslator = nullptr;
  SimpleTDCCountTranslator& getSimpleTDCCountTranslatorInstance()
  {
    if (not s_simpleTDCCountTranslator) s_simpleTDCCountTranslator = new SimpleTDCCountTranslator();
    return *s_simpleTDCCountTranslator;
  }



  RealisticTDCCountTranslator* s_realisticTDCCountTranslator = nullptr;

  // Declare function as currently unused to avoid compiler warning
  RealisticTDCCountTranslator& getRealisticTDCCountTranslatorInstance()  __attribute__((__unused__));

  RealisticTDCCountTranslator& getRealisticTDCCountTranslatorInstance()
  {
    if (not s_realisticTDCCountTranslator) s_realisticTDCCountTranslator = new RealisticTDCCountTranslator();
    return *s_realisticTDCCountTranslator;
  }


  TDCCountTranslatorBase& getTDCCountTranslatorInstance()
  { return getSimpleTDCCountTranslatorInstance(); }
  //{ return getRealisticTDCCountTranslatorInstance(); }

}



CDCWireHit::CDCWireHit():
  m_wire(&(CDCWire::getLowest())),
  m_hit(nullptr),
  m_refDriftLength(0.0),
  m_refDriftLengthVariance(0.0),
  m_automatonCell(1)
{;}

CDCWireHit::CDCWireHit(const CDCWire* wire):
  m_wire(wire),
  m_hit(nullptr),
  m_refDriftLength(0.0),
  m_refDriftLengthVariance(0.0),
  m_automatonCell(1)
{;}

CDCWireHit::CDCWireHit(const CDCHit* ptrHit):
  m_wire(ptrHit ? CDCWire::getInstance(*ptrHit) : nullptr),
  m_hit(ptrHit),
  m_refDriftLength(0.0),
  m_refDriftLengthVariance(0.0),
  m_automatonCell(1)
{
  if (not ptrHit) {
    B2ERROR("CDCWireHit constructor invoked with nullptr CDCHit");
    return;
  }
  const CDCHit& hit = *ptrHit;

  TDCCountTranslatorBase& translator = getTDCCountTranslatorInstance();

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
                             -999.9, // meaning unclear ?
                             -999.9 // meaning unclear ?
                                                                );


}





CDCWireHit::CDCWireHit(const WireID& wireID, const FloatType& driftLength):
  m_wire(CDCWire::getInstance(wireID)),
  m_hit(nullptr),
  m_refDriftLength(driftLength),
  m_refDriftLengthVariance(getSimpleTDCCountTranslatorInstance().getDriftLengthResolution(driftLength, wireID, false, NAN, NAN)),
  m_automatonCell(1)
{;}


CDCWireHit::~CDCWireHit() {;}






