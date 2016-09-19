/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/calculations/SoftwareTriggerCalculation.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void SoftwareTriggerCalculation::writeDebugOutput(const std::unique_ptr<TTree>& debugOutputTTree)
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

    void SoftwareTriggerCalculation::addDebugOutput(const StoreObjPtr<SoftwareTriggerVariable>& storeObject, const std::string& prefix)
    {
      for (auto& identifierWithValue : m_calculationResult) {
        const std::string& identifier = identifierWithValue.first;
        const double value = identifierWithValue.second;

        storeObject->append(prefix + "_" + identifier, value);
      }
    }

    const SoftwareTriggerObject& SoftwareTriggerCalculation::fillInCalculations()
    {
      const unsigned int sizeBeforeCheck = m_calculationResult.size();
      doCalculation(m_calculationResult);

      if (m_calculationResult.size() != sizeBeforeCheck and sizeBeforeCheck > 0) {
        B2WARNING("Calculator added more variables (" << m_calculationResult.size() <<
                  ") than there were before (" << sizeBeforeCheck << "). Probably something strange is going on!");
      }

      return m_calculationResult;
    }
  }
}