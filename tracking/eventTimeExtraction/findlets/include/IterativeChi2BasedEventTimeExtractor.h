/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/IterativeEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/Chi2BasedEventTimeExtractor.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>
#include <string>

namespace Belle2 {
  class IterativeChi2BasedEventTimeExtractor final : public IterativeEventTimeExtractor<Chi2BasedEventTimeExtractor> {
  private:
    using Super = IterativeEventTimeExtractor<Chi2BasedEventTimeExtractor>;

  public:
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->getParameter<unsigned int>(TrackFindingCDC::prefixed(prefix, "iterations")).setDefaultValue(5);
      moduleParamList->getParameter<bool>(TrackFindingCDC::prefixed(prefix, "useLastEventT0")).setDefaultValue(false);
    }
  };
}
