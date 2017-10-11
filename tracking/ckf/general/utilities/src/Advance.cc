/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/general/utilities/Advance.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/MaterialEffects.h>
#include <genfit/Exception.h>

using namespace Belle2;

double Advancer::extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                    const genfit::SharedPlanePtr& plane, double direction)
{
  try {
    genfit::MaterialEffects::getInstance()->setNoEffects(not m_param_useMaterialEffects);
    const double extrapolatedS = measuredStateOnPlane.extrapolateToPlane(plane);
    genfit::MaterialEffects::getInstance()->setNoEffects(false);

    if (direction * extrapolatedS > 0) {
      return NAN;
    } else {
      return direction * extrapolatedS;
    }
  } catch (const genfit::Exception& e) {
    B2DEBUG(50, "Extrapolation failed: " << e.what());
    return NAN;
  }
}

void Advancer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useMaterialEffects"),
                                m_param_useMaterialEffects,
                                "",
                                m_param_useMaterialEffects);
}
