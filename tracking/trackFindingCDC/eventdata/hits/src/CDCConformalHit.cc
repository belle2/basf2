/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCConformalHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

CDCConformalHit::CDCConformalHit(const CDCWireHit* wireHit)
  : m_wireHit(wireHit)
{
  assert(wireHit);
  std::tie(m_conformalPos2D, m_conformalDriftLength) = performConformalTransformWithRespectToPoint(Vector2D(0, 0));
}

std::tuple<Vector2D, double> CDCConformalHit::performConformalTransformWithRespectToPoint(const Vector2D& pos2D) const
{
  Circle2D conformalDriftCircle = m_wireHit->conformalTransformed(pos2D);

  // TODO : Resolve sad mismatch between the legendre conformal transformation and the one defined in the reset of the CDC tracking.
  return std::make_tuple(conformalDriftCircle.center() * 2,
                         conformalDriftCircle.radius() * 2);
}
