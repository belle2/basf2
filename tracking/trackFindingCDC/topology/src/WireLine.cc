/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/WireLine.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireLine::WireLine(const Vector3D& forward,
                   const Vector3D& backward,
                   double sagCoeff)
  : m_refPos3D{(backward * forward.z() - forward * backward.z()) / (forward.z() - backward.z()) }
  , m_nominalMovePerZ{(forward.xy() - backward.xy()) / (forward.z() - backward.z())}
  , m_forwardZ{forward.z()}
  , m_backwardZ{backward.z()}
  , m_sagCoeff(sagCoeff)
{
  B2ASSERT("Wire reference position is not at 0", m_refPos3D.z() == 0);
}

WireLine WireLine::movedBy(const Vector3D& offset) const
{
  WireLine moved = *this;
  moved.m_refPos3D += offset.xy() + nominalMovePerZ() * offset.z();
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
