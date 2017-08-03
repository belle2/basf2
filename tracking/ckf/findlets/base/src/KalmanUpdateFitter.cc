/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template <>
double KalmanUpdateFitter::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                      const SpacePoint& spacePoint) const
{
  if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
    double chi2 = 0;
    for (const SVDCluster& relatedCluster : spacePoint.getRelationsTo<SVDCluster>()) {
      SVDRecoHit recoHit(&relatedCluster);
      chi2 += kalmanStep(measuredStateOnPlane, recoHit);
    }
    return chi2;
  } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
    PXDRecoHit recoHit(spacePoint.getRelated<PXDCluster>());
    return kalmanStep(measuredStateOnPlane, recoHit);
  } else {
    B2FATAL("Can not fit unknown type " << spacePoint.getType() << "!");
  }
}

template <>
double KalmanUpdateFitter::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane __attribute__((unused)),
                                      const TrackFindingCDC::CDCRLWireHit& rlWireHit __attribute__((unused))) const
{
  // TODO: Do something here
  return -1;
}