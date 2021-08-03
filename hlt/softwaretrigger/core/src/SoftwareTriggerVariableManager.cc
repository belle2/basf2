/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    SoftwareTriggerVariableManager::SoftwareTriggerVariable* SoftwareTriggerVariableManager::getVariable(
      const std::string& variableName, const std::vector<std::string>& arguments)
    {
      auto fullname = variableName + "(" + boost::algorithm::join(arguments, ", ") + ")";
      return SoftwareTriggerVariableManager::getVariable(fullname);
    }
  }
}
