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
#include "tracking/modules/VXDTF/VXDTFModule.h"


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
  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::optional();

}


void EventCounterModule::beginRun()
{
  m_eventCounter = 0;
  m_pxdClusterCounter = 0;
  m_svdClusterCounter = 0;
  B2INFO("################## eventCounter enabled, highlighting every " << m_stepSize << " event ######################");
}


void EventCounterModule::event()
{
  m_eventCounter++;

  StoreArray<PXDCluster> aPxdClusterArray;
  m_pxdClusterCounter += aPxdClusterArray.getEntries();

  StoreArray<SVDCluster> aSvdClusterArray;
  m_svdClusterCounter += aSvdClusterArray.getEntries();

  if (m_eventCounter % m_stepSize == 0) {
    B2INFO("EventCounterModule - Event: " << m_eventCounter << " having " << aPxdClusterArray.getEntries() << "/" << aSvdClusterArray.getEntries() << " pxd/svdClusters");
  }
}


void EventCounterModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;
  B2INFO("EventCounterModule: after " << m_eventCounter << " events there were " << m_pxdClusterCounter << "/" << m_svdClusterCounter << " pxd/svdClusters total and " << double(m_pxdClusterCounter)*invEvents << "/" << double(m_svdClusterCounter)*invEvents << " pxd/svdClusters per event");
}


void EventCounterModule::terminate()
{

}

