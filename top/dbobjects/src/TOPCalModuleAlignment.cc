/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalModuleAlignment.h>
#include <framework/logging/Logger.h>
#include <Math/RotationX.h>
#include <Math/RotationY.h>
#include <Math/RotationZ.h>
#include <Math/Translation3D.h>

using namespace std;
using namespace ROOT::Math;

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
    m_transforms.clear();
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
    m_transforms.clear();
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
    m_transforms.clear();
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
    m_transforms.clear();
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
    m_transforms.clear();
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
    m_transforms.clear();
  }

  void TOPCalModuleAlignment::setCalibrated(int moduleID)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module] = c_Calibrated;
    m_transforms.clear();
  }

  void TOPCalModuleAlignment::setUnusable(int moduleID)
  {
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
      return;
    }
    m_status[module] = c_Unusable;
    m_transforms.clear();
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
      Transform3D T;
      if (m_status[i] == c_Calibrated) {
        RotationX Rx(m_alpha[i]);
        RotationY Ry(m_beta[i]);
        RotationZ Rz(m_gamma[i]);
        Translation3D t(m_x[i], m_y[i], m_z[i]);
        T = Transform3D(Rz * Ry * Rx, t);
      }
      m_transforms.push_back(T);
    }
    m_transforms.push_back(Transform3D()); // for invalid module ID
  }

  const Transform3D& TOPCalModuleAlignment::getTransformation(int moduleID) const
  {
    if (m_transforms.empty()) setTransformations();
    unsigned module = moduleID - 1;
    if (module >= c_numModules) {
      B2ERROR("Invalid module number, returning identity transformation (" << ClassName() << ")");
      return m_transforms[c_numModules];
    }
    return m_transforms[module];
  }


} // namespace Belle2

