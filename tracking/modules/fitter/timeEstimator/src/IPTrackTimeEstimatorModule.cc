/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/fitter/timeEstimator/IPTrackTimeEstimatorModule.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/geometry/B2Vector3.h>

using namespace Belle2;

REG_MODULE(IPTrackTimeEstimator)

double IPTrackTimeEstimatorModule::estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane& measuredStateOnPlane)
const
{
  const B2Vector3D ipPosition;
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
  const ROOT::Math::XYZVector& position = recoTrack.getPositionSeed();
  const ROOT::Math::XYZVector& momentum = recoTrack.getMomentumSeed();
  const short int charge = recoTrack.getChargeSeed();

  const double bZ = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
  const Helix h(position, momentum, charge, bZ);
  const double s2D = h.getArcLength2DAtXY(position.X(), position.Y());
  const double s = s2D * hypot(1, h.getTanLambda());

  return s;
}
