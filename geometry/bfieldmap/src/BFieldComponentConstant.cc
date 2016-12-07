/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentConstant.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace std;
using namespace Belle2;

B2Vector3D BFieldComponentConstant::calculate(const B2Vector3D& point) const
{
  double Bz = m_magneticField[2];
  if (maxRadius4BField > 0.0 && maxRadius4BField < point.Perp()) Bz = 0.0;
  if (point.z() < minZ4BField  || point.z() > maxZ4BField) Bz = 0.0;

  //  return B2Vector3D(m_magneticField[0], m_magneticField[1], m_magneticField[2]);
  return B2Vector3D(m_magneticField[0], m_magneticField[1], Bz);
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
