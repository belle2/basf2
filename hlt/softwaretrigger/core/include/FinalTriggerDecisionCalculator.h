/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <mdst/dataobjects/SoftwareTriggerResult.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper class to give the getFinalTriggerDecision to python
    class FinalTriggerDecisionCalculator {
    public:
      /**
       * Calculate the final cut decision using all "total_results" of all sub triggers in the software trigger
       * (trigger and skim). The return value is a bool, which has the values accept (true) and reject (false):
       * * accept if and only if the trigger stage has accepted the event.
       *   The event is also accepted if none of the triggers have run.
       * * reject if the trigger stage rejected the event
       */
      static bool getFinalTriggerDecision(const SoftwareTriggerResult& result);

      /**
       * Calculate the "total_result" for a given base identifier by looping through all results with the given
       * base identifier. The result of the function then depends on the setting of accept overrides
       * reject. Please see the SoftwareTriggerModule description, for more information.
       */
      static SoftwareTriggerCutResult getModuleResult(const SoftwareTriggerResult& result, const std::string& baseIdentifier,
                                                      bool acceptOverridesReject);
    };
  }
}
