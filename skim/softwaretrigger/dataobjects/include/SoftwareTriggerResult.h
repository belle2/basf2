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

#include <map>
#include <string>
#include <TObject.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    class SoftwareTriggerResult : public TObject {
    public:
      void addResult(const std::string& triggerIdentifier, const bool& result)
      {
        m_results[triggerIdentifier] = result;
      }

      bool getResult(const std::string& triggerIdentifier) const
      {
        return m_results.at(triggerIdentifier);
      }

      bool getTotalResult() const
      {
        for (const auto& identifierWithResult : m_results) {
          const auto& result = identifierWithResult.second;
          if (result) {
            return true;
          }
        }

        return false;
      }

    private:
      std::map<std::string, bool> m_results;

      /** Making this class a ROOT class.*/
      ClassDef(SoftwareTriggerResult, 1);
    };
  }
}
