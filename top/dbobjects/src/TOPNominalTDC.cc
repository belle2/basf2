/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPNominalTDC.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  TOPNominalTDC::TOPNominalTDC(int numWindows,
                               int subBits,
                               double syncTimeBase,
                               int numofBunches,
                               double offset,
                               double pileupTime,
                               double doubleHitResolution,
                               double timeJitter,
                               double efficiency,
                               const std::string& name):
    TOPGeoBase(name),
    m_numWindows(numWindows), m_subBits(subBits), m_syncTimeBase(syncTimeBase),
    m_numofBunches(numofBunches),
    m_offset(offset), m_pileupTime(pileupTime), m_doubleHitResolution(doubleHitResolution),
    m_timeJitter(timeJitter), m_efficiency(efficiency)
  {
    if (numWindows <= 0)
      B2FATAL("TOPNominalTDC: numWindows must be > 0");
    if (subBits < 0)
      B2FATAL("TOPNominalTDC: subBits must be >= 0");

    int numSamples = numWindows * c_WindowSize;
    int numBits = subBits - 1;
    int k = numSamples;
    do {
      numBits++;
      k /= 2;
    } while (k > 0);
    if (numSamples > (1 << (numBits - subBits))) numBits++;
    m_numBits = numBits;

    int syncSamples = c_syncWindows * c_WindowSize;
    m_sampleWidth = syncTimeBase / syncSamples;
    m_binWidth = m_sampleWidth / (1 << subBits);
  }

  int TOPNominalTDC::getTDCcount(double time) const
  {
    time += m_offset;
    int overflow = getOverflowValue();
    if (time < 0) return overflow;
    if (time > overflow * m_binWidth) return overflow;
    return int(time / m_binWidth);
  }

  int TOPNominalTDC::getSample(double time) const
  {
    time += m_offset;
    if (time > 0) {
      return int(time / m_sampleWidth);
    } else {
      return int(time / m_sampleWidth) - 1;
    }
  }

  bool TOPNominalTDC::isSampleValid(int sample) const
  {
    if (sample < 0) return false;
    if (sample >= (int) m_numWindows * c_WindowSize) return false;
    return true;
  }

  bool TOPNominalTDC::isConsistent() const
  {
    if (m_pileupTime < 0) return false;
    if (m_doubleHitResolution < 0) return false;
    if (m_timeJitter < 0) return false;
    if (m_efficiency <= 0 or m_efficiency > 1) return false;
    return true;
  }

  void TOPNominalTDC::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);
    cout << " range: [" << getTimeMin() << ", " << getTimeMax() << "] ns" << endl;
    cout << " offset: " << getOffset() << " ns" << endl;
    cout << " number of bits: " << getNumBits() << endl;
    cout << " bin width: " << getBinWidth() / Unit::ps << " ps" << endl;
    cout << " number of ASIC windows: " << getNumWindows() << endl;
    cout << " number of bits per sample: " << getSubBits() << endl;
    cout << " synchonization time base: " << getSyncTimeBase() << " ns" << endl;
    cout << " pile-up time: " << getPileupTime() << " ns" << endl;
    cout << " double hit resolution: " << getDoubleHitResolution() << " ns" << endl;
    cout << " r.m.s. of time jitter: " << getTimeJitter() / Unit::ps << " ps" << endl;
    cout << " electronic efficiency: " << getEfficiency() << endl;
  }

} // end Belle2 namespace
