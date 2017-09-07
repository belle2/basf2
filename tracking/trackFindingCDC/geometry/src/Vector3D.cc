/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <TVector3.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector3D::Vector3D(const TVector3& tVector3)
  : m_xy(tVector3.X(), tVector3.Y())
  , m_z(tVector3.Z())
{
}

Vector3D& Vector3D::operator=(const TVector3& tVector3)
{
  m_xy.setX(tVector3.X());
  m_xy.setY(tVector3.Y());
  m_z = tVector3.Z();
  return *this;
}

Vector3D::operator const TVector3() const
{
  return TVector3(x(), y(), z());
}

