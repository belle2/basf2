/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, CDC group                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationVector.h>


using namespace std;
using namespace Belle2;

CDCHit::CDCHit(unsigned short tdcCount, unsigned short charge,
               unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire, unsigned short status, signed short otherHitIndex,
               unsigned short leadingEdgeCharge)
{
  setTDCCount(tdcCount);
  setADCCount(charge);
  setWireID(iSuperLayer, iLayer, iWire);
  setStatus(status);
  setOtherHitIndex(otherHitIndex);
  setADCCountAtLeadingEdge(leadingEdgeCharge);
}


DigitBase::EAppendStatus CDCHit::addBGDigit(const DigitBase* bg)
{
  const auto* bgDigit = static_cast<const CDCHit*>(bg);
  const unsigned short adc = m_adcCount;
  int diff  = static_cast<int>(m_tdcCount) - static_cast<int>(bgDigit->getTDCCount());

  // If the BG hit is faster than the true hit, the TDC count is replaced, and
  // relation is removed.
  // ADC counts are summed up.
  if (diff < 0) {
    *this = *bgDigit;
    auto relSimHits = this->getRelationsFrom<CDCSimHit>();
    for (int i = relSimHits.size() - 1; i >= 0; --i) {
      relSimHits.remove(i);
    }
    auto relMCParticles = this->getRelationsFrom<MCParticle>();
    for (int i = relMCParticles.size() - 1; i >= 0; --i) {
      relMCParticles.remove(i);
    }
  }
  m_adcCount += adc;


  return DigitBase::c_DontAppend;

}
