/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni, Bjoern Spruck          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROIPayloadAssemblerModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <stdlib.h>
#include <set>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
// #include <tracking/dataobjects/ROIrawID.h>

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
  m_eventMetaData.isRequired();
  m_ROIList.isOptional(m_ROIListName);

  m_roiPayloads.registerInDataStore(
    m_ROIpayloadName); // DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered will not work with two modules in seperate path branches

  // in case we don't accept all events, we have to look
  // up the trigger decision
  if (!mAcceptAll) {
    StoreObjPtr<SoftwareTriggerResult> result;
    result.isRequired();
  }
}


void ROIPayloadAssemblerModule::event()
{
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  unsigned int evtNr = m_eventMetaData->getEvent();
  unsigned int countROIs = 0;
  bool accepted = true;

  if (!mAcceptAll) {
    StoreObjPtr<SoftwareTriggerResult> result;
    if (!result.isValid()) {
      B2FATAL("SoftwareTriggerResult object not available but needed to generate the ROI payload.");
    }
    accepted = SoftwareTrigger::FinalTriggerDecisionCalculator::getFinalTriggerDecision(*result);
  }

  map<VxdID, set<ROIrawID, ROIrawID>> mapOrderedROIraw;

  if (accepted) {
    // skip the preprocessing if the event is not accepted to save CPU time

    B2DEBUG(1, " number of ROIs in the list = " << m_ROIList.getEntries());

    for (auto& iROI : m_ROIList) {
      ROIrawID roiraw; /**< 64 bit union containing a single ROI info to be sent to ONSEN*/

      int layer = (iROI.getSensorID()).getLayerNumber() - 1;
      int ladder = (iROI.getSensorID()).getLadderNumber();
      int sensor = (iROI.getSensorID()).getSensorNumber() - 1;

      roiraw.setSystemFlag(0);// System 0 is HLT, 1 would be DATCON
      roiraw.setDHHID(((layer) << 5) | ((ladder) << 1) | (sensor));

      roiraw.setMinVid(iROI.getMinVid());
      roiraw.setMaxVid(iROI.getMaxVid());
      roiraw.setMinUid(iROI.getMinUid());
      roiraw.setMaxUid(iROI.getMaxUid());

      // order set will drop identical ROIs automatically
      mapOrderedROIraw[iROI.getSensorID()].insert(roiraw);
    }

    B2DEBUG(1, " number of original ROIs = " << m_ROIList.getEntries());

    // The payload is created with a buffer long enough to contains all
    // the ROIs but the actual payload could be smaller, if the ROIs per
    // sensor exceed the (ONSEN) limit, we can save bandwidth by just
    // sending only one full size ROI for that sensor
    for (auto& it : mapOrderedROIraw) {
      if (it.second.size() < mCutNrROIs) countROIs += it.second.size();
      else countROIs++;
    }
  }

  if (m_roiPayloads.isValid()) {
    B2FATAL("ROIpayload already in datastore, this must not be the case when calling the ROIPayloadAssemblerModule.");
  }

  ROIpayload* payload = new ROIpayload(countROIs);// let the ROIpayload compute the size itself

  m_roiPayloads.assign(payload);

  // set all the Header flags and event number
  payload->setHeader(accepted || mNoRejectFlag,
                     mSendAllDS  ? (evtNr % mSendAllDS) == 0 : 0, mSendROIsDS ? (evtNr % mSendROIsDS) == 0 : 0);
  payload->setTriggerNumber(evtNr);

  // Set run subrun exp number
  payload->setRunSubrunExpNumber(m_eventMetaData->getRun(), m_eventMetaData->getSubrun(), m_eventMetaData->getExperiment());

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
          ROIrawID roiraw; /**< 64 bit union containing a single ROI info to be sent to ONSEN*/
          B2INFO("Nr ROI on DHHID " << it.second.begin()->getDHHID() << endl <<
                 " exceeds limit CutNrROIs, thus full sensor ROI is created.");
          const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(it.first);
          const int nPixelsU = aSensorInfo.getUCells();
          const int nPixelsV = aSensorInfo.getVCells();

          roiraw.setSystemFlag(0);
          roiraw.setDHHID(it.second.begin()->getDHHID());

          roiraw.setMinVid(0);
          roiraw.setMaxVid(nPixelsV - 1);
          roiraw.setMinUid(0);
          roiraw.setMaxUid(nPixelsU - 1);

          payload->addROIraw(roiraw.getBigEndian());
        }
      }
    }
  }

  payload->setPayloadLength();
  payload->setCRC();

  B2DEBUG(1, " number of ROIs in payload = " << addROI);
}
