/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <iostream>

#include <calibration/modules/TriggerSkim/TriggerSkimModule.h>
#include <map>
#include <algorithm>
#include <softwaretrigger/core/utilities.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TriggerSkim)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TriggerSkimModule::TriggerSkimModule() : Module()
{
  // Set module properties
  setDescription("Trigger Skim Module");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("triggerLines", m_triggerLines,
           "List of trigger lines to skim on. Module will return an overall result (True/False) based on "
           "whether or not ANY of the specified triggers returned a value of 1 from the SoftwareTriggerResult. "
           "A SoftwareTriggerResult of either 0 or -1 doesn't count.");
  addParam("expectedResult", m_expectedResult,
           "The SoftwareTriggerResult value that each trigger line in the 'triggerLines' param will be tested for.", int(1));
  addParam("logicMode", m_logicMode,
           "Each trigger line in 'triggerLines' will be tested against the 'expectedResult' parameter. "
           "The logic mode controls whether we test using 'and' or 'or' logic to get a return value. "
           "'and' means that all trigger lines must have results == expectedResult, 'or' means that only "
           "one of the trigger lines must match the expectedResult value.",
           string("or"));
  addParam("prescaleFactors", m_prescales,
           "List of prescale factors for the trigger lines. If given it needs to be a list of prescales for each trigger line",
           m_prescales);
  addParam("useRandomNumbersForPreScale", m_useRandomNumbersForPreScale, "Flag to use random numbers (True) "
           "or a counter (False) for applying the prescale.", m_param_useRandomNumbersForPreScale);
}

void TriggerSkimModule::initialize()
{
  m_trigResults.isRequired();
  if (m_logicMode != "and" && m_logicMode != "or") {
    B2FATAL("You have entered an invalid parameter for logicMode. "
            "Valid strings are any of ['or', 'and']");
  }
  if (not m_prescales.empty()) {
    if (m_prescales.size() != m_triggerLines.size()) {
      B2FATAL("Number of prescale factors needs to correspond to number of trigger lines");
    }
    if (*std::min_element(m_prescales.begin(), m_prescales.end()) < 1) {
      B2FATAL("All prescale factors need to be positive numbers");
    }
    if (not m_useRandomNumbersForPreScale) {
      m_prescaleCounters.clear();
      m_prescaleCounters.assign(m_prescales.size(), 0);
    }
  }
}

void TriggerSkimModule::event()
{
  // Get our results
  const std::map<string, int> results = m_trigResults->getResults();
  // and now check for each line we're interested in
  std::vector<bool> accepted(m_triggerLines.size(), 0);
  size_t index{0};
  for (const auto& trigger : triggerLines) {
    bool triggerResult = results.find(trigger)->second == m_expectedResult;
    // now if we want to prescale apply the prescale as well
    if (triggerResult and not m_prescales.empty()) {
      uint32_t* prescaleCounter = m_useRandomNumbersForPreScale ? nullptr : &m_prescaleCounters[index];
      triggerResult &= SoftwareTrigger::makePrescale(m_prescales[index], prescaleCounter);
    }
    accepted[index++] = triggerResult;
  }
  // Apply our logic on the test results
  if (m_logicMode == "and") {
    setReturnValue(std::all_of(accepted.begin(), accepted.end()));
  }
  if (m_logicMode == "or") {
    setReturnValue(std::any_of(accepted.begin(), accepted.end()));
  }
}
