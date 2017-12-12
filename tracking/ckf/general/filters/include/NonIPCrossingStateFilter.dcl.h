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
#include <string>

namespace Belle2 {
  class ModuleParamList;

  template <class AllStateFilter>
  class NonIPCrossingStateFilter : public AllStateFilter {
    using Object = typename AllStateFilter::Object;
  public:
    TrackFindingCDC::Weight operator()(const Object& pair) final;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    double m_param_direction = 1;
  };
}
