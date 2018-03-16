/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/modules/core/IoVDependentConditionModule.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;

REG_MODULE(IoVDependentCondition)

IoVDependentConditionModule::IoVDependentConditionModule() : Module()
{
  setDescription("Module which sets its return value based on the fact, if the event is in the given "
                 "run/exp interval or not. If you set the maximal value of experiment and run to -1, "
                 "there will be no upper limit for the interval. If you only set the maximal run to -1, "
                 "there is no upper limit on the run number.");
  setPropertyFlags(Module::EModulePropFlags::c_ParallelProcessingCertified);

  addParam("iovList", m_iovList, "The list of IoV to test in the format [(min exp, min run, max exp, max run), ...]."
           "If multiple IoVs are given, their union will be checked against the event's IoV.");
}

void IoVDependentConditionModule::initialize()
{
  m_eventMetaData.isRequired();

  B2ASSERT("You did not specify any IoVs to test!", not m_iovList.empty());
  for (const auto& iovAsTuple : m_iovList) {
    double minimalExpNumber, minimalRunNumber, maximalExpNumber, maximalRunNumber;
    std::tie(minimalExpNumber, minimalRunNumber, maximalExpNumber, maximalRunNumber) = iovAsTuple;
    const IntervalOfValidity iovToCheck(minimalExpNumber, minimalRunNumber, maximalExpNumber, maximalRunNumber);

    B2ASSERT("One of the specified interval of exp/run is empty. This is probably not what you want!",
             not iovToCheck.empty());

    m_iovsToCheck.push_back(iovToCheck);
  }

}

void IoVDependentConditionModule::beginRun()
{
  m_conditionIsMet = false;
  for (const IntervalOfValidity& iov : m_iovsToCheck) {
    if (iov.contains(*m_eventMetaData)) {
      m_conditionIsMet = true;
      break;
    }
  }
}

void IoVDependentConditionModule::event()
{
  setReturnValue(m_conditionIsMet);
}
