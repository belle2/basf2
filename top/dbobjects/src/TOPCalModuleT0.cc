/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalModuleT0.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalModuleT0::setT0(int moduleID, double T0, double errT0)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_T0[module] = T0;
    m_errT0[module] = errT0;
    m_status[module] = c_Calibrated;
  }


  void TOPCalModuleT0::setUnusable(int moduleID)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module] = c_Unusable;
  }


  void TOPCalModuleT0::suppressAverage()
  {
    float s = 0;
    int n = 0;
    for (int i = 0; i < c_numModules; i++) {
      if (m_status[i] == c_Calibrated) {
        s += m_T0[i];
        n++;
      }
    }
    if (n == 0) return;
    s /= n;
    for (int i = 0; i < c_numModules; i++) {
      if (m_status[i] != c_Default) m_T0[i] -= s;
    }
    B2INFO("Average of " << s  << " ns subtracted.");
  }


  double TOPCalModuleT0::getT0(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_T0[module];
  }


  double TOPCalModuleT0::getT0Error(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errT0[module];
  }


  bool TOPCalModuleT0::isCalibrated(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    return m_status[module] == c_Calibrated;
  }


  bool TOPCalModuleT0::isDefault(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    return m_status[module] == c_Default;
  }


  bool TOPCalModuleT0::isUnusable(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    return m_status[module] == c_Unusable;
  }


} // end Belle2 namespace


