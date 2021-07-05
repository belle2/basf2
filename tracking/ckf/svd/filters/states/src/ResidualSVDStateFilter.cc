/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/ResidualSVDStateFilter.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void ResidualSVDStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalResidual"), m_param_maximalResidual,
                                "Maximum residual between extrapolated and measured position based on Kalman step.",
                                m_param_maximalResidual);
}

Weight ResidualSVDStateFilter::operator()(const BaseSVDStateFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& previousStates = pair.first;
  CKFToSVDState* currentState = pair.second;

  const auto* spacePoint = currentState->getHit();

  genfit::MeasuredStateOnPlane firstMeasurement;
  if (currentState->mSoPSet()) {
    firstMeasurement = currentState->getMeasuredStateOnPlane();
  } else {
    firstMeasurement = previousStates.back()->getMeasuredStateOnPlane();
  }

  Vector3D position = Vector3D(firstMeasurement.getPos());
  const Vector3D hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const bool sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();
  if (not sameHemisphere) {
    return NAN;
  }

  const double residual = m_kalmanStepper.calculateResidual(firstMeasurement, *currentState);
  if (residual > m_param_maximalResidual) {
    return NAN;
  }

  // add 1e-6 to prevent divisin with 0.
  return 1 / (residual + 1e-6);
}
