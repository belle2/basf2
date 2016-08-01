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
    /**
     * Dataobject to store the results of the cut calculations
     * performed by the SoftwareTriggerModule. This is basically
     * a map storing
     *  cut/trigger identifier -> result
     * where cut/trigger identifier is the identifier of the checked cut
     * and result is a bool variable with the result of the decision.
     */
    class SoftwareTriggerResult : public TObject {
    public:
      /// Add a new cut result to the storage or override the result with the same name.
      void addResult(const std::string& triggerIdentifier, const bool& result)
      {
        m_results[triggerIdentifier] = result;
      }

      /// Return the cut result with the given name or throw an error if no result is there.
      bool getResult(const std::string& triggerIdentifier) const
      {
        return m_results.at(triggerIdentifier);
      }

      /**
       * Return the "total result" of this event. The result of an event
       * is true if and only if at least on of the stored trigger decision
       * is true. This means that the event has to "survive" at least one cut
       * to be called "survived".
       */
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
      /// Internal storage of the cut decisions with names.
      std::map<std::string, bool> m_results;

      /** Making this class a ROOT class.*/
      ClassDef(SoftwareTriggerResult, 1);
    };
  }
}
