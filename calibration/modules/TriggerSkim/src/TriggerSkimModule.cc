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
           "List of trigger lines to skim on. Module will return an overall result (True/False) based on whether or not ANY of the specified triggers returned a value of 1 from the SoftwareTriggerResult. A SoftwareTriggerResult of either 0 or -1 doesn't count.",
           emptyvector);
}

void TriggerSkimModule::initialize()
{
  m_trigResults.isRequired();
}

void TriggerSkimModule::event()
{
  const map<string, int> results = m_trigResults->getResults();
  bool overall_result = 0;
  for (const string& trigger : m_triggerLines) {
    if (results.find(trigger)->second == 1) overall_result = 1;
    if (overall_result == 1) break;
  }
  setReturnValue(overall_result);
//  cout << "Result = " << overall_result  << "\n";
}
