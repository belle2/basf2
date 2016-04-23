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

ROIPayloadAssemblerModule::ROIPayloadAssemblerModule() : Module(),
  m_triggerNumberShift(0)
{
  //Set module properties
  setDescription("This module assembles payload for the ROI in the correct format to be sent to the ONSEN");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));
  addParam("Desy2016ROIExtension", m_DESYROIExtension,
           "Does a ROI coordinate extension for Desy TB 2016, WORKAROUND for missing DHH functionality", false);
  addParam("Desy2016Remapping", m_DESYremap,
           "Does a ROI coordinate remapping for Desy TB 2016, WORKAROUND for missing DHH functionality", false);
  addParam("SendAllDownscaler", mSendAllDS,
           "Send all Data (no selection) downscaler; Workaround for missing ONSEN functionality, 0 never set, 1 alway set, 2 set in every second...",
           9u);
  addParam("SendROIsDownscaler", mSendROIsDS,
           "Send ROIs downscaler; Workaround for missing ONSEN functionality, 0 never set, 1 alway set, 2 set in every second...", 3u);
  addParam("triggerNumberShift", m_triggerNumberShift,
           "Ideally = 0. Payload trigger number = event trigger number + triggerNumberShift", 0);
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
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  set<ROIrawID, ROIrawID> orderedROIraw;
  set<ROIrawID, ROIrawID>::iterator itOrderedROIraw;

  B2DEBUG(1, " number of ROIs in the list = " << ROIList.getEntries());

  /// This is necessary as long as DHH is NOT delivering the DHP coordinates in local sensor coordinates (Ucell/Vcell)
  for (auto& iROI : ROIList) {
    const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(iROI.getSensorID());
    const int nPixelsU = aSensorInfo.getUCells() - 1;
    const int nPixelsV = aSensorInfo.getVCells() - 1;

    unsigned int tmpRowMin;
    unsigned int tmpRowMax;
    if (m_DESYremap and (iROI.getSensorID()).getLayerNumber() == 1) {
      // Inner (because of NO -1 above
      tmpRowMin = nPixelsV - iROI.getMinVid();
      tmpRowMax = nPixelsV - iROI.getMaxVid();
    } else {
      // Outer
      tmpRowMin = iROI.getMinVid();
      tmpRowMax = iROI.getMaxVid();
    }

    unsigned int tmpColMin;
    unsigned int tmpColMax;
    if (m_DESYremap and (iROI.getSensorID()).getLayerNumber() != (iROI.getSensorID()).getSensorNumber()) {
      // Outer Forward, Inner Backward
      tmpColMin = nPixelsU - iROI.getMinUid();
      tmpColMax = nPixelsU - iROI.getMaxUid();
    } else { // (layer!=sensor)
      // Inner Forward, Outer Backward
      tmpColMin = iROI.getMinUid();
      tmpColMax = iROI.getMaxUid();
    }

    int layer = (iROI.getSensorID()).getLayerNumber() - 1;
    int ladder = (iROI.getSensorID()).getLadderNumber();
    int sensor = (iROI.getSensorID()).getSensorNumber() - 1;

    m_roiraw.setSystemFlag(0);
    m_roiraw.setDHHID(((layer) << 5) | ((ladder) << 1) | (sensor));

    unsigned int row1 = std::min(tmpRowMin, tmpRowMax);
    unsigned int row2 = std::max(tmpRowMin, tmpRowMax);

    unsigned int column1 = std::min(tmpColMin, tmpColMax);
    unsigned int column2 = std::max(tmpColMin, tmpColMax);

    if (m_DESYROIExtension) DESYremap(row1, row2, column1, column2);

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
  ROIpayload* payload = new ROIpayload(orderedROIraw.size());// let the ROIpayload compute the size itself

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);

  payloadPtr.assign(payload);

  unsigned int evtNr = eventMetaDataPtr->getEvent();
  bool accepted = true; // thats the default until HLT has reject mechanism
  payload->setHeader(accepted, mSendAllDS  ? (evtNr % mSendAllDS) == 0 : 0, mSendROIsDS ? (evtNr % mSendROIsDS) == 0 : 0);

//  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  payload->setTriggerNumber(eventMetaDataPtr->getEvent() + m_triggerNumberShift);

// Set run subrun exp number
  payload->setRunSubrunExpNumber(eventMetaDataPtr->getRun(), eventMetaDataPtr->getSubrun(), eventMetaDataPtr->getExperiment());

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


void ROIPayloadAssemblerModule::DESYremap(unsigned int& row1, unsigned int& row2, unsigned int& col1, unsigned int& col2)
{
  row1 &= ~3; /// Decrease row min to readout boundary
  row2 = (row2 & ~3) + 3;/// Increase row max to readout boundary

  // IB OF and OB IF are same (luckly) at least with this coarse mapping
  {
    if (col1 <= 63) {
      col1 = 0;
    } else if (col1 <= 127) {
      col1 = 62;
    } else if (col1 <= 191) {
      col1 = 125;
    } else { // col1<=255
      col1 = 187;
    }
    if (col2 >= 192) {
      col2 = 249;
    } else if (col2 >= 128) {
      col2 = 187;
    } else if (col2 >= 64) {
      col2 = 124;
    } else { // col1>=0
      col2 = 62;
    }
  }

}
