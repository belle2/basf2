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

float TOPDigit::s_doubleHitResolution = 0;
float TOPDigit::s_pileupTime = 0;


DigitBase::EAppendStatus TOPDigit::addBGDigit(const DigitBase* bg)
{
  const auto* bgDigit = static_cast<const TOPDigit*>(bg);
  double diff = m_time - bgDigit->getTime();

  if (fabs(diff) > s_doubleHitResolution) return DigitBase::c_Append; // no pile-up

  if (fabs(diff) < s_pileupTime) { // pile-up results in time averaging
    double time[2] = {m_time, bgDigit->getTime()};
    double rawTime[2] = {m_rawTime, bgDigit->getRawTime()};
    int pulseHeight[2] = {m_pulseHeight, bgDigit->getPulseHeight()};
    double sum = pulseHeight[0] + pulseHeight[1];
    if (sum > 0) {
      m_time = (time[0] * pulseHeight[0] + time[1] * pulseHeight[1]) / sum;
      m_rawTime = (rawTime[0] * pulseHeight[0] + rawTime[1] * pulseHeight[1]) / sum;
      // m_timeError =
      // m_pulseWidth =
      // m_integral =
    } else {
      m_time = (time[0] + time[1]) / 2;
      m_rawTime = (rawTime[0] + rawTime[1]) / 2;
      // m_timeError =
      // m_pulseWidth =
      // m_integral =
    }
    m_pulseHeight = int(sum);
    // double reweight = sum > 0 ? pulseHeight[0]/sum : 0.5;
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

