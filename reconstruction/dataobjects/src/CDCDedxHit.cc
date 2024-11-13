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
    //TODO...
    double doca = sqrt(m_dx * m_dx + m_dy * m_dy);
    double phidiff = getPOCAOnTrack().Phi() - getPOCAOnWire().Phi();
    // be careful about "wrap around" cases when the poca and wire are close, but the difference in phi is largy
    if (phidiff > -3.1416 and (phidiff < 0 or phidiff > 3.1416)) return -doca;
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

