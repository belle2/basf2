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

#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TTree.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Base class for all calculation algorithms to be used when calculating
     * the variables needed in the SoftwareTrigger modules for the cuts.
     *
     * Override in your own calculation. Your override class must
     * have a function requireStoreArray (which can be empty) and a function doCalculation
     * getting the SoftwareTriggerObject as a reference.
     */
    class SoftwareTriggerCalculation {
    public:
      /**
       * Function to be called in the initialize phase of the module, in which all later needed
       * StoreArrays can be made a requirement.
       * Override it in your class if you need special store arrays.
       */
      virtual void requireStoreArrays() = 0;

      /**
       * Function to write out debug output into the given TTree.
       * Needs an already prefilled calculationResult for this
       * (probably using the fillInCalculations function).
       */
      void writeDebugOutput(const std::unique_ptr<TTree>& debugOutputTTree);

      /**
       * Function to write out debug output into the given StoreObject.
       * Needs an already prefilled calculationResult for this
       * (probably using the fillInCalculations function).
       * All added variables are prefixed with the given prefix string.
       */
      void addDebugOutput(const StoreObjPtr<SoftwareTriggerVariables>& storeObject, const std::string& prefix);

      /**
       * Main function of this class: calculate the needed variables using the
       * overwritten doCalculation function and write out
       * the values into the results object (with their names).
       *
       * Please make sure to override (or clear) the variables! Otherwise it can happen that their
       * old values are still in the object.
       *
       * What variables exactly are added to the result depends on the implementation
       * details of the class.
       */
      const SoftwareTriggerObject& fillInCalculations();

      /// Override this function to implement your calculation.
      virtual void doCalculation(SoftwareTriggerObject& m_calculationResult) = 0;

    private:
      /// Internal storage of the result of the calculation.
      SoftwareTriggerObject m_calculationResult;
      /// Flag to not add the branches twice to the TTree.
      bool m_debugPrepared = false;
    };
  }
}