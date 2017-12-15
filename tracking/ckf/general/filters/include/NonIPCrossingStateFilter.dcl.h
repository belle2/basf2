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

  template <class AllStateFilter>
  class NonIPCrossingStateFilter : public AllStateFilter {
    using Object = typename AllStateFilter::Object;
    using Super = AllStateFilter;
  public:
    TrackFindingCDC::Weight operator()(const Object& pair) final;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    void initialize() final;

  private:
    /// Parameter for the distance given to the framework (can not handle EForwardBackward directly)
    std::string m_param_directionAsString = "unknown";
    /// Direction parameter converted from the string parameters
    TrackFindingCDC::EForwardBackward m_param_direction = TrackFindingCDC::EForwardBackward::c_Unknown;
  };
}
