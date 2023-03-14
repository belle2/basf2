/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/fitter/timeEstimator/RadiusTrackTimeEstimatorModule.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

REG_MODULE(RadiusTrackTimeEstimator);

RadiusTrackTimeEstimatorModule::RadiusTrackTimeEstimatorModule() : BaseTrackTimeEstimatorModule()
{
  addParam("radiusForExtrapolation", m_param_radiusForExtrapolation,
           "Radius used for extrapolation. Please be aware that if the RecoTrack does not reach this radius, "
           "the results are wrong.", m_param_radiusForExtrapolation);
}

double RadiusTrackTimeEstimatorModule::estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane&
    measuredStateOnPlane)
const
{
  try {
    const double s = measuredStateOnPlane.extrapolateToCylinder(m_param_radiusForExtrapolation);
    return s;
  } catch (const genfit::Exception& e) {
    B2WARNING("Extrapolation failed: " << e.what());
    return 0;
  }
}

double RadiusTrackTimeEstimatorModule::estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const
{
  const ROOT::Math::XYZVector& momentum = recoTrack.getMomentumSeed();
  const short int charge = recoTrack.getChargeSeed();
  const ROOT::Math::XYZVector& position = recoTrack.getPositionSeed();

  const double bZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  const Helix h(position, momentum, charge, bZ);
  const double arcLengthOfIntersection = h.getArcLength2DAtCylindricalR(m_param_radiusForExtrapolation);
  const double s = arcLengthOfIntersection * hypot(1, h.getTanLambda());
  return s;
}
