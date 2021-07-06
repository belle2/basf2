/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
       * (filter and skim). The return value is a bool, which has the values accept (true) and reject (false):
       * * accept if and only if the filter stage has accepted the event.
       *   The event is also accepted if none of the triggers have run.
       * * reject if the filter stage rejected the event
       *
       * If the final result is already stored to the result, it is returned immediately.
       * Except for forgetTotalResult is st to true.
       *
       * Deprecated: in an older version, the trigger stages were called "hlt", "fast_reco" and "skim".
       * They are still supported by the function to read in old results, but should not be used anymore
       * for new data. A warning will be issued.
       */
      static bool getFinalTriggerDecision(const SoftwareTriggerResult& result, bool forgetTotalResult = false);

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
