/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/core/SoftwareTriggerVariableManager.h>

#include <memory>

namespace Belle2 {
  namespace SoftwareTrigger {
    SoftwareTriggerVariableManager& SoftwareTriggerVariableManager::Instance()
    {
      static SoftwareTriggerVariableManager instance;
      return instance;
    }

    SoftwareTriggerVariableManager::SoftwareTriggerVariable* SoftwareTriggerVariableManager::getVariable(
      const std::string& variableName)
    {
      auto lowerBoundIterator = m_variableNameToVariable.lower_bound(variableName);
      if (lowerBoundIterator == m_variableNameToVariable.end() or lowerBoundIterator->first != variableName) {
        lowerBoundIterator = m_variableNameToVariable.insert(lowerBoundIterator,
        {variableName, std::shared_ptr<SoftwareTriggerVariable>(new SoftwareTriggerVariable(variableName))});
      }
      return lowerBoundIterator->second.get();
    }
  }
}
