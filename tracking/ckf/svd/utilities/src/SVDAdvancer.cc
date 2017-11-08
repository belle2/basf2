/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/utilities/SVDAdvancer.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

double SVDAdvancer::extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                       const SpacePoint& spacePoint)
{
  SVDRecoHit recoHit(spacePoint.getRelated<SVDCluster>());
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);
  return Advancer::extrapolateToPlane(measuredStateOnPlane, plane);
}
