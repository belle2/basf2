/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dataobjects/TOPRawWaveform.h>

using namespace std;

namespace Belle2 {

  int TOPRawWaveform::getIntegral(int sampleRise, int samplePeak, int sampleFall) const
  {
    sampleRise -= m_startSample;
    samplePeak -= m_startSample;
    sampleFall -= m_startSample;

    return Integral(sampleRise, samplePeak, sampleFall);
  }

  int TOPRawWaveform::Integral(int sampleRise, int samplePeak, int sampleFall) const
  {
    int min = samplePeak - 3 * (samplePeak - sampleRise);
    if (min < 0) min = 0;
    int max = samplePeak + 4 * (sampleFall + 1 - samplePeak);
    int size = m_data.size();
    if (max > size) max = size;
    int integral = 0;
    while (min < max) {
      integral += m_data[min];
      min++;
    }
    return integral;
  }


  int TOPRawWaveform::featureExtraction(int threshold, int hysteresis,
                                        int thresholdCount) const
  {

    m_features.clear();

    int currentThr = threshold;
    bool lastState = false;
    int samplePeak = 0;
    int size = m_data.size();
    int aboveThr = 0;
    for (int i = 0; i < size; i++) {
      const auto& adc = m_data[i];
      if (adc > currentThr and i < size - 1) {
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
          feature.sampleRise = sampleRise + m_startSample;
          feature.samplePeak = samplePeak + m_startSample;
          feature.sampleFall = sampleFall + m_startSample;
          feature.vRise0 = m_data[sampleRise];
          feature.vRise1 = m_data[sampleRise + 1];
          feature.vPeak = m_data[samplePeak];
          feature.vFall0 = m_data[sampleFall];
          feature.vFall1 = m_data[sampleFall + 1];
          feature.integral = Integral(sampleRise, samplePeak, sampleFall);
          m_features.push_back(feature);
        }
        lastState = false;
        aboveThr = 0;
      }
    }

    return m_features.size();
  }

} // end Belle2 namespace

