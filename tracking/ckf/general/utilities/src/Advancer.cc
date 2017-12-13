/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/general/utilities/Advancer.h>

#include <framework/core/ModuleParamList.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>
#include <genfit/MaterialEffects.h>
#include <genfit/Exception.h>

using namespace Belle2;

double Advancer::extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                    const genfit::SharedPlanePtr& plane)
{
  setMaterialEffectsToParameterValue();

  double returnValue = NAN;
  try {
    const double extrapolatedS = measuredStateOnPlane.extrapolateToPlane(plane);

    if (m_param_direction * extrapolatedS <= 0) {
      returnValue = m_param_direction * extrapolatedS;
    }
  } catch (const genfit::Exception& e) {
    B2DEBUG(50, "Extrapolation failed: " << e.what());
  }

  resetMaterialEffects();
  return returnValue;
}

void Advancer::setMaterialEffectsToParameterValue() const
{
  genfit::MaterialEffects::getInstance()->setNoEffects(not m_param_useMaterialEffects);
}

void Advancer::resetMaterialEffects() const
{
  genfit::MaterialEffects::getInstance()->setNoEffects(false);
}

void Advancer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useMaterialEffects"),
                                m_param_useMaterialEffects,
                                "Use the material effects during extrapolation.",
                                m_param_useMaterialEffects);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "direction"),
                                m_param_direction,
                                "If direction != 0, forbid any extrapolation into the other direction.",
                                m_param_direction);
}
