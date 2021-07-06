/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalAsicShift.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalAsicShift::setT0(int moduleID, unsigned asic, double T0)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    if (asic >= c_numAsics) {
      B2ERROR("Invalid asic number, constant not set (" << ClassName() << ")");
      return;
    }
    m_T0[module][asic] = T0;
    m_status[module][asic] = c_Calibrated;
  }


  void TOPCalAsicShift::setUnusable(int moduleID, unsigned asic)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    if (asic >= c_numAsics) {
      B2ERROR("Invalid asic number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module][asic] = c_Unusable;
  }


  double TOPCalAsicShift::getT0(int moduleID, unsigned asic) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    if (asic >= c_numAsics) {
      B2WARNING("Invalid asic number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_T0[module][asic];
  }


  bool TOPCalAsicShift::isCalibrated(int moduleID, unsigned asic) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (asic >= c_numAsics) return false;
    return m_status[module][asic] == c_Calibrated;
  }


  bool TOPCalAsicShift::isDefault(int moduleID, unsigned asic) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (asic >= c_numAsics) return false;
    return m_status[module][asic] == c_Default;
  }


  bool TOPCalAsicShift::isUnusable(int moduleID, unsigned asic) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false;
    if (asic >= c_numAsics) return false;
    return m_status[module][asic] == c_Unusable;
  }


} // end Belle2 namespace
