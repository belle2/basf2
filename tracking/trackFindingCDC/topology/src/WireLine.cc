/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/WireLine.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireLine::WireLine(const Vector3D& forward,
                   const Vector3D& backward)
  : m_refPos3D{(backward * forward.z() - forward * backward.z()) / (forward.z() - backward.z()) },
    m_movePerZ{(forward.xy() - backward.xy()) / (forward.z() - backward.z())},
    m_forwardZ{forward.z()},
    m_backwardZ{backward.z()}
{
  B2ASSERT("Wire reference position is not at 0", m_refPos3D.z() == 0);
}

WireLine WireLine::movedBy(const Vector3D& offset) const
{
  WireLine moved = *this;
  moved.m_refPos3D += offset.xy() + movePerZ() * offset.z();
  moved.m_forwardZ += offset.z();
  moved.m_backwardZ += offset.z();
  return moved;
}

WireLine WireLine::movedBy(const Vector2D& offset) const
{
  WireLine moved = *this;
  moved.m_refPos3D += offset;
  return moved;
}
