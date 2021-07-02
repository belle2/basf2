/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/core/utilities.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    std::unique_ptr<SoftwareTriggerCut> SoftwareTriggerCut::compile(const std::string& cut_string,
        unsigned int prescaleFactor,
        const bool rejectCut)
    {
      auto compiledGeneralCut = GeneralCut<SoftwareTriggerVariableManager>::compile(cut_string);
      std::unique_ptr<SoftwareTriggerCut> compiledSoftwareTriggerCut(new SoftwareTriggerCut(std::move(compiledGeneralCut),
          prescaleFactor, rejectCut));

      return compiledSoftwareTriggerCut;
    }

    std::pair<SoftwareTriggerCutResult, SoftwareTriggerCutResult> SoftwareTriggerCut::check(const
        SoftwareTriggerVariableManager::Object& prefilledObject, uint32_t* counter) const
    {
      if (not m_cut) {
        B2FATAL("Software Trigger is not initialized!");
      }
      const bool cutCondition = m_cut->check(&prefilledObject);

      // If the cut is a reject cut, return false if the cut is true and false if the cut is true.
      if (isRejectCut()) {
        if (cutCondition) {
          return {SoftwareTriggerCutResult::c_reject, SoftwareTriggerCutResult::c_reject};
        } else {
          return {SoftwareTriggerCutResult::c_noResult, SoftwareTriggerCutResult::c_noResult};
        }
      } else {
        // This is the "normal" accept case:
        // First check if the cut gives a positive result. If not, we can definitely return "noResult".
        if (cutCondition) {
          // if yes, we have to use the prescale factor to see, if the result is really yes.
          if (makePreScale(getPreScaleFactor(), counter)) {
            return {SoftwareTriggerCutResult::c_accept, SoftwareTriggerCutResult::c_accept};
          } else {
            // This is the only case were prescaled and non-prescaled results are different.
            return {SoftwareTriggerCutResult::c_noResult, SoftwareTriggerCutResult::c_accept};
          }
        } else {
          return {SoftwareTriggerCutResult::c_noResult, SoftwareTriggerCutResult::c_noResult};
        }
      }
    }

    SoftwareTriggerCutResult SoftwareTriggerCut::checkPreScaled(const SoftwareTriggerVariableManager::Object& prefilledObject) const
    {
      return check(prefilledObject).first;
    }
  }
}
