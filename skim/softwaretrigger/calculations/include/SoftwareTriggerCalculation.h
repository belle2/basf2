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
#include <skim/softwaretrigger/core/SoftwareTriggerVariableManager.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Abstract base class for all calculation algorithms to be used when calculating
     * the variables needed in the SoftwareTrigger modules for the cuts.
     *
     * Overload this class to implement your own calculations.
     */
    class SoftwareTriggerCalculation {
    public:
      /**
       * Function to be called in the initialize phase of the module, in which all later needed
       * StoreArrays can be made a requirement.
       * Override it in your class if you need special store arrays.
       */
      virtual void requireStoreArrays() { };

      /**
       * Function to write out debug output into the given TTree.
       * Needs an already prefilled calculationResult for this
       * (probably using the fillInCalculations function).
       */
      void writeDebugOutput(const SoftwareTriggerObject& calculationResult,
                            const TTree& debugOutputTTree) const
      {

        B2FATAL("This function is not implemented in the moment");
      }

      /**
       * Main function of this class: calculate the needed variables using the
       * implemented soCalculation function and write out the values into the
       * results object (with their names).
       *
       * Before this, remove all old calculations from the result.
       *
       * What variables exactly are added to the result depends on the implementation
       * details of the class.
       */
      void fillInCalculations(SoftwareTriggerObject& calculationResult) const
      {
        calculationResult.clear();
        doCalculation(calculationResult);
      }

    private:
      /// Abstract function for doing the calculations.
      virtual void doCalculation(SoftwareTriggerObject& calculationResult) const = 0;
    };
  }
}