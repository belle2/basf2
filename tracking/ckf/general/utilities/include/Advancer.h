/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/SharedPlanePtr.h>

#include <string>

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {
  class ModuleParamList;

  /**
   * Helper findlet for performing an extrapolation of a mSoP of one plane to another plane
   * using the representation stored in the mSoP.
   *
   * Will return the travelled distance or NAN, if
   *    * the extrapolation failed or
   *    * the extrapolation was into another direction as the direction parameter. See below.
   */
  class Advancer {
  public:
    /// Extrapolate the mSoP of one plane to another plane and return the traveled distance or NAN, if travelling into the wrong direction.
    double extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                              const genfit::SharedPlanePtr& plane);

    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Call this to set the material effects handling accorinf to the given parameters.
    void setMaterialEffectsToParameterValue() const;

    /// Call this to reset the material effects back to their default value (turned on).
    void resetMaterialEffects() const;

  private:
    /// Use material effects during extrapolation or not.
    bool m_param_useMaterialEffects = false;

    /**
     * Check the traveled orientation after the extrapolation and return NAN, if extrapolation
     * into the wrong direction.
     * Direction has 3 possible values:
     *   * 0: all orientations are fine and will return a not-NAN value.
     *   * 1: only extrapolations *against* the direction of the mSoP are fine.
     *   * -1 only extrapolations *with* the direction of the mSoP are fine.
     */
    double m_param_direction = 1;
  };
}