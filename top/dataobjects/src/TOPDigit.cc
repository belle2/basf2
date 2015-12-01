/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;

// ClassImp(TOPDigit);

int TOPDigit::s_doubleHitResolution = 0;
int TOPDigit::s_pileupTime = 0;


DigitBase::EAppendStatus TOPDigit::addBGDigit(const DigitBase* bg)
{
  const auto* bgDigit = static_cast<const TOPDigit*>(bg);
  int diff = m_TDC - bgDigit->getTDC();

  if (abs(diff) > s_doubleHitResolution) return DigitBase::c_Append; // no pile-up

  if (abs(diff) < s_pileupTime) { // pile-up results in time averaging
    float tdc[2] = {(float) m_TDC, (float) bgDigit->getTDC()};
    float adc[2] = {(float) m_ADC, (float) bgDigit->getADC()};
    float sum = adc[0] + adc[1];
    if (sum > 0) {
      m_TDC = int((tdc[0] * adc[0] + tdc[1] * adc[1]) / sum);
    } else {
      m_TDC = int((tdc[0] + tdc[1]) / 2);
    }
    m_ADC = int(sum);
    // double reweight = sum > 0 ? adc[0]/sum : 0.5;
    // this->modifyRelationsTo<TOPSimHit>(reweight);
    // this->modifyRelationsTo<MCParticle>(reweight);
  } else { // pile-up results in discarding the second-in-time hit
    if (diff > 0) { // bg digit is the first-in-time hit, therefore replace
      *this = *bgDigit;
      // this->removeRelationsTo<TOPSimHit>();
      // this->removeRelationsTo<MCParticle>();
    }
  }
  return DigitBase::c_DontAppend;

}

