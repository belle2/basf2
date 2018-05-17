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

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <TObject.h>

namespace Belle2 {
  /// Enumeration with all possible results of the SoftwareTriggerCut.
  enum class SoftwareTriggerCutResult {
    c_reject = -1, /**< Reject this event. */
    c_accept = 1, /**< Accept this event. */
    c_noResult = 0 /**< There were not enough information to decide on what to do with the event. */
  };

  /**
   * Dataobject to store the results of the cut calculations
   * performed by the SoftwareTriggerModule. This is basically
   * a map storing
   *  cut/trigger identifier -> result
   * where cut/trigger identifier is the identifier of the checked cut
   * and result is a bool variable with the result of the decision.
   * and result is a bool variable with the result of the decision.
   */
  class SoftwareTriggerResult : public RelationsObject {
  public:
    /// Add a new cut result to the storage or override the result with the same name.
    void addResult(const std::string& triggerIdentifier, const SoftwareTriggerCutResult& result);

    /// Return the cut result with the given name or throw an error if no result is there.
    SoftwareTriggerCutResult getResult(const std::string& triggerIdentifier) const;

    /**
     * Return all stored cut tags with their results as a map identifier -> cut result.
     * Please be aware that the cut result is an integer (because of ROOT reasons).
     */
    const std::map<std::string, int>& getResults() const
    {
      return m_results;
    };

    /// Clear all results
    void clear();

    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const override;

  private:
    /// Internal storage of the cut decisions with names.
    std::map<std::string, int> m_results;

    /** Making this class a ROOT class.*/
    ClassDefOverride(SoftwareTriggerResult, 4);
  };
}
