/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/core/EventErrorFlagModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventErrorFlag)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventErrorFlagModule::EventErrorFlagModule() : Module()
{
  setDescription("Returns error flags of the EventMetaData and can add further error flags.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("errorFlag", m_ErrorFlag, "Error flags to add", 0);
}

void EventErrorFlagModule::initialize()
{
  m_eventMetaData.isRequired();
}

void EventErrorFlagModule::event()
{
  setReturnValue(int(m_eventMetaData->getErrorFlag()));
  m_eventMetaData->addErrorFlag(EventMetaData::EventErrorFlag(m_ErrorFlag));
}
