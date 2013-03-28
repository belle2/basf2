/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/VXDTFHelperTools/EventCounterModule.h"
// #include <framework/dataobjects/EventMetaData.h>
// #include <framework/datastore/StoreArray.h>
// #include <framework/gearbox/GearDir.h> // braucht man zum Auslesen von xml-files
/*
#include <TVector3.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <set>*/


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventCounter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventCounterModule::EventCounterModule() : Module()
{
  //Set module properties
  setDescription("simply highlights current event");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("stepSize", m_stepSize, "e.g. 100 will highlight every 100th event", int(100));

}


EventCounterModule::~EventCounterModule()
{

}


void EventCounterModule::initialize()
{


}


void EventCounterModule::beginRun()
{
  m_eventCounter = 0;
  B2INFO("################## eventCounter enabled, highlighting every " << m_stepSize << " event ######################");
}


void EventCounterModule::event()
{
  m_eventCounter++;
  if (m_eventCounter % m_stepSize == 0) {
    B2INFO("EventCounterModule - Event: " << m_eventCounter);
  }
}


void EventCounterModule::endRun()
{
}


void EventCounterModule::terminate()
{

}

