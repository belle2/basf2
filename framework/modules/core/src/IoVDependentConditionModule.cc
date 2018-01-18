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

using namespace Belle2;

REG_MODULE(IoVDependentCondition)

IoVDependentConditionModule::IoVDependentConditionModule() : Module()
{
  setDescription("Module which sets its return value based on the fact, if the event is in the given run/exp interval or not.");
  setPropertyFlags(Module::EModulePropFlags::c_ParallelProcessingCertified);

  addParam("minimalExpNumber", m_minimalExpNumber, "Minimal exp number to return true.", m_minimalExpNumber);
  addParam("minimalRunNumber", m_minimalRunNumber, "Minimal run number to return true.", m_minimalRunNumber);
  addParam("maximalExpNumber", m_maximalExpNumber, "Maximal exp number to return true.", m_maximalExpNumber);
  addParam("maximalRunNumber", m_maximalRunNumber, "Maximal run number to return true.", m_maximalRunNumber);
}


void IoVDependentConditionModule::initialize()
{
  m_eventMetaData.isRequired();

  // Copy the input parameters
  m_iovToCheck = IntervalOfValidity(m_minimalExpNumber, m_minimalRunNumber, m_maximalExpNumber, m_maximalRunNumber);
}

void IoVDependentConditionModule::beginRun()
{
  m_conditionIsMet = m_iovToCheck.contains(*m_eventMetaData);
}

void IoVDependentConditionModule::event()
{
  setReturnValue(m_conditionIsMet);
}