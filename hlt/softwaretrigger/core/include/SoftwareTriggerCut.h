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

#include <mdst/dbobjects/SoftwareTriggerCutBase.h>
#include <framework/utilities/GeneralCut.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <stdint.h>

namespace Belle2 {

  namespace SoftwareTrigger {
    /**
     * Software Trigger Cut to be used in the Software Trigger Modules.
     * This cut can be down- and uploaded from the database or compiled from a string.
     * It is basically a GeneralCut using the SoftwareTriggerVariableManager with a prescale factor.
     *
     * For database interactions, use the SoftwareTriggerDBHandler.
     */
    class SoftwareTriggerCut : public SoftwareTriggerCutBase {
    public:
      /**
       * Compile a new SoftwareTriggerCut from a cut string (by using the GeneralCut::compile function) and
       * an optional prescale factor. This together with the downloadFromDatabase function is the only possibility to
       * create a new SoftwareTriggerCut.
       * @param cut_string: The string from which the cut should be compiled. Must be in a format, the GeneralCut::compile function understands.
       * @param prescaleFactor: An optional prescale factor which will be used whenever the cut is checked.
       *        The prescale factor is a integer value. If the prescale is e.g. 10, the cut will only result in a "accept" result
       *        (although the cut condition itself is true) in 1 of 10 cases on average for accept cuts
       *        (for reject cuts, the prescale is *not* used).
       *        Defaults to 1, which means that the prescale has no impact on the cut check.
       * @param rejectCut: Turn this cut into a reject cut and not a accept cut. The result of the SoftwareTriggerModules
       *        is defined by these two cut types. See the SoftwareTriggerModule for more information. Please note that
       *        the condition is turned the other way round if it is a reject cut. See the example below.
       * @return a unique_ptr on a SoftwareTriggerCut with the given cut condition and prescale.
       *
       * To summarize: the cut has the following result:
       * * if the cut is an accept cut:
       *    * if the cut condition is true, it returns *accept* in 1 out of prescaleFactor cases
       *      and *noResult* otherwise.
       *    * if the cut condition is false, it returns *noResult*.
       * * if the cut is a reject cut:
       *    * if the cut condition is true, it returns *dismiss*.
       *    * if the cut condition is false, it returns *noResult*.
       *
       * Let us look into two examples. The first is an accept cut for gg events. It may have the following parameters:
       *
       *    cut condition = something which is only true for gg events
       *    prescaleFactor = 100
       *    rejectCut = false
       *
       * If the event is a gg event (and the cut conditions is then true), the cut will return "accept" in 1% of the cases,
       * and "noResult" in all other cases. If the event is not a gg event, the cut will always return "noResult".
       * The SoftwareTriggerModule will keep the event if this trigger results in "accept",
       * in all other cases it depends on the other loaded cuts.
       *
       * The second cut is an reject cut for ee events. It may have the following parameters:
       *
       *    cut condition = something which is only true for ee events
       *    rejectCut = true
       *    prescaleFactor will not be used
       *
       * If the event is an ee event (and the cut condition is true), the cut will result in "dismiss". In all other cases
       * it returns "noResult".
       * The SoftwareTriggerModule will not pass this event if the result is
       * "dismiss", in all other cases it depends on the other loaded cuts.
       *
       */
      static std::unique_ptr<SoftwareTriggerCut> compile(const std::string& cut_string,
                                                         const unsigned int prescaleFactor,
                                                         const bool rejectCut = false);

      /**
       * Decompile the internal General Cut back into a string.
       * This function is needed for streaming the SoftwareTriggerCut into the database.
       */
      std::string decompile() const
      {
        return m_cut->decompile();
      }

      /**
       * Main function of the SoftwareTriggerCut: check the cut condition.
       * See the constructor of this class for more information on when which result is returned.
       */
      SoftwareTriggerCutResult checkPreScaled(const SoftwareTriggerVariableManager::Object& prefilledObject) const;

      /**
       * Return both the prescaled and the non-prescaled result. This function should only be use experts, you basically always want to
       * use the checkPreScaled function.
       * Returns a pair [prescaled, non-prescaled]
       */
      std::pair<SoftwareTriggerCutResult, SoftwareTriggerCutResult> check(const SoftwareTriggerVariableManager::Object& prefilledObject,
          uint32_t* counter = nullptr)
      const;

    private:
      /// Internal representation of the cut condition as a general cut.
      std::unique_ptr<GeneralCut<SoftwareTriggerVariableManager>> m_cut = nullptr;

      /**
      * Make constructor private. You should only download a SoftwareCut from the database or compile a new one from a string.
      */
      SoftwareTriggerCut(std::unique_ptr<GeneralCut<SoftwareTriggerVariableManager>>&& cut,
                         unsigned int prescaleFactor,
                         const bool rejectCut) : SoftwareTriggerCutBase(prescaleFactor, rejectCut),
        m_cut(std::move(cut))
      {
      }
    };
  }
}
