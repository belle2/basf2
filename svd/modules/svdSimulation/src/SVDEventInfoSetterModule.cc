/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDEventInfoSetterModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDEventInfoSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDEventInfoSetterModule::SVDEventInfoSetterModule() : Module()
{
  //Set module properties
  setDescription(
    "Sets the SVD event information. You must use this "
    "module to fill SVDEventInfo object which tells the digitizer "
    "the conditions for creating ShaperDigits."
  );

  //Parameter definition for SVDModeByte, TriggerType and cross-talk
  addParam("runType", m_runType, "Defines the run type: raw/transparent/zero-suppressed/z-s+hit time finding", int(2));
  addParam("eventType", m_eventType, "Defines the event type: TTD event (global run)/standalone event (local run)", int(0));
  addParam("daqMode", m_daqMode, "Defines the DAQ mode: 1/3/6 samples", int(2));
  addParam("triggerBin", m_triggerBin, "Trigger time info: 0/1/2/3 - useful to synchronise the SVD with the machine", int(0));
  addParam("triggerType", m_triggerType, "Defines the trigger type, default: CDC trigger", uint8_t(3));
  addParam("crossTalk", m_xTalk, "Defines the cross-talk flag for the event", bool(false));

  // default ModeByte settings: 10 0 10 000 (144)
}

SVDEventInfoSetterModule::~SVDEventInfoSetterModule() = default;

void SVDEventInfoSetterModule::initialize()
{
  //Register the EventInfo in the data store
  m_svdEventInfoPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

// TO BE ADDED(?): some functions than can check the validity of the given parameters
}

void SVDEventInfoSetterModule::event()
{
  m_SVDModeByte.setRunType(m_runType);
  m_SVDModeByte.setEventType(m_eventType);
  m_SVDModeByte.setDAQMode(m_daqMode);
  m_SVDModeByte.setTriggerBin(m_triggerBin);
  m_SVDTriggerType.setType(m_triggerType);

  m_svdEventInfoPtr.create();
  m_svdEventInfoPtr->setModeByte(m_SVDModeByte);
  m_svdEventInfoPtr->setMatchModeByte(m_ModeByteMatch);
  m_svdEventInfoPtr->setTriggerType(m_SVDTriggerType);
  m_svdEventInfoPtr->setMatchTriggerType(m_TriggerTypeMatch);
  m_svdEventInfoPtr->setCrossTalk(m_xTalk);
}




