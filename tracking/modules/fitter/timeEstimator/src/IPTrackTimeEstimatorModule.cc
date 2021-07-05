/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/timeEstimator/IPTrackTimeEstimatorModule.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

using namespace std;
using namespace Belle2;

REG_MODULE(IPTrackTimeEstimator)

double IPTrackTimeEstimatorModule::estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane)
const
{
  const TVector3 ipPosition;
  try {
    const double s = measuredStateOnPlane.extrapolateToPoint(ipPosition);
    // Negative, because we extrapolated in the wrong direction
    return -s;
  } catch (const genfit::Exception& e) {
    B2WARNING("Extrapolation failed: " << e.what());
    return 0;
  }
}

double IPTrackTimeEstimatorModule::estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const
{
  const TVector3& position = recoTrack.getPositionSeed();
  const TVector3& momentum = recoTrack.getMomentumSeed();
  const short int charge = recoTrack.getChargeSeed();

  const double bZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  const Helix h(position, momentum, charge, bZ);
  const double s2D = h.getArcLength2DAtXY(position.X(), position.Y());
  const double s = s2D * hypot(1, h.getTanLambda());

  return s;
}
