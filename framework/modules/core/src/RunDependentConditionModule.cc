/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/modules/core/RunDependentConditionModule.h>

using namespace Belle2;

REG_MODULE(RunDependentCondition)

RunDependentConditionModule::RunDependentConditionModule() : Module()
{
  setDescription("Module which sets its return value based on the fact, if the event is in the given run or not.");
  setPropertyFlags(Module::EModulePropFlags::c_ParallelProcessingCertified);

  addParam("trueOnRun", m_trueOnRun, "Returns true if in this run - or false otherwise.");
}


void RunDependentConditionModule::initialize()
{
  m_eventMetaData.isRequired();
}

void RunDependentConditionModule::beginRun()
{
  m_runConditionMet = m_eventMetaData->getRun() == m_trueOnRun;
}


void RunDependentConditionModule::event()
{
  setReturnValue(m_runConditionMet);
}