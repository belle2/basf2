/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/fitter/timeEstimator/PlaneTriggerTrackTimeEstimatorModule.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>

using namespace std;
using namespace Belle2;

REG_MODULE(PlaneTriggerTrackTimeEstimator)

PlaneTriggerTrackTimeEstimatorModule::PlaneTriggerTrackTimeEstimatorModule() : BaseTrackTimeEstimatorModule()
{
  addParam("triggerPlanePosition", m_param_triggerPlanePosition, "3-Position of the plane of the trigger.",
           m_param_triggerPlanePosition);

  addParam("triggerPlaneDirection", m_param_triggerPlaneNormalDirection, "3-Normal direction of the plane of the trigger.",
           m_param_triggerPlaneNormalDirection);
}

double PlaneTriggerTrackTimeEstimatorModule::estimateFlightLengthUsingFittedInformation(genfit::MeasuredStateOnPlane&
    measuredStateOnPlane)
const
{
  B2ASSERT("Position must have 3 components.", m_param_triggerPlanePosition.size() == 3);
  B2ASSERT("Normal direction must have 3 components.", m_param_triggerPlaneNormalDirection.size() == 3);

  TVector3 triggerPlanePosition(m_param_triggerPlanePosition[0], m_param_triggerPlanePosition[1], m_param_triggerPlanePosition[2]);
  TVector3 triggerPlaneNormalDirection(m_param_triggerPlaneNormalDirection[0], m_param_triggerPlaneNormalDirection[1],
                                       m_param_triggerPlaneNormalDirection[2]);

  try {
    const double s = measuredStateOnPlane.extrapolateToPlane(genfit::SharedPlanePtr(new genfit::DetPlane(triggerPlanePosition,
                                                             triggerPlaneNormalDirection)));
    // Negative, because we extrapolated in the wrong direction
    return -s;
  } catch (const genfit::Exception& e) {
    B2WARNING("Extrapolation failed.");
    return 0;
  }

}

double PlaneTriggerTrackTimeEstimatorModule::estimateFlightLengthUsingSeedInformation(const RecoTrack& recoTrack) const
{
  B2ASSERT("Position must have 3 components.", m_param_triggerPlanePosition.size() == 3);
  B2ASSERT("Normal direction must have 3 components.", m_param_triggerPlaneNormalDirection.size() == 3);

  const TVector3& momentum = recoTrack.getMomentumSeed();
  const short int charge = recoTrack.getChargeSeed();
  const TVector3& position = recoTrack.getPositionSeed();

  const Helix h(position, momentum, charge, 1.5);
  double arcLengthOfIntersection;

  TVector3 triggerPlanePosition(m_param_triggerPlanePosition[0], m_param_triggerPlanePosition[1], m_param_triggerPlanePosition[2]);
  TVector3 triggerPlaneNormalDirection(m_param_triggerPlaneNormalDirection[0], m_param_triggerPlaneNormalDirection[1],
                                       m_param_triggerPlaneNormalDirection[2]);

  // Currently, there are only one cases implemented!
  // Case 1: Plane for a certain z:
  if (triggerPlaneNormalDirection.X() == triggerPlaneNormalDirection.Y() == 0) {
    // This is the easiest case: We just have so solve tan lambda * arcLength + z0 = p_z
    arcLengthOfIntersection = (triggerPlanePosition.Z() - h.getZ0()) / h.getTanLambda();
  } else if (triggerPlaneNormalDirection.Z() == 0) {
    // This case is a bit harder. We have to find solutions for n_x * x + n_y * y = n_x * p_x + n_y * p_y
    // with n_i the i-th component of the normal vector, p_i the i-th component of the position vector
    // and x or y given by the helix function.

    // TODO
    B2FATAL("This case for the normal direction is not implemented for using tracking seeds!");
  } else {
    // All the other cases are not algebraically solvable. As I do not think they will be necessary, we do not
    // spend time on constructing a numerical solution here.
    B2FATAL("This case for the normal direction is not implemented for using tracking seeds!");
  }

  const double s = arcLengthOfIntersection * hypot(1, h.getTanLambda());
  return s;
}
