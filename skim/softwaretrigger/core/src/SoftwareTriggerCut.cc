/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <skim/softwaretrigger/core/SoftwareTriggerCut.h>
#include <skim/softwaretrigger/dataobjects/SoftwareTriggerResult.h>
#include <skim/softwaretrigger/core/utilities.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerCut::compile(const std::string& cut_string,
        const std::vector<unsigned int>& prescaleFactor,
        const bool rejectCut)
    {
      auto compiledGeneralCut = GeneralCut<SoftwareTriggerVariableManager>::compile(cut_string);
      std::unique_ptr<SoftwareTriggerCut> compiledSoftwareTriggerCut(new SoftwareTriggerCut(std::move(compiledGeneralCut),
          prescaleFactor, rejectCut));

      return compiledSoftwareTriggerCut;
    }

    /**
     * Main function of the SoftwareTriggerCut: check the cut condition.
     * See the constructor of this class for more information on when which result is returned.
     */
    SoftwareTriggerCutResult SoftwareTriggerCut::checkPreScaled(const SoftwareTriggerVariableManager::Object& prefilledObject) const
    {
      if (not m_cut) {
        B2FATAL("Software Trigger is not initialized!");
      }
      const bool cutCondition = m_cut->check(&prefilledObject);

      // If the cut is a reject cut, return false if the cut is true and false if the cut is true.
      if (isRejectCut()) {
        if (cutCondition) {
          return SoftwareTriggerCutResult::c_reject;
        } else {
          return SoftwareTriggerCutResult::c_noResult;
        }
      } else {
        // This is the "normal" accept case:
        // First check if the cut gives a positive result. If not, we can definitely return "noResult".
        if (cutCondition) {
          // if yes, we have to use the prescale factor to see, if the result is really yes.
          if (makePreScale(getPreScaleFactor())) {
            return SoftwareTriggerCutResult::c_accept;
          }
        }

        return SoftwareTriggerCutResult::c_noResult;
      }
    }
  }
}
