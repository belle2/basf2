/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/topology/WireLine.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace CDC;

WireLine::WireLine(const ROOT::Math::XYZVector& forward,
                   const ROOT::Math::XYZVector& backward,
                   double sagCoeff)
  : m_refPos3D{(backward * forward.z() - forward * backward.z()) / (forward.z() - backward.z()) }
  , m_forwardZ{forward.z()}
  , m_backwardZ{backward.z()}
  , m_sagCoeff(sagCoeff)
{
  m_refPos2D = ROOT::Math::XYVector(m_refPos3D.x(), m_refPos3D.y());
  const double deltaZ = forward.z() - backward.z();
  m_nominalMovePerZ = ROOT::Math::XYVector((forward.x() - backward.x()) / deltaZ, (forward.y() - backward.y()) / deltaZ);

  B2ASSERT("Wire reference position is not at 0", m_refPos3D.z() == 0);
}

WireLine WireLine::movedBy(const ROOT::Math::XYZVector& offset) const
{
  WireLine moved = *this;
  const ROOT::Math::XYVector tmp = nominalMovePerZ();
  moved.m_refPos3D += ROOT::Math::XYZVector(offset.X(), offset.Y(), 0) + ROOT::Math::XYZVector(tmp.X(), tmp.Y(), 0)  * offset.z();
  moved.m_forwardZ += offset.z();
  moved.m_backwardZ += offset.z();
  return moved;
}

WireLine WireLine::movedBy(const ROOT::Math::XYVector& offset) const
{
  WireLine moved = *this;
  moved.m_refPos3D += ROOT::Math::XYZVector(offset.X(), offset.Y(), 0);
  return moved;
}
