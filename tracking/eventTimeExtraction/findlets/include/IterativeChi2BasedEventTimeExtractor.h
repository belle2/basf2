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
  /// Class to iteratively extract the event t0 using the chi-squared approach
  class IterativeChi2BasedEventTimeExtractor final : public IterativeEventTimeExtractor<Chi2BasedEventTimeExtractor> {
  private:
    /// Type of the base class
    using Super = IterativeEventTimeExtractor<Chi2BasedEventTimeExtractor>;

  public:
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->getParameter<bool>(TrackFindingCDC::prefixed(prefix, "useLastEventT0")).setDefaultValue(false);
    }
  };
}
