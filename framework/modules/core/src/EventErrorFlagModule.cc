/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
