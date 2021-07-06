/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalModuleAlignment.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalModuleAlignment::setAlpha(int moduleID, double alpha, double errAlpha)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_alpha[module] = alpha;
    m_errAlpha[module] = errAlpha;
  }

  void TOPCalModuleAlignment::setBeta(int moduleID, double beta, double errBeta)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_beta[module] = beta;
    m_errBeta[module] = errBeta;
  }

  void TOPCalModuleAlignment::setGamma(int moduleID, double gamma, double errGamma)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_gamma[module] = gamma;
    m_errGamma[module] = errGamma;
  }

  void TOPCalModuleAlignment::setX(int moduleID, double x, double errX)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_x[module] = x;
    m_errX[module] = errX;
  }

  void TOPCalModuleAlignment::setY(int moduleID, double y, double errY)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_y[module] = y;
    m_errY[module] = errY;
  }

  void TOPCalModuleAlignment::setZ(int moduleID, double z, double errZ)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
      return;
    }
    m_z[module] = z;
    m_errZ[module] = errZ;
  }

  void TOPCalModuleAlignment::setCalibrated(int moduleID)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module] = c_Calibrated;
  }

  void TOPCalModuleAlignment::setUnusable(int moduleID)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module] = c_Unusable;
  }

  double TOPCalModuleAlignment::getAlpha(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_alpha[module];
  }

  double TOPCalModuleAlignment::getBeta(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_beta[module];
  }

  double TOPCalModuleAlignment::getGamma(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_gamma[module];
  }

  double TOPCalModuleAlignment::getX(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_x[module];
  }

  double TOPCalModuleAlignment::getY(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_y[module];
  }

  double TOPCalModuleAlignment::getZ(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_z[module];
  }

  double TOPCalModuleAlignment::getAlphaErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errAlpha[module];
  }

  double TOPCalModuleAlignment::getBetaErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errBeta[module];
  }

  double TOPCalModuleAlignment::getGammaErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errGamma[module];
  }

  double TOPCalModuleAlignment::getXErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errX[module];
  }

  double TOPCalModuleAlignment::getYErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errY[module];
  }

  double TOPCalModuleAlignment::getZErr(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
      return 0;
    }
    return m_errZ[module];
  }

  bool TOPCalModuleAlignment::isCalibrated(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false ;
    return m_status[module] == c_Calibrated;
  }

  bool TOPCalModuleAlignment::isDefault(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false ;
    return m_status[module] == c_Default;
  }

  bool TOPCalModuleAlignment::isUnusable(int moduleID) const
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) return false ;
    return m_status[module] == c_Unusable;
  }

  bool TOPCalModuleAlignment::areAllCalibrated() const
  {
    for (int i = 0; i <  c_numModules; i++) {
      if (m_status[i] != c_Calibrated) return false;
    }
    return true;
  }

  bool TOPCalModuleAlignment::areAllPrecise(double spatialPrecision,
                                            double angularPrecision) const
  {
    for (int i = 0; i <  c_numModules; i++) {
      if (m_errAlpha[i] > angularPrecision) return false;
      if (m_errBeta[i] > angularPrecision) return false;
      if (m_errGamma[i] > angularPrecision) return false;
      if (m_errX[i] > spatialPrecision) return false;
      if (m_errY[i] > spatialPrecision) return false;
      if (m_errZ[i] > spatialPrecision) return false;
    }
    return true;
  }

  void TOPCalModuleAlignment::setTransformations() const
  {
    for (int i = 0; i < c_numModules; i++) {
      TRotation rot;
      TVector3 vec;
      if (m_status[i] == c_Calibrated) {
        rot.RotateX(m_alpha[i]).RotateY(m_beta[i]).RotateZ(m_gamma[i]);
        vec.SetXYZ(m_x[i], m_y[i], m_z[i]);
      }
      m_rotations.push_back(rot);
      m_translations.push_back(vec);
    }
    m_rotations.push_back(TRotation());   // for invalid module ID
    m_translations.push_back(TVector3()); // for invalid module ID
  }

  const TRotation& TOPCalModuleAlignment::getRotation(int moduleID) const
  {
    if (m_rotations.empty()) setTransformations();
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, returning identity rotation (" << ClassName() << ")");
      return m_rotations[c_numModules];
    }
    return m_rotations[module];
  }

  const TVector3& TOPCalModuleAlignment::getTranslation(int moduleID) const
  {
    if (m_translations.empty()) setTransformations();
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, returning null vector (" << ClassName() << ")");
      return m_translations[c_numModules];
    }
    return m_translations[module];
  }


} // namespace Belle2

