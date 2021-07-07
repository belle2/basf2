/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <TVector3.h>

#include <sstream>
#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector3D::Vector3D(const TVector3& tVector3)
  : m_xy(tVector3.X(), tVector3.Y())
  , m_z(tVector3.Z())
{
}

Vector3D::Vector3D(const B2Vector3D& b2Vector3)
  : m_xy(b2Vector3.X(), b2Vector3.Y())
  , m_z(b2Vector3.Z())
{
}

Vector3D& Vector3D::operator=(const TVector3& tVector3)
{
  m_xy.setX(tVector3.X());
  m_xy.setY(tVector3.Y());
  m_z = tVector3.Z();
  return *this;
}

Vector3D& Vector3D::operator=(const B2Vector3D& b2Vector3)
{
  m_xy.setX(b2Vector3.X());
  m_xy.setY(b2Vector3.Y());
  m_z = b2Vector3.Z();
  return *this;
}

Vector3D::operator const TVector3() const
{
  return TVector3(x(), y(), z());
}

Vector3D::operator const B2Vector3D() const
{
  return B2Vector3D(x(), y(), z());
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const Vector3D& vector3D)
{
  return output << "Vector3D(" << vector3D.x() << "," << vector3D.y() << "," << vector3D.z() << ")";
}

std::string Vector3D::__str__() const
{
  std::stringstream sstream;
  sstream << *this;
  return sstream.str();
}
