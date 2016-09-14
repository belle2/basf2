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

#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <TObject.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Enumeration with all possible results of the SoftwareTriggerCut.
    enum class SoftwareTriggerCutResult {
      c_reject, /**< Reject this event. */
      c_accept, /**< Accept this event. */
      c_noResult /**< This cut did not give any information on what to do with the event. */
    };

    /// Make the SoftwareTriggerCutResult printable
    //std::ostream &operator<<(std::ostream &os, SoftwareTriggerCutResult const &result);

    /**
     * Dataobject to store the results of the cut calculations
     * performed by the SoftwareTriggerModule. This is basically
     * a map storing
     *  cut/trigger identifier -> result
     * where cut/trigger identifier is the identifier of the checked cut
     * and result is a bool variable with the result of the decision.
     * and result is a bool variable with the result of the decision.
     */
    class SoftwareTriggerResult : public TObject {
    public:
      /// Add a new cut result to the storage or override the result with the same name.
      void addResult(const std::string& triggerIdentifier, const SoftwareTriggerCutResult& result)
      {
        m_results[triggerIdentifier] = result;
      }

      /// Return the cut result with the given name or throw an error if no result is there.
      const SoftwareTriggerCutResult& getResult(const std::string& triggerIdentifier) const
      {
        return m_results.at(triggerIdentifier);
      }

      /**
       * Return the "total result" of this event. See the SoftwareTriggerModule for a description on
       * when what is returned.
       */
      int getTotalResult(bool acceptOverridesReject = false) const
      {
        bool hasOneAcceptCut = false;
        bool hasOneRejectCut = false;

        for (const auto& identifierWithResult : m_results) {
          const auto& result = identifierWithResult.second;
          if (result == SoftwareTriggerCutResult::c_accept) {
            hasOneAcceptCut = true;
          } else if (result == SoftwareTriggerCutResult::c_reject) {
            hasOneRejectCut = true;
          }
        }

        if (acceptOverridesReject) {
          if (hasOneAcceptCut) {
            return 1;
          } else if (hasOneRejectCut) {
            return -1;
          } else {
            return 0;
          }
        } else {
          if (hasOneRejectCut) {
            return -1;
          } else if (hasOneAcceptCut) {
            return 1;
          } else {
            return 0;
          }
        }
      }

      /// Clear all results
      void clear()
      {
        m_results.clear();
      }

    private:
      /// Internal storage of the cut decisions with names.
      std::map<std::string, SoftwareTriggerCutResult> m_results;

      /** Making this class a ROOT class.*/
      ClassDef(SoftwareTriggerResult, 2);
    };
  }
}
