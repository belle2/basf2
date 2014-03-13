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

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

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
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));
  addParam("TrigDivider", m_divider, "Generates one ROI every TrigDivider events", 2);
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

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  StoreArray<ROIid> ROIList(m_ROIListName);

  int ROIListSize = ROIList.getEntries();

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int trgNum = eventMetaDataPtr->getEvent(); // trigger number

  if (trgNum % m_divider != 0)
    for (int iROI = 0; iROI < ROIListSize; iROI++) {

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(ROIList[iROI]->getSensorID());
      const int nPixelsU = aSensorInfo.getUCells() - 1;
      const int nPixelsV = aSensorInfo.getVCells() - 1;

      ROIid tmpROIid;

      tmpROIid.setSensorID(ROIList[iROI]->getSensorID());
      unsigned int tmpRowMin = nPixelsU - ROIList[iROI]->getMinUid();
      unsigned int tmpRowMax =  nPixelsU - ROIList[iROI]->getMaxUid();
      unsigned int tmpColMin =  nPixelsV - ROIList[iROI]->getMinVid();
      unsigned int tmpColMax = nPixelsV -  ROIList[iROI]->getMaxVid();

      tmpROIid.setMinUid(std::min(tmpRowMin, tmpRowMax));
      tmpROIid.setMaxUid(std::max(tmpRowMin, tmpRowMax));
      tmpROIid.setMinVid(std::min(tmpColMin, tmpColMax));
      tmpROIid.setMaxVid(std::max(tmpColMin, tmpColMax));

      ROIList.appendNew(tmpROIid);
    }


  ROIListSize = ROIList.getEntries();

  set<ROIrawID, ROIrawID> orderedROIraw;
  set<ROIrawID, ROIrawID>::iterator itOrderedROIraw;

  B2DEBUG(1, " number of ROIs in the list = " << ROIListSize);

  for (int iROI = 0; iROI < ROIListSize; iROI++) {

    int layer = (ROIList[iROI]->getSensorID()).getLayerNumber() - 1;
    int ladder = (ROIList[iROI]->getSensorID()).getLadderNumber();
    int sensor = (ROIList[iROI]->getSensorID()).getSensorNumber() - 1;

    m_roiraw.setSystemFlag(0);
    m_roiraw.setDHHID(((layer) << 5) | ((ladder) << 1) | (sensor));

    // ToDo!!! Code elsewhere the translation for u v row and column
    unsigned int tmpRowMin = ROIList[iROI]->getMinUid();
    unsigned int tmpRowMax = ROIList[iROI]->getMaxUid();

    unsigned int tmpColMin = ROIList[iROI]->getMinVid();
    unsigned int tmpColMax = ROIList[iROI]->getMaxVid();

    unsigned int row1 = std::min(tmpRowMin, tmpRowMax);
    unsigned int row2 = std::max(tmpRowMin, tmpRowMax);

    unsigned int column1 = std::min(tmpColMin, tmpColMax);
    unsigned int column2 = std::max(tmpColMin, tmpColMax);

    m_roiraw.setRowMin(row1);
    m_roiraw.setRowMax(row2);
    m_roiraw.setColMin(column1);
    m_roiraw.setColMax(column2);

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

  //  StoreObjPtr<EventMetaData> eventMetaDataPtr;
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

