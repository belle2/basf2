/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPWaveform.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  int TOPWaveform::setDigital(float upperThr, float lowerThr, unsigned minWidth)
  {

    m_digital.clear();
    m_hits.clear();

    // set digital waveform

    float threshold = upperThr;
    for (const auto& sample : m_data) {
      if (sample.err > 0) {
        if (sample.adc / sample.err > threshold) {
          m_digital.push_back(true);
          threshold = lowerThr;
        } else {
          m_digital.push_back(false);
          threshold = upperThr;
        }
      } else {
        m_digital.push_back(threshold == lowerThr);
      }
    }

    // remove pulses having width <= minWidth, count accepted pulses

    m_digital.push_back(false);
    bool prev = false;
    unsigned i0 = 0;
    int n = 0;
    for (unsigned i = 0; i < m_digital.size(); i++) {
      const auto& digital = m_digital[i];
      if (digital and !prev) { // begin of pulse
        i0 = i;
      } else if (prev and !digital) { // end of pulse
        if (i - i0 > minWidth) {
          n++;
        } else {
          for (unsigned k = i0; k < i; k++) m_digital[k] = false;
        }
      }
      prev = digital;
    }
    m_digital.pop_back();

    return n;
  }


  int TOPWaveform::convertToHits(float fraction)
  {

    m_hits.clear();

    if (m_digital.empty()) {
      B2ERROR("TOPWaveform::reconstructHits: setDigital must be called first");
      return 0;
    }

    m_digital.push_back(false);
    m_data.push_back(WFSample());
    Hit hit;
    bool prev = false;
    for (unsigned i = 0; i < m_digital.size(); i++) {
      const auto& digital = m_digital[i];
      const auto& data = m_data[i];
      if (digital and !prev) { // begin of digital pulse
        hit.height = data.adc;
        hit.heightErr = data.err;
        hit.width = 0;
        hit.area = data.adc;
        hit.areaErr = data.err * data.err;
        hit.imax = i;
      } else if (prev and !digital) { // end of digital pulse: determine time and append hit
        float heightFraction = hit.height * fraction;
        unsigned k = hit.imax;
        if (k >= m_data.size() - 2) continue; // last sample may not be the maximum
        while (k > 0 and m_data[k].adc > heightFraction) k--;
        float t1 = m_data[k].time;
        float y1 = m_data[k].adc - heightFraction;
        if (y1 > 0) continue; // leading edge of signal not present
        k++;
        float t2 = m_data[k].time;
        float y2 = m_data[k].adc - heightFraction;
        if (y1 != y2) {
          hit.time = t1 - (t2 - t1) / (y2 - y1) * y1;
        } else {
          hit.time = (t1 + t2) / 2;
        }
        hit.areaErr = sqrt(hit.areaErr);
        m_hits.push_back(hit);
      }
      if (data.adc > hit.height) {
        hit.height = data.adc;
        hit.heightErr = data.err;
        hit.imax = i;
      }
      hit.width++;
      hit.area += data.adc;
      hit.areaErr += data.err * data.err;
      prev = digital;
    }
    m_digital.pop_back();
    m_data.pop_back();

    return m_hits.size();
  }

} // end Belle2 namespace

