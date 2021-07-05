/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>

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
  class Advancer : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// Extrapolate the mSoP of one plane to another plane and return the traveled distance or NAN, if travelling into the wrong direction.
    double extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                              const genfit::SharedPlanePtr& plane) const;

    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Call this to set the material effects handling accorinf to the given parameters.
    void setMaterialEffectsToParameterValue() const;

    /// Call this to reset the material effects back to their default value (turned on).
    void resetMaterialEffects() const;

    /// Convert the string parameter to a valid forward backward information
    void initialize() final;

  private:
    /// Use material effects during extrapolation or not.
    bool m_param_useMaterialEffects = false;

    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_directionAsString = "both";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_direction = TrackFindingCDC::EForwardBackward::c_Unknown;
  };
}