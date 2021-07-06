/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/Arc2D.h>

using namespace Belle2;

EKLM::Arc2D::Arc2D(double x, double y, double radius,
                   double angle1, double angle2) :
  Circle2D(x, y, radius),
  m_Angle1(angle1),
  m_Angle2(angle2)
{
}

EKLM::Arc2D::~Arc2D()
{
}

bool EKLM::Arc2D::angleWithinRange(double angle) const
{
  if (m_Angle2 > m_Angle1)
    return (angle <= m_Angle2 && angle >= m_Angle1);
  return (angle >= m_Angle1 || angle <= m_Angle2);
}

HepGeom::Point3D<double> EKLM::Arc2D::getInitialPoint() const
{
  HepGeom::Point3D<double> p;
  p.setX(m_Radius * cos(m_Angle1));
  p.setY(m_Radius * sin(m_Angle1));
  p.setZ(0);
  return p;
}

