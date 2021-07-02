/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationVector.h>


using namespace std;
using namespace Belle2;

CDCHit::CDCHit(unsigned short tdcCount, unsigned short charge,
               unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire, unsigned short status, unsigned short tot,
               signed short otherHitIndex, unsigned short leadingEdgeCharge)
{
  setTDCCount(tdcCount);
  setADCCount(charge);
  setWireID(iSuperLayer, iLayer, iWire);
  setStatus(status);
  setTOT(tot);
  setOtherHitIndex(otherHitIndex);
  setADCCountAtLeadingEdge(leadingEdgeCharge);
}


DigitBase::EAppendStatus CDCHit::addBGDigit(const DigitBase* bg)
{
  const auto* bgDigit = static_cast<const CDCHit*>(bg);
  const unsigned short tdc4Sg = m_tdcCount;
  const unsigned short adc4Sg = m_adcCount;
  const unsigned short tot4Sg = m_tot;
  const unsigned short tdc4Bg = bgDigit->getTDCCount();
  const unsigned short adc4Bg = bgDigit->getADCCount();
  const unsigned short tot4Bg = bgDigit->getTOT();
  //  B2DEBUG(28, "Sg tdc,adc,tot= " << tdc4Sg << " " << adc4Sg << " " << tot4Sg);
  //  B2DEBUG(28, "Bg tdc,adc,tot= " << tdc4Bg << " " << adc4Bg << " " << tot4Bg);
  int diff = static_cast<int>(m_tdcCount) - static_cast<int>(bgDigit->getTDCCount());

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

  m_adcCount = adc4Sg + adc4Bg;

  //Set TOT for signal+background case. It is assumed that the start timing
  //of a pulse (input to ADC) is given by the TDC-count. This is an
  //approximation becasue analog (for ADC) and digital (for TDC) parts are
  //different in the front-end electronics.
  unsigned short s1 = tdc4Sg; //start time of 1st pulse
  unsigned short s2 = tdc4Bg; //start time of 2nd pulse
  unsigned short w1 = 32 * tot4Sg; //its width
  unsigned short w2 = 32 * tot4Bg; //its width
  if (tdc4Sg < tdc4Bg) {
    s1 = tdc4Bg;
    w1 = 32 * tot4Bg;
    s2 = tdc4Sg;
    w2 = 32 * tot4Sg;
  }
  const unsigned short e1 = s1 - w1; //end time of 1st pulse
  const unsigned short e2 = s2 - w2; //end time of 2nd pulse
  //  B2DEBUG(28, "s1,e1,w1,s2,e2,w2= " << s1 << " " << e1 << " " << w1 << " " << s2 << " " << e2 << " " << w2);

  double pulseW = w1 + w2;
  if (e1 <= e2) {
    pulseW = w1;
  } else if (e1 <= s2) {
    pulseW = s1 - e2;
  }

  // maxtot=29 is hard-coded now
  m_tot = std::min(std::round(pulseW / 32.), 29.);
  // B2DEBUG(28, "adcCount,tot= " << m_adcCount << " " << m_tot);

  return DigitBase::c_DontAppend;
}
