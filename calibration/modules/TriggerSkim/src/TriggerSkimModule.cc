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

using namespace Belle2;
using namespace std;

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
  vector<string> emptyvector;
  addParam("triggerLines", m_triggerLines,
           "List of trigger lines to skim on. Module will return an overall result (True/False) based on "
           "whether or not ANY of the specified triggers returned a value of 1 from the SoftwareTriggerResult. "
           "A SoftwareTriggerResult of either 0 or -1 doesn't count.",
           emptyvector);
  addParam("expectedResult", m_expectedResult,
           "The SoftwareTriggerResult value that each trigger line in the 'triggerLines' param will be tested for.", int(1));
  addParam("logicMode", m_logicMode,
           "Each trigger line in 'triggerLines' will be tested against the 'expectedResult' parameter. "
           "The logic mode controls whether we test using 'and' or 'or' logic to get a return value. "
           "'and' means that all trigger lines must have results == expectedResult, 'or' means that only "
           "one of the trigger lines must match the expectedResult value.",
           string("or"));
}

void TriggerSkimModule::initialize()
{
  m_trigResults.isRequired();
  if (m_logicMode != "and"
      && m_logicMode != "or") B2FATAL("You have entered an invalid parameter for logicMode. "
                                        "Valid strings are any of ['or', 'and']");
}

void TriggerSkimModule::event()
{
  // Get our results
  const map<string, int> results = m_trigResults->getResults();
  // Apply our logic on the test results
  if (m_logicMode == "and") {
    setReturnValue(all_of(m_triggerLines.begin(), m_triggerLines.end(),
                          [&results, this](string trigger) -> bool {return (results.find(trigger)->second == m_expectedResult);}));
  }
  if (m_logicMode == "or") {
    setReturnValue(any_of(m_triggerLines.begin(), m_triggerLines.end(),
                          [&results, this](string trigger) -> bool {return (results.find(trigger)->second == m_expectedResult);}));
  }
}
