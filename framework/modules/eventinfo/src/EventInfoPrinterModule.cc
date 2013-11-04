/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/modules/eventinfo/EventInfoPrinterModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventInfoPrinter)
REG_MODULE(EvtMetaInfo)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventInfoPrinterModule::EventInfoPrinterModule() : Module()
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, event numbers).");
}


EventInfoPrinterModule::~EventInfoPrinterModule()
{

}

void EventInfoPrinterModule::initialize()
{
  StoreObjPtr<EventMetaData>::required();
}

void EventInfoPrinterModule::beginRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2INFO("========================================================================");
  B2INFO(">>> Start new run: " << eventMetaDataPtr->getRun());
  B2INFO("------------------------------------------------------------------------");
}


void EventInfoPrinterModule::event()
{
  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (!eventMetaDataPtr) return;
  B2INFO(boost::format("EXP: %8d        RUN: %8d        EVT: %8d") % eventMetaDataPtr->getExperiment() % eventMetaDataPtr->getRun() % eventMetaDataPtr->getEvent()) ;
}


void EventInfoPrinterModule::endRun()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2INFO("------------------------------------------------------------------------");
  B2INFO("<<< End run: " << eventMetaDataPtr->getRun());
  B2INFO("========================================================================");
}
