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

B2Vector3D BFieldComponentConstant::calculate(const B2Vector3D& point) const
{
  if ((maxRadius4BField > 0.0 && maxRadius4BField < point.Perp()) ||
      (point.z() < minZ4BField  || point.z() > maxZ4BField)) return B2Vector3D(0, 0, 0);
  return B2Vector3D(m_magneticField);
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
