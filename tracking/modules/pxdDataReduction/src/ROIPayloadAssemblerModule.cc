/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni, Bjoern Spruck          *
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
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
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
  addParam("SendAllDownscaler", mSendAllDS,
           "Send all Data (no selection) downscaler; Workaround for missing ONSEN functionality, 0 never set, 1 alway set, 2 set in every second...",
           9u);
  addParam("SendROIsDownscaler", mSendROIsDS,
           "Send ROIs downscaler; Workaround for missing ONSEN functionality, 0 never set, 1 alway set, 2 set in every second...", 3u);
  addParam("CutNrROIs", mCutNrROIs,
           "If Nr of ROIs per DHHID reach this, send out only one full sensor ROI", 32u);
  addParam("AcceptAll", mAcceptAll, "Accept all, Ignore HLT decision", false);
  addParam("NoRejectFlag", mNoRejectFlag, "Never reject, just send no ROI", true);
}

void ROIPayloadAssemblerModule::initialize()
{

  StoreArray<ROIid> roiIDs;
  roiIDs.isRequired(m_ROIListName);

  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.isRequired();

  StoreObjPtr<ROIpayload> roiPayloads;
  roiPayloads.registerInDataStore(m_ROIpayloadName, DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);

}


void ROIPayloadAssemblerModule::event()
{
  StoreArray<ROIid> ROIList(m_ROIListName);
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  unsigned int evtNr = eventMetaDataPtr->getEvent();
  unsigned int countROIs = 0;
  bool accepted = true;

  if (!mAcceptAll) {
    StoreObjPtr<SoftwareTriggerResult> result;
    accepted = SoftwareTrigger::FinalTriggerDecisionCalculator::getFinalTriggerDecision(*result);
  }


  map<VxdID, set<ROIrawID, ROIrawID>> mapOrderedROIraw;

  if (accepted) {
    // skip the preprocessing if the event is not accepted to save CPU time

    B2DEBUG(1, " number of ROIs in the list = " << ROIList.getEntries());

    /// This is necessary as long as DHH is NOT delivering the DHP coordinates in local sensor coordinates (Ucell/Vcell)
    for (auto& iROI : ROIList) {
      // The following would be needed if we make some overflow checks ... but they should not be necessary!

      int layer = (iROI.getSensorID()).getLayerNumber() - 1;
      int ladder = (iROI.getSensorID()).getLadderNumber();
      int sensor = (iROI.getSensorID()).getSensorNumber() - 1;

      m_roiraw.setSystemFlag(0);// System 0 is HLT, 1 would be DATCON
      m_roiraw.setDHHID(((layer) << 5) | ((ladder) << 1) | (sensor));

      m_roiraw.setMinVid(iROI.getMinVid());
      m_roiraw.setMaxVid(iROI.getMaxVid());
      m_roiraw.setMinUid(iROI.getMinUid());
      m_roiraw.setMaxUid(iROI.getMaxUid());

      // order set will drop identical ROIs automatically
      mapOrderedROIraw[iROI.getSensorID()].insert(m_roiraw);
    }

    B2DEBUG(1, " number of original ROIs = " << ROIList.getEntries());

    // The payload is created with a buffer long enough to contains all
    // the ROIs but the actual payload could be smaller, if the ROIs per
    // sensor exceed the (ONSEN) limit, we can save bandwidth by just
    // sending only one full size ROI for that sensor
    for (auto& it : mapOrderedROIraw) {
      if (it.second.size() < mCutNrROIs) countROIs += it.second.size();
      else countROIs++;
    }
  }

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);

  if (payloadPtr.isValid()) {
    B2FATAL("ROIpayload already in datastore, this must not be the case when calling the ROIPayloadAssemblerModule.");
  }

  ROIpayload* payload = new ROIpayload(countROIs);// let the ROIpayload compute the size itself

  payloadPtr.assign(payload);

  // set all the Header flags and event number
  payload->setHeader(accepted || mNoRejectFlag,
                     mSendAllDS  ? (evtNr % mSendAllDS) == 0 : 0, mSendROIsDS ? (evtNr % mSendROIsDS) == 0 : 0);
  payload->setTriggerNumber(evtNr);

  // Set run subrun exp number
  payload->setRunSubrunExpNumber(eventMetaDataPtr->getRun(), eventMetaDataPtr->getSubrun(), eventMetaDataPtr->getExperiment());

  unsigned int addROI = 0;

  if (accepted) {
    // skip this if the event is not accepted (actually mapOrderedROIraw should be empty anyway)
    // iterate over map
    for (auto& it : mapOrderedROIraw) {
      // check size of set
      if (it.second.size() > 0) {
        if (it.second.size() < mCutNrROIs) {
          for (auto& itSet : it.second) {
            payload->addROIraw(itSet.getBigEndian());
            addROI++;
          }
        } else {
          B2INFO("Nr ROI on DHHID " << it.second.begin()->getDHHID() << endl <<
                 " exceeds limit CutNrROIs, thus full sensor ROI is created.");
          const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(it.first);
          const int nPixelsU = aSensorInfo.getUCells();
          const int nPixelsV = aSensorInfo.getVCells();

          m_roiraw.setSystemFlag(0);
          m_roiraw.setDHHID(it.second.begin()->getDHHID());

          m_roiraw.setMinVid(0);
          m_roiraw.setMaxVid(nPixelsV - 1);
          m_roiraw.setMinUid(0);
          m_roiraw.setMaxUid(nPixelsU - 1);

          payload->addROIraw(m_roiraw.getBigEndian());
        }
      }
    }
  }

  payload->setPayloadLength();
  payload->setCRC();

  B2DEBUG(1, " number of ROIs in payload = " << addROI);
}
