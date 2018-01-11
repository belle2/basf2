/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Malwin Weiler, Nils Braun                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/vxdCDCTrackMerger/FilterBasedVXDCDCTrackMergerFindlet.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParam.h>

namespace Belle2 {
  /// This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC
  class FilterBasedVXDCDCTrackMergerModule : public TrackFindingCDC::FindletModule<FilterBasedVXDCDCTrackMergerFindlet> {
  public:
    FilterBasedVXDCDCTrackMergerModule()
    {
      setDescription(
        "This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");

      ModuleParamList moduleParamList = this->getParamList();
      moduleParamList.getParameter<std::string>("distanceFilter").setDefaultValue("pass_through");
      this->setParamList(moduleParamList);
    }
  };
}
