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
#include <framework/geometry/BFieldManager.h>

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
    B2WARNING("Extrapolation failed: " << e.what());
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

  const double bZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  const Helix h(position, momentum, charge, bZ);

  const double arcLengthAtPosition = h.getArcLength2DAtXY(position.X(), position.Y());

  double arcLengthOfIntersection;

  TVector3 triggerPlanePosition(m_param_triggerPlanePosition[0], m_param_triggerPlanePosition[1], m_param_triggerPlanePosition[2]);
  TVector3 triggerPlaneNormalDirection(m_param_triggerPlaneNormalDirection[0], m_param_triggerPlaneNormalDirection[1],
                                       m_param_triggerPlaneNormalDirection[2]);

  // Currently, there are only one cases implemented!
  // Case 1: Plane for a certain z:
  if (triggerPlaneNormalDirection.X() == 0 and triggerPlaneNormalDirection.Y() == 0) {
    // This is the easiest case: We just have so solve tan lambda * arcLength + z0 = p_z
    arcLengthOfIntersection = (triggerPlanePosition.Z() - h.getZ0()) / h.getTanLambda();
  } else if (triggerPlaneNormalDirection.Z() == 0) {
    // This case is a bit harder. We have to find solutions for n_x * x + n_y * y = n_x * p_x + n_y * p_y
    // with n_i the i-th component of the normal vector, p_i the i-th component of the position vector
    // and x or y given by the helix function.

    // We make our life a bit easier and rotate the plane (as well as the normal vector) by -phi0. This makes the
    // helix (and all the equations) much easier.
    triggerPlaneNormalDirection.RotateZ(-h.getPhi0());
    triggerPlanePosition.RotateZ(-h.getPhi0());

    const double n_x = triggerPlaneNormalDirection.X();
    const double n_y = triggerPlaneNormalDirection.Y();

    // We first define n_x * p_x + n_y * p_y as alpha
    const double alpha = triggerPlanePosition.Dot(triggerPlaneNormalDirection);

    // we do a case distinction between cosmics and "normal" tracks:
    if (fabs(h.getOmega()) < 1e-3) {
      // in case of cosmics, this is quite easy
      const double arcLengthOfTrigger = (alpha + n_y * h.getD0()) / n_x;

      arcLengthOfIntersection = arcLengthAtPosition - arcLengthOfTrigger;
    } else {
      // And then we define a reoccuring element in the equation as beta
      const double beta = n_y * h.getOmega() * h.getD0() + alpha * h.getOmega();

      // The equation we have top solve is now
      // -n_x * A - n_y * (1 - sqrt(1 - A^2)) = beta
      // with A = sin(\chi) and cos = sqrt(1 - sin^2)

      // there are two possible solutions, and we want the positive one
      const double A1 = (sqrt(n_x * n_x * n_y * n_y - 2 * beta * n_y * n_y * n_y - beta * beta * n_y * n_y) - beta * n_x - n_x * n_y) /
                        (n_x * n_x + n_y * n_y);
      const double A2 = (-sqrt(n_x * n_x * n_y * n_y - 2 * beta * n_y * n_y * n_y - beta * beta * n_y * n_y) - beta * n_x - n_x * n_y) /
                        (n_x * n_x + n_y * n_y);

      const double x1_unrotated = -A1 / h.getOmega();
      const double y1_unrotated = -(1 - sqrt(1 - A1 * A1)) / h.getOmega() - h.getD0();
      const double x2_unrotated = -A2 / h.getOmega();
      const double y2_unrotated = -(1 - sqrt(1 - A2 * A2)) / h.getOmega() - h.getD0();

      const double x1_rotated = h.getCosPhi0() * x1_unrotated + h.getSinPhi0() * y1_unrotated;
      const double y1_rotated = -h.getSinPhi0() * x1_unrotated + h.getCosPhi0() * y1_unrotated;
      const double x2_rotated = h.getCosPhi0() * x2_unrotated + h.getSinPhi0() * y2_unrotated;
      const double y2_rotated = -h.getSinPhi0() * x2_unrotated + h.getCosPhi0() * y2_unrotated;

      // Finaly, we can calculate the arc length
      const double arcLengthOfTrigger1 = h.getArcLength2DAtXY(x1_rotated, y1_rotated);
      const double arcLengthOfTrigger2 = h.getArcLength2DAtXY(x2_rotated, y2_rotated);

      const double arcLengthOfIntersection1 = arcLengthAtPosition - arcLengthOfTrigger1;
      const double arcLengthOfIntersection2 = arcLengthAtPosition - arcLengthOfTrigger2;

      arcLengthOfIntersection = fabs(arcLengthOfIntersection1) < fabs(arcLengthOfIntersection2) ? arcLengthOfIntersection1 :
                                arcLengthOfIntersection2;
    }
  } else {
    // All the other cases are not algebraically solvable. As I do not think they will be necessary, we do not
    // spend time on constructing a numerical solution here.
    B2FATAL("This case for the normal direction is not implemented for using tracking seeds!");
  }

  const double s = arcLengthOfIntersection * hypot(1, h.getTanLambda());
  return s;
}
