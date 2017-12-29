/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/utilities/PXDAdvancer.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/dataobjects/PXDCluster.h>

using namespace Belle2;

double PXDAdvancer::extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                       const SpacePoint& spacePoint)
{
  PXDRecoHit recoHit(spacePoint.getRelated<PXDCluster>());
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);
  return Advancer::extrapolateToPlane(measuredStateOnPlane, plane);
}
