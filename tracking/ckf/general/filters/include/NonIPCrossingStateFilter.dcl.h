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

#include <tracking/ckf/svd/filters/states/AllSVDStateFilter.h>
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Reusable filter for checking the direction of a new state if it is in the correct orientation
   * (forwards or backwards) by their arc length. Especially, the following is checked:
   *
   *    arc length of this hit < arc length of the origin (backward) or > (forward)
   *    arc length of this hit > arc length of the hit before (backward) or < (forward)
   *
   * All the arc lengths are calculated using a trajectory based on the last valid mSoP (always pointing in
   * forward direction).
   */
  template <class AllStateFilter>
  class NonIPCrossingStateFilter : public AllStateFilter {
    using Object = typename AllStateFilter::Object;
    using Super = AllStateFilter;
  public:
    /// Main function testing the object for the direction
    TrackFindingCDC::Weight operator()(const Object& pair) final;

    /// Expose the direction parameter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Copy the string direction parameter to the enum.
    void initialize() final;

  private:
    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_directionAsString = "unknown";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_direction = TrackFindingCDC::EForwardBackward::c_Unknown;
  };
}
