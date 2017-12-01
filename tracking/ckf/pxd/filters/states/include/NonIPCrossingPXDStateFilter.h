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

#include <tracking/ckf/pxd/filters/states/AllPXDStateFilter.h>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  class NonIPCrossingPXDStateFilter : public AllPXDStateFilter {
  public:
    TrackFindingCDC::Weight operator()(const AllPXDStateFilter::Object& pair) final;

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    double m_param_direction = 1;
  };
}
