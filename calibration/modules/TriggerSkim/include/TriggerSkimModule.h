/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>

#include <map>
#include <optional>
#include <string>

#include <boost/variant.hpp>

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
    /** Check a single trigger line for the expected result */
    bool checkTrigger(const std::string& name, unsigned int prescale = 1, uint32_t* counter = nullptr) const;
    /** Required input for trigger results */
    StoreObjPtr<SoftwareTriggerResult> m_trigResults;
    /** List of triggerlines we're interested in. Or Optional a map of trigger lines and their prescale values */
    std::vector<boost::variant<std::string, std::tuple<std::string, unsigned int>>> m_triggerLines;
    /** Result we want for each or any trigger line */
    int m_expectedResult{1};
    /** do we want each or any trigger line? */
    std::string m_logicMode{"or"};
    /** and do we want random prescale or counters ? */
    bool m_useRandomNumbersForPreScale{true};
    /** value to return if there's no SoftwareTriggerResult */
    std::optional<int> m_resultOnMissing;
    /** if we don't use random prescale we need counters */
    std::vector<uint32_t> m_prescaleCounters;
    /** EventMetaData is used to report warning/error messages. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    /** Check event is the first event for hbasf2 usage, especiaaly for HLT storage. */
    bool m_firstEvent{false};
  };
}
