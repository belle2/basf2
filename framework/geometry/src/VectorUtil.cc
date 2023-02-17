/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <framework/geometry/VectorUtil.h>

double Belle2::VectorUtil::phi0TwoPi(double angle)
{
  const double c_TwoPi = 2.0 * M_PI;
  while (angle < 0)
    angle = angle + c_TwoPi;
  while (angle >= c_TwoPi)
    angle = angle - c_TwoPi;
  return angle;
}

double Belle2::VectorUtil::phiMinusPiPi(double angle)
{
  const double c_TwoPi = 2.0 * M_PI;
  while (angle < -M_PI)
    angle = angle + c_TwoPi;
  while (angle >= M_PI)
    angle = angle - c_TwoPi;
  return angle;
}
