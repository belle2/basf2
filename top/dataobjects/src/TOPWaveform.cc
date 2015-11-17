/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPWaveform.h>

using namespace std;
using namespace Belle2;

// ClassImp(TOPWaveform)


bool TOPWaveform::setDigital(float upperThr, float lowerThr, int minWidth)
{

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

  int n = 0;
  for (const auto& digital : m_digital) {
    if (digital) n++;
    else n = 0;
    if (n > minWidth) return true;
  }

  return false;
}


