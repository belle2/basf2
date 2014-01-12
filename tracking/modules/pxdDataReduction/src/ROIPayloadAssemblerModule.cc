/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/modules/pxdDataReduction/ROIPayloadAssemblerModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/ROIid.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/ROIpayload.h>
#include <stdlib.h>
#include <set>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ROIPayloadAssembler)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROIPayloadAssemblerModule::ROIPayloadAssemblerModule() : Module()
{
  //Set module properties
  setDescription("This module assembles payload for the ROI in the correct format to be sent to the ONSEN");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));

}

ROIPayloadAssemblerModule::~ROIPayloadAssemblerModule()
{
}


void ROIPayloadAssemblerModule::initialize()
{

  StoreArray<ROIid>::required(m_ROIListName);
  StoreObjPtr<EventMetaData>::required();
  StoreObjPtr<ROIpayload>::registerPersistent(m_ROIpayloadName);

}

void ROIPayloadAssemblerModule::beginRun()
{

}


void ROIPayloadAssemblerModule::event()
{

  StoreArray<ROIid> ROIList(m_ROIListName);

  int ROIListSize = ROIList.getEntries();

  set<ROIrawID, ROIrawID> orderedROIraw;
  set<ROIrawID, ROIrawID>::iterator itOrderedROIraw;

  B2DEBUG(1, " number of ROIs in the list = " << ROIListSize);

  for (int iROI = 0; iROI < ROIListSize; iROI++) {

    int layer = (ROIList[iROI]->getSensorID()).getLayerNumber() - 1;
    int ladder = (ROIList[iROI]->getSensorID()).getLadderNumber();
    int sensor = (ROIList[iROI]->getSensorID()).getSensorNumber() - 1;

    m_roiraw.setSystemFlag(0);
    m_roiraw.setDHHID(((layer) << 5) | ((ladder) << 1) | (sensor));
    m_roiraw.setRowMin(ROIList[iROI]->getMinVid());
    m_roiraw.setRowMax(ROIList[iROI]->getMaxVid());
    m_roiraw.setColMin(ROIList[iROI]->getMinUid());
    m_roiraw.setColMax(ROIList[iROI]->getMaxUid());

    orderedROIraw.insert(m_roiraw);

  }

  B2DEBUG(1, " number of ROIs in the set = " << orderedROIraw.size());

  // The payload is created with a buffer long enough to contains all
  // the ROIs but the actual payload will contains max 32 ROIs per pxd
  // sensor per event as required by the Onsens specifications
  ROIpayload* payload = new ROIpayload(4 + 2 * ROIListSize);

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);

  payloadPtr.assign(payload);

  payload->setHeader();

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  payload->setTriggerNumber(eventMetaDataPtr->getEvent());

  int tmpDHHID = -1;
  int countROIs = 0;
  int addROI = 0;

  for (itOrderedROIraw = orderedROIraw.begin(); itOrderedROIraw != orderedROIraw.end(); ++itOrderedROIraw) {

    if ((int) itOrderedROIraw->getDHHID() == tmpDHHID)
      countROIs ++;
    else
      countROIs = 0;

    if (countROIs < 32) {
      //      payload->addROIraw(*itOrderedROIraw);
      payload->addROIraw(itOrderedROIraw->getBigEndian());
      addROI++;
    } else
      B2ERROR("A ROI on DHHID " << itOrderedROIraw->getDHHID() << endl <<
              " is rejected because the max number of ROIs per pxd sensor per event (32) was exceeded.");

    tmpDHHID = itOrderedROIraw->getDHHID();

  }

  for (itOrderedROIraw = orderedROIraw.begin(); itOrderedROIraw != orderedROIraw.end(); ++itOrderedROIraw)
    B2DEBUG(1, "ordered DHHID: " << itOrderedROIraw->getDHHID());

  payload->setPayloadLength();
  payload->setCRC();

  B2DEBUG(1, " number of ROIs in payload = " << addROI);
}


void ROIPayloadAssemblerModule::endRun()
{
}


void ROIPayloadAssemblerModule::terminate()
{
}

