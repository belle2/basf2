/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalChannelT0.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalChannelT0::setT0(int moduleID, unsigned channel, double T0, double errT0)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    if (channel >= c_numChannels) {
      B2ERROR("Invalid channel number, constant not set (" << ClassName() << ")");
      return;
    }
    m_T0[module][channel] = T0;
    m_errT0[module][channel] = errT0;
    m_status[module][channel] = c_Calibrated;
  }


  void TOPCalChannelT0::setUnusable(int moduleID, unsigned channel)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    if (channel >= c_numChannels) {
      B2ERROR("Invalid channel number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module][channel] = c_Unusable;
  }


  void TOPCalChannelT0::suppressAverage()
  {
    for (int m = 0; m < c_numModules; m++) {
      float s = 0;
      int n = 0;
      for (int i = 0; i < c_numChannels; i++) {
        if (m_status[m][i] == c_Calibrated) {
          s += m_T0[m][i];
          n++;
        }
      }
      if (n == 0) continue;
      s /= n;
      for (int i = 0; i < c_numChannels; i++) {
        if (m_status[m][i] != c_Default) m_T0[m][i] -= s;
      }
      B2INFO("Slot " << m + 1 << ": average of " << s  << " ns subtracted.");
    }
  }


  double TOPCalChannelT0::getT0(int moduleID, unsigned channel) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    if (channel >= c_numChannels) {
      B2WARNING("Invalid channel number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_T0[module][channel];
  }


  double TOPCalChannelT0::getT0Error(int moduleID, unsigned channel) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    if (channel >= c_numChannels) {
      B2WARNING("Invalid channel number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errT0[module][channel];
  }


  bool TOPCalChannelT0::isCalibrated(int moduleID, unsigned channel) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (channel >= c_numChannels) return false;
    return m_status[module][channel] == c_Calibrated;
  }


  bool TOPCalChannelT0::isDefault(int moduleID, unsigned channel) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (channel >= c_numChannels) return false;
    return m_status[module][channel] == c_Default;
  }


  bool TOPCalChannelT0::isUnusable(int moduleID, unsigned channel) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (channel >= c_numChannels) return false;
    return m_status[module][channel] == c_Unusable;
  }


} // end Belle2 namespace
