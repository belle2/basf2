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
#include <eklm/geometry/Arc2D.h>

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

