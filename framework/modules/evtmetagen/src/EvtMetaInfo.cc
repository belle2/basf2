/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../evtmetagen/EvtMetaInfo.h"

#include <fwcore/ModuleManager.h>
#include <datastore/StoreObjPtr.h>
#include <datastore/StoreDefs.h>
#include <datastore/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtMetaInfo)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtMetaInfo::EvtMetaInfo() : Module("EvtMetaInfo")
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, evt).");

  //Parameter definition
}


EvtMetaInfo::~EvtMetaInfo()
{

}


void EvtMetaInfo::initialize()
{

}


void EvtMetaInfo::beginRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
  INFO_S("========================================================================");
  INFO_S(">>> Start new run: " << eventMetaDataPtr->getRun());
  INFO_S("------------------------------------------------------------------------");
}


void EvtMetaInfo::event()
{
  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
  INFO("EXP NUMBER: " << eventMetaDataPtr->getExperiment());
  INFO("RUN NUMBER: " << eventMetaDataPtr->getRun());
  INFO("EVT NUMBER: " << eventMetaDataPtr->getEvent());
}


void EvtMetaInfo::endRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", c_Persistent);
  INFO_S("------------------------------------------------------------------------");
  INFO_S("<<< End run: " << eventMetaDataPtr->getRun());
  INFO_S("========================================================================");
}


void EvtMetaInfo::terminate()
{

}
