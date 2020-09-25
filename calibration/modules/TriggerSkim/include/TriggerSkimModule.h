/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/Module.h>

namespace Belle2 {
  /// Returns the calibration result from SoftwareTriigerResult for skimming out calibration flagged events
  class TriggerSkimModule : public Module {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    TriggerSkimModule();
    /// Initialize
    virtual void initialize() override;
    /// Event function
    virtual void event() override;

  private:
    /** Required input for trigger results */
    StoreObjPtr<SoftwareTriggerResult> m_trigResults;
    /** List of triggerlines we're interested in */
    std::vector<std::string> m_triggerLines;
    /** Result we want for each or any trigger line */
    int m_expectedResult{1};
    /** do we want each or any trigger line? */
    std::string m_logicMode{"or"};
    /** do we want to prescale each trigger line ? */
    std::vector<unsigned int> m_prescales;
    /** and do we want random prescale or counters ? */
    bool m_useRandomNumbersForPReScale{true};
    /** if we don't use random prescale we need counters */
    std::vector<uint32_t> m_prescaleCounters;
  };
}
