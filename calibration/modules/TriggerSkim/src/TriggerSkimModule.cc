
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/modules/TriggerSkim/TriggerSkimModule.h>
#include <hlt/softwaretrigger/core/utilities.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/utilities/Utils.h>

#include <algorithm>
#include <stdexcept>

using namespace Belle2;

REG_MODULE(TriggerSkim)

TriggerSkimModule::TriggerSkimModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Trigger Skim Module

Module will return an overall result (True/False) based on whether or not ANY or
ALL (depending on ``logicMode``) of the specified triggers returned the given
value (``expectedResult``) from the SoftwareTriggerResult.

Optionally it can apply prescales on each trigger line. In that case ``logicMode='and'``
might not be very meaningful as the return value will only be true if all trigger
lines were accepted after their own prescale.
)DOC");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("triggerLines", m_triggerLines,
           "Trigger lines to skim on. This can either be just a list of names or "
           "each item can also optionally be a tuple with name and prescale factor. "
           "So either ``['hadron', 'cosmic']`` or ``[('hadron', 1), ('cosmic', 20)]`` "
           "or a mix of both");
  addParam("expectedResult", m_expectedResult,
           "The SoftwareTriggerResult value that each trigger line in the ``triggerLines`` "
           "parameter will be tested for.", int(1));
  addParam("logicMode", m_logicMode,
           "Each trigger line in ``triggerLines`` will be tested against the ``expectedResult`` parameter. "
           "The logic mode controls whether we test using 'and' or 'or' logic to get a return value. "
           "'and' means that all trigger lines must have ``results == expectedResult``, 'or' means that only "
           "one of the trigger lines must match the expectedResult value.", m_logicMode);
  addParam("useRandomNumbersForPreScale", m_useRandomNumbersForPreScale, "Flag to use random numbers (True) "
           "or a counter (False) for applying the prescale. In the latter case, the module will retain exactly "
           "one event every N processed, where N (the counter value) is set for each line via the "
           "``triggerLines`` option. By default, random numbers are used.", m_useRandomNumbersForPreScale);
  addParam("resultOnMissing", m_resultOnMissing, "Value to return if hlt trigger result or a particular line is not available. "
           "If this is set to None a FATAL error will be raised if the results are missing. Otherwise the value "
           "given will be set as return value of the module", m_resultOnMissing);
}

void TriggerSkimModule::initialize()
{
  if(!m_resultOnMissing) {
    m_trigResults.isRequired();
  } else {
    m_trigResults.isOptional();
  }
  if (m_logicMode != "and" and m_logicMode != "or") {
    B2FATAL("You have entered an invalid parameter for logicMode. "
            "Valid strings are any of ['or', 'and']");
  }
  if(m_expectedResult < -1 or m_expectedResult > 1) {
    B2FATAL("Expected trigger result needs to be -1, 0 or 1");
  }

  // check if and how many prescales we have
  int havePrescales{0};
  for(const auto& triggerLine: m_triggerLines){
    boost::apply_visitor(Utils::VisitOverload{
      [&havePrescales](const std::tuple<std::string, unsigned int>& line) {
        const auto& [name, prescale] = line;
        if(prescale < 1) {
          B2ERROR("Prescale is not positive, needs to be at least 1"
                  << LogVar("trigger line", name) << LogVar("prescale", prescale));
        }
        ++havePrescales;
        B2DEBUG(20, "Prescale of " << prescale << " to be applied to " << name);
      },
      [](const std::string& name) {
        // nothing to do without prescales
        B2DEBUG(20, "No prescales to be applied to " << name);
      },
    }, triggerLine);
  }
  // and reserve counters if we need them
  if(havePrescales > 0 and not m_useRandomNumbersForPreScale) {
    m_prescaleCounters.clear();
    m_prescaleCounters.assign(havePrescales, 0);
  }
}

bool TriggerSkimModule::checkTrigger(const std::string& name, unsigned int prescale, uint32_t* counter) const {
  try {
    bool accepted = m_trigResults->getResult(name) == static_cast<SoftwareTriggerCutResult>(m_expectedResult);
    if(accepted and prescale != 1) {
      accepted &= SoftwareTrigger::makePreScale(prescale, counter);
    }
    return accepted;
  } catch(std::out_of_range &e) {
    // typo? change in lines? In any case we nope out of here but let's try to give a helpful message
    std::string available_lines = "";
    for(auto&& [line, result]: m_trigResults->getResults()) {
      available_lines += line + "(" + std::to_string((int)result) + ") ";
    }
    if(m_resultOnMissing) {
      B2WARNING("software trigger line not found" << LogVar("requested", name) << LogVar("available", available_lines)
                                                  << LogVar("errorFlag", m_eventMetaDataPtr->getErrorFlag())
                                                  << LogVar("experiment", m_eventMetaDataPtr->getExperiment())
                                                  << LogVar("run", m_eventMetaDataPtr->getRun())
                                                  << LogVar("event", m_eventMetaDataPtr->getEvent()));
      return *m_resultOnMissing;
    }
    else {
      B2FATAL("software trigger line not found" << LogVar("requested", name) << LogVar("available", available_lines)
                                                << LogVar("errorFlag", m_eventMetaDataPtr->getErrorFlag())
                                                << LogVar("experiment", m_eventMetaDataPtr->getExperiment())
                                                << LogVar("run", m_eventMetaDataPtr->getRun())
                                                << LogVar("event", m_eventMetaDataPtr->getEvent())
                                                << "(if this is expected,please use the `resultOnMissing` parameter)");
      return false;
    }
  }
}

void TriggerSkimModule::event()
{
  if(!m_trigResults) {
    if(m_resultOnMissing) {
      setReturnValue(*m_resultOnMissing);
      return;
    }
    B2FATAL("No HLT Trigger result available (if this is expected to be possible "
            "please use the `resultOnMissing` parameter");
  }
  // count accepted triggers
  size_t numAccepted(0);
  // Now check all lines, index is the prescale counter if needed
  size_t counterIndex{0};
  for(const auto& triggerLine: m_triggerLines) {
    const bool accept = boost::apply_visitor(Utils::VisitOverload{
      // check in prescale case
      [this, &counterIndex](const std::tuple<std::string, unsigned int>& line) {
        const auto& [name, prescale] = line;
        uint32_t* prescaleCounter = m_useRandomNumbersForPreScale ? nullptr : &m_prescaleCounters[counterIndex++];
        return checkTrigger(name, prescale, prescaleCounter);
      },
      // check without prescale
      [this](const std::string& name) {
        return checkTrigger(name);
      },
    }, triggerLine);
    if(accept) 
      ++numAccepted;
  }

  // Apply our logic on the test results
  if (m_logicMode == "and") {
    setReturnValue(numAccepted == m_triggerLines.size());
  }
  if (m_logicMode == "or") {
    setReturnValue(numAccepted > 0);
  }
}
