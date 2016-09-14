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
#include <TTree.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Templated base class for all calculation algorithms to be used when calculating
     * the variables needed in the SoftwareTrigger modules for the cuts.
     *
     * Put in your own calculation here as a template argument. This algorithm class must
     * have a function requireStoreArray (which can be empty) and a function doCalculation
     * getting the SoftwareTriggerObject as a reference.
     */
    template <class ACalculator>
    class SoftwareTriggerCalculation {
    public:
      /**
       * Function to be called in the initialize phase of the module, in which all later needed
       * StoreArrays can be made a requirement.
       * Override it in your class if you need special store arrays.
       */
      void requireStoreArrays()
      {
        m_calculatorAlgorithm.requireStoreArrays();
      };

      /**
       * Function to write out debug output into the given TTree.
       * Needs an already prefilled calculationResult for this
       * (probably using the fillInCalculations function).
       */
      void writeDebugOutput(const std::unique_ptr<TTree>& debugOutputTTree)
      {
        if (not m_debugPrepared) {
          for (auto& identifierWithValue : m_calculationResult) {
            const std::string& identifier = identifierWithValue.first;
            double& value = identifierWithValue.second;

            debugOutputTTree->Branch(identifier.c_str(), &value);
          }
          m_debugPrepared = true;
        }

        debugOutputTTree->Fill();
      }

      /**
       * Main function of this class: calculate the needed variables using the
       * implemented doCalculation function of the given algorithm and write out
       * the values into the results object (with their names).
       *
       * Please make sure to override the variables! Otherwise is can happen that their
       * old values are still in the object.
       *
       * What variables exactly are added to the result depends on the implementation
       * details of the class.
       */
      const SoftwareTriggerObject& fillInCalculations()
      {
        const unsigned int sizeBeforeCheck = m_calculationResult.size();
        m_calculatorAlgorithm.doCalculation(m_calculationResult);

        if (m_calculationResult.size() != sizeBeforeCheck and sizeBeforeCheck > 0) {
          B2WARNING("Calculator added more variables (" << m_calculationResult.size() <<
                    ") than there were before (" << sizeBeforeCheck << "). Probably something strange is going on!");
        }

        return m_calculationResult;
      }

    private:
      /// Internal storage of the underlaying algorithm class implemented by the user.
      ACalculator m_calculatorAlgorithm;
      /// Internal storage of the result of the calculation.
      SoftwareTriggerObject m_calculationResult;
      /// Flag to not add the branches twice to the TTree.
      bool m_debugPrepared = false;
    };
  }
}