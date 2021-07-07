/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/general/utilities/Advancer.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>
#include <genfit/MaterialEffects.h>
#include <genfit/Exception.h>

using namespace Belle2;
using namespace TrackFindingCDC;

double Advancer::extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                    const genfit::SharedPlanePtr& plane) const
{
  setMaterialEffectsToParameterValue();

  double returnValue = NAN;
  try {
    const double extrapolatedS = measuredStateOnPlane.extrapolateToPlane(plane);

    if (arcLengthInRightDirection(extrapolatedS, m_param_direction)) {
      returnValue = m_param_direction * extrapolatedS;
    }
  } catch (const genfit::Exception& e) {
    B2DEBUG(29, "Extrapolation failed: " << e.what());
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
  moduleParamList->addParameter(prefixed(prefix, "useMaterialEffects"),
                                m_param_useMaterialEffects,
                                "Use the material effects during extrapolation.",
                                m_param_useMaterialEffects);

  moduleParamList->addParameter(prefixed(prefix, "direction"), m_param_directionAsString,
                                "The direction where the extrapolation will happen.");
}

void Advancer::initialize()
{
  ProcessingSignalListener::initialize();

  m_param_direction = fromString(m_param_directionAsString);
}
