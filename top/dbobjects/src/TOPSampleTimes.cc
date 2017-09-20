/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPSampleTimes.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  void TOPSampleTimes::setTimeAxis(double syncTimeBase)
  {
    double DTime = 2 * syncTimeBase;
    double timeBin = DTime / c_TimeAxisSize;
    for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = timeBin * i;
    m_timeAxis[c_TimeAxisSize] = DTime;
    m_calibrated = false;
  }


  void TOPSampleTimes::setTimeAxis(const std::vector<double>& sampleTimes,
                                   double syncTimeBase)
  {
    if (sampleTimes.size() < c_TimeAxisSize) {
      B2FATAL("TOPSampleTimes::setTimeAxis: vector too short");
      return;
    }

    for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = sampleTimes[i];
    double DTime = 2 * syncTimeBase;
    m_timeAxis[c_TimeAxisSize] = DTime;
    m_calibrated = true;
  }


  double TOPSampleTimes::getFullTime(unsigned window, double sample) const
  {

    int sampleNum = int(sample);
    if (sample < 0) sampleNum--;
    double frac = sample - sampleNum;

    sampleNum += (int)(window * c_WindowSize);  // counted from window 0
    int n = sampleNum / c_TimeAxisSize;
    int k = sampleNum % c_TimeAxisSize;
    if (k < 0) {
      n--;
      k += c_TimeAxisSize;
    }

    double time = n * getTimeRange() + m_timeAxis[k]; // from sample 0 window 0
    time += (m_timeAxis[k + 1] - m_timeAxis[k]) * frac; // add fraction

    return time;
  }


  double TOPSampleTimes::getSample(unsigned window, double time) const
  {
    time += getFullTime(window, 0);
    int n = int(time / getTimeRange());
    if (time < 0) n--;

    double t = time - getTimeRange() * n;
    int i1 = 0;
    int i2 = c_TimeAxisSize;
    while (i2 - i1 > 1) {
      int i = (i1 + i2) / 2;
      if (t > m_timeAxis[i]) {
        i1 = i;
      } else {
        i2 = i;
      }
    }

    return (n * (int) c_TimeAxisSize - (int)(window * c_WindowSize) + i1 +
            (t - m_timeAxis[i1]) / (m_timeAxis[i2] - m_timeAxis[i1]));

  }

} // end Belle2 namespace

