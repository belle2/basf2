/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPRawWaveform.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

int TOPRawWaveform::featureExtraction(int threshold, int hysteresis,
                                      int thresholdCount) const
{

  m_features.clear();

  int currentThr = threshold;
  bool lastState = false;
  int samplePeak = 0;
  int size = m_data.size();
  int aboveThr = 0;
  for (unsigned i = 0; i < m_data.size(); i++) {
    const auto& adc = m_data[i];
    if (adc > currentThr) {
      currentThr = threshold - hysteresis;
      if (!lastState or adc > m_data[samplePeak]) samplePeak = i;
      lastState = true;
      aboveThr++;
    } else {
      currentThr = threshold;
      if (lastState and aboveThr >= thresholdCount) {
        auto halfValue = m_data[samplePeak] / 2;
        auto sampleRise = samplePeak;
        while (sampleRise > 0 and m_data[sampleRise] > halfValue) sampleRise--;
        if (sampleRise == 0 and m_data[sampleRise] > halfValue) continue;
        auto sampleFall = samplePeak;
        while (sampleFall < size - 1 and m_data[sampleFall] > halfValue) sampleFall++;
        if (sampleFall == size - 1 and m_data[sampleFall] > halfValue) continue;
        sampleFall--;
        FeatureExtraction feature;
        feature.sampleRise = sampleRise;
        feature.samplePeak = samplePeak;
        feature.sampleFall = sampleFall;
        feature.vRise0 = m_data[sampleRise];
        feature.vRise1 = m_data[sampleRise + 1];
        feature.vPeak = m_data[samplePeak];
        feature.vFall0 = m_data[sampleFall];
        feature.vFall1 = m_data[sampleFall + 1];
        int min = samplePeak - 3 * (samplePeak - sampleRise);
        if (min < 0) min = 0;
        int max = samplePeak + 4 * (sampleFall + 1 - samplePeak);
        if (max > size) max = size;
        while (min < max) {
          feature.integral += m_data[min];
          min++;
        }
        m_features.push_back(feature);
      }
      lastState = false;
      aboveThr = 0;
    }
  }

  return m_features.size();
}
