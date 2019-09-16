/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/eklm/geometry/Arc2D.h>

using namespace Belle2;

EKLM::Arc2D::Arc2D(double x, double y, double radius,
                   double angle1, double angle2) : Circle2D(x, y, radius)
{
  m_Angle1 = angle1;
  m_Angle2 = angle2;
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

