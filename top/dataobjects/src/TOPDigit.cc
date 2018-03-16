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
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  float TOPDigit::s_doubleHitResolution = 0;
  float TOPDigit::s_pileupTime = 0;

  int TOPDigit::getModulo256Sample() const
  {
    int sample = int(m_rawTime);
    if (m_rawTime < 0) sample--;
    sample += (int) m_firstWindow * 64;
    if (sample < 0) return 256 + sample % 256;
    return sample % 256;
  }


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
      double reweight = sum > 0 ? pulseHeight[0] / sum : 0.5;
      auto relSimHits = this->getRelationsTo<TOPSimHit>();
      for (size_t i = 0; i < relSimHits.size(); ++i) {
        float weight = relSimHits.weight(i) * reweight;
        relSimHits.setWeight(i, weight);
      }
      auto relRawDigits = this->getRelationsTo<TOPRawDigit>();
      for (size_t i = 0; i < relRawDigits.size(); ++i) {
        float weight = relRawDigits.weight(i) * reweight;
        relRawDigits.setWeight(i, weight);
      }
      auto relMCParticles = this->getRelationsTo<MCParticle>();
      for (size_t i = 0; i < relMCParticles.size(); ++i) {
        float weight = relMCParticles.weight(i) * reweight;
        relMCParticles.setWeight(i, weight);
      }
    } else { // pile-up results in discarding the second-in-time hit
      if (diff > 0) {
        // bg digit is the first-in-time hit, therefore replace and remove relations
        *this = *bgDigit;

        // remove relations (going from back side!)
        auto relSimHits = this->getRelationsTo<TOPSimHit>();
        for (int i = relSimHits.size() - 1; i >= 0; --i) {
          relSimHits.remove(i);
        }
        auto relRawDigits = this->getRelationsTo<TOPRawDigit>();
        for (int i = relRawDigits.size() - 1; i >= 0; --i) {
          relRawDigits.remove(i);
        }
        auto relMCParticles = this->getRelationsTo<MCParticle>();
        for (int i = relMCParticles.size() - 1; i >= 0; --i) {
          relMCParticles.remove(i);
        }
      }
    }
    return DigitBase::c_DontAppend;

  }

} // end Belle2 namespace

