/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentConstant.h>

using namespace std;
using namespace Belle2;

ROOT::Math::XYZVector BFieldComponentConstant::calculate(const ROOT::Math::XYZVector& point) const
{
  if ((maxRadius4BField > 0.0 && maxRadius4BField < point.Rho()) ||
      (point.Z() < minZ4BField  || point.Z() > maxZ4BField)) return ROOT::Math::XYZVector(0, 0, 0);
  return ROOT::Math::XYZVector(m_magneticField[0], m_magneticField[1], m_magneticField[2]);
}


void BFieldComponentConstant::setMagneticFieldValues(double x, double y, double z, double rmax, double zmin, double zmax)
{
  m_magneticField[0] = x;
  m_magneticField[1] = y;
  m_magneticField[2] = z;
  maxRadius4BField = rmax;  // unit [cm]
  minZ4BField = zmin;       // unit [cm]
  maxZ4BField = zmax;       // unit [cm]
}
