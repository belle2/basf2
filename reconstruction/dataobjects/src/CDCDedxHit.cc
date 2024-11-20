/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <reconstruction/dataobjects/CDCDedxHit.h>

namespace Belle2 {

  double CDCDedxHit::getSignedDOCAXY() const
  {
    double doca = sqrt(m_dx * m_dx + m_dy * m_dy);
    double z = m_dx * m_y - m_dy * m_x;
    if (z < 0) doca = -doca;

    return doca;
  }

  double CDCDedxHit::getEntranceAngle() const
  {
    double px = m_px;
    double py = m_py;
    double wx = m_x + m_dx;
    double wy = m_y + m_dy;
    double cross = wx * py - wy * px;
    double dot = wx * px + wy * py;

    return atan2(cross, dot);
  }

} // end namespace Belle2

