/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dataobjects/TOPRawDigit.h>
#include <math.h>

using namespace std;

namespace Belle2 {

  bool TOPRawDigit::isPedestalJump() const
  {
    if (m_sampleRise == 0) {
      if ((m_dSampleFall + 1) % 64 == 0) return true;
    } else if ((m_sampleRise + 1) % 64 == 0) {
      if (m_dSampleFall % 64 == 0) return true;
    }
    return false;
  }


  bool TOPRawDigit::isAtWindowDiscontinuity(unsigned short storageDepth) const
  {
    if (m_windows.empty()) return false;

    unsigned size = m_windows.size();
    unsigned i0 = getSampleRise() / c_WindowSize;
    if (i0 >= size) i0 = size - 1;
    unsigned i1 = (getSampleFall() + 1) / c_WindowSize;
    if (i1 >= size) i1 = size - 1;
    if (i0 == i1) return false;

    int diff = m_windows[i1] - m_windows[i0];
    if (diff < 0) diff += storageDepth;
    if (diff != 1) return true;
    return false;
  }


  bool TOPRawDigit::areWindowsInOrder(unsigned short storageDepth) const
  {
    unsigned lastSample = getSampleFall() + 1;
    unsigned last = lastSample / c_WindowSize + 1;
    unsigned size = m_windows.size();
    for (unsigned i = 1; i < std::min(last, size); i++) {
      int diff = m_windows[i] - m_windows[i - 1];
      if (diff < 0) diff += storageDepth;
      if (diff != 1) return false;
    }
    return true;
  }


  double TOPRawDigit::correctTime(double time, unsigned short storageDepth) const
  {
    if (m_windows.empty()) return time;

    unsigned last = int(time) / c_WindowSize + 1;
    unsigned size = m_windows.size();
    int missing = 0;
    for (unsigned i = 1; i < std::min(last, size); i++) {
      int diff = m_windows[i] - m_windows[i - 1];
      if (diff < 0) diff += storageDepth;
      if (diff != 1) missing += diff - 1;
      if (diff == 0) B2ERROR("TOPRawDigit: two consecutive windows with the same number");
    }

    return time + missing * c_WindowSize;
  }


  double TOPRawDigit::timeErrorCoefficient(double y1, double y2) const
  {
    double dy = y2 - y1;
    double ym = m_VPeak / 2.0;
    double a = 1.0 / (2.0 * dy);
    double b = (ym - y2) / (dy * dy);
    double c = (ym - y1) / (dy * dy);

    return sqrt(a * a + b * b + c * c);
  }

} // end Belle2 namespace


