/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/modules/evtmetagen/EvtMetaInfoModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtMetaInfoModule, "EvtMetaInfo")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtMetaInfoModule::EvtMetaInfoModule() : Module()
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, evt).");

  //Parameter definition
}


EvtMetaInfoModule::~EvtMetaInfoModule()
{

}


void EvtMetaInfoModule::beginRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Event);
  BELLE2_INFO("========================================================================");
  BELLE2_INFO(">>> Start new run: " << eventMetaDataPtr->getRun());
  BELLE2_INFO("------------------------------------------------------------------------");
}


void EvtMetaInfoModule::event()
{
  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Event);
  BELLE2_INFO("EXP NUMBER: " << eventMetaDataPtr->getExperiment());
  BELLE2_INFO("RUN NUMBER: " << eventMetaDataPtr->getRun());
  BELLE2_INFO("EVT NUMBER: " << eventMetaDataPtr->getEvent());
}


void EvtMetaInfoModule::endRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Event);
  BELLE2_INFO("------------------------------------------------------------------------");
  BELLE2_INFO("<<< End run: " << eventMetaDataPtr->getRun());
  BELLE2_INFO("========================================================================");
}
