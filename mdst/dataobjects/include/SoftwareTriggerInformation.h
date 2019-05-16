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

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/logging/Logger.h>

#include <map>
#include <string>
#include <TObject.h>

namespace Belle2 {
  /**
   * Dataobject to store additional information for the software trigger result,
   * such as used prescaling or the results without prescaling.
   * Can be used in the analysis later.
   *
   * Attention: if you are interested in the decision of the software trigger,
   * please use the SoftwareTriggerResult in the data store!
   * This object just gives additional information - the decision of
   * an event is kept or not is not done here.
   */
  class SoftwareTriggerInformation : public TObject {
  public:
    /// Add a new non-prescaled cut result to the storage or override the result with the same name.
    void addNonPrescaledResult(const std::string& triggerIdentifier, const SoftwareTriggerCutResult& result);

    /// Add a used prescaling to the storage or override the result with the same name.
    void addPrescaling(const std::string& triggerIdentifier, const std::vector<unsigned int>& prescaling);

    /// Return the non-prescaled cut result with the given name or throw an error if no result is there.
    SoftwareTriggerCutResult getNonPrescaledResult(const std::string& triggerIdentifier) const;

    /**
     * Return the used prescaling for the cut with the given name or throw an error if no prescaling is stored.
     *
     * Attention: this is the prescaling used while doing the cut result stored in the SoftwareTriggerResult -
     * it might be different from the one that is written into the database!
     */
    const std::vector<unsigned int>& getPrescaling(const std::string& triggerIdentifier) const;

    /**
     * Return all stored non-prescaled cut tags with their results as a map identifier -> cut result.
     * Please be aware that the cut result is an integer (because of ROOT reasons).
     */
    const std::map<std::string, int>& getNonPrescaledResults() const
    {
      return m_nonPrescaledResults;
    };

    /**
     * Return the used prescalings for all cuts.
     *
     * Attention: these are the prescalings used while doing the cut result stored in the SoftwareTriggerResult -
     * they might be different from the ones that are written into the database!
     */
    const std::map<std::string, std::vector<unsigned int>>& getPrescalings() const
    {
      return m_prescalings;
    };

    /// Clear all results
    void clear();

  private:
    /// Internal storage of the non-prescaled cut decisions with names.
    std::map<std::string, int> m_nonPrescaledResults;

    /// Internal storage of the used prescalings
    std::map<std::string, std::vector<unsigned int>> m_prescalings;

    /** Making this class a ROOT class.*/
    ClassDefOverride(SoftwareTriggerInformation, 1);
  };
}
