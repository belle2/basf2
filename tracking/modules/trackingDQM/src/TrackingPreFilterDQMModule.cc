/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackingDQM/TrackingPreFilterDQMModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>
#include <string>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackingPreFilterDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingPreFilterDQMModule::TrackingPreFilterDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("DQM Module to monitor Tracking-related quantities before the HLT filter.");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("TrackingPreFilter"));

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}


TrackingPreFilterDQMModule::~TrackingPreFilterDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingPreFilterDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  //number of events with and without at least one abort
  m_nEventsWithAbort = new TH1F("EventsWithAborts", "Events With at Least one Abort", 2, -0.5, 1.5);
  m_nEventsWithAbort->GetXaxis()->SetBinLabel(1, "No Abort");
  m_nEventsWithAbort->GetXaxis()->SetBinLabel(2, "At Least One Abort");

  //abort flag reason
  m_trackingErrorFlagsReasons = new TH1F("TrackingErrorFlagsReasons",
                                         "Tracking errors by reason. A single event may fall in multiple bins.", 5, -0.5, 4.5);
  m_trackingErrorFlagsReasons->GetXaxis()->SetTitle("Type of error occurred");
  m_trackingErrorFlagsReasons->GetYaxis()->SetTitle("Number of events");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(1, "Unspecified PR");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(2, "VXDTF2");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(3, "SVDCKF");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(4, "PXDCKF");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(5, "SpacePoint");

  //SVD L3 occupancy
  //see SVDDQMDose module for details
  m_svdL3vZS5Occupancy = new TH1F("SVDL3Occ", "SVD L3 v-side ZS5 Occupancy [%]", 90, 0, 100.0 / 1536.0 * 90);
  m_svdL3vZS5Occupancy->GetXaxis()->SetTitle("occupancy [%]");
  m_svdL3vZS5Occupancy->GetYaxis()->SetTitle("Number Of Events");


  //CDC extra hits

  //


  oldDir->cd();
}

void TrackingPreFilterDQMModule::initialize()
{
  m_eventLevelTrackingInfo.isOptional();
  m_eventMetaData.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void TrackingPreFilterDQMModule::beginRun()
{

  int expNumber = m_eventMetaData->getExperiment();
  int runNumber = m_eventMetaData->getRun();

  TString histoTitle = TString::Format("SVD Data Format Monitor, Exp %d Run %d", expNumber, runNumber);

  if (m_trackingErrorFlagsReasons != nullptr) {
    m_trackingErrorFlagsReasons->Reset();
    m_trackingErrorFlagsReasons->SetTitle(histoTitle.Data());
  }

  if (m_nEventsWithAbort != nullptr)  m_nEventsWithAbort->Reset();
  if (m_svdL3vZS5Occupancy != nullptr)  m_svdL3vZS5Occupancy->Reset();

}


void TrackingPreFilterDQMModule::event()
{

  //skip the empty events
  bool eventIsEmpty = false;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkPacketCRCError)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkEventCRCError)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_HLTCrash)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_ReconstructionAbort)
    eventIsEmpty = true;

  if (eventIsEmpty) return;

  //fill the tracking abort reason histogram & nEvents with Abort
  if (m_eventLevelTrackingInfo.isValid()) {
    if (m_eventLevelTrackingInfo->hasAnErrorFlag()) {

      m_nEventsWithAbort->Fill(1);

      if (m_eventLevelTrackingInfo->hasUnspecifiedTrackFindingFailure())
        m_trackingErrorFlagsReasons->Fill(0);
      if (m_eventLevelTrackingInfo->hasVXDTF2AbortionFlag())
        m_trackingErrorFlagsReasons->Fill(1);
      if (m_eventLevelTrackingInfo->hasSVDCKFAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(2);
      if (m_eventLevelTrackingInfo->hasPXDCKFAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(3);
      if (m_eventLevelTrackingInfo->hasSVDSpacePointCreatorAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(4);
    } else { //EventLevelTrackingIinfo valid but no error
      m_nEventsWithAbort->Fill(0);
    }
  } else //EventLevelTrackingIinfo not valid
    m_nEventsWithAbort->Fill(0);


  // fill the svd L3 v ZS5 occupancy
  float nStripsL3VZS5 = 0;
  for (const SVDShaperDigit& hit : m_strips) {
    const VxdID& sensorID = hit.getSensorID();
    if (sensorID.getLayerNumber() != 3) continue;
    if (hit.isUStrip()) continue;
    float noise = m_NoiseCal.getNoise(sensorID, 0, hit.getCellID());
    float cutMinSignal = 5 * noise + 0.5;
    cutMinSignal = (int)cutMinSignal;

    if (hit.passesZS(1, cutMinSignal)) nStripsL3VZS5++;
  }
  m_svdL3vZS5Occupancy->Fill(nStripsL3VZS5 / nStripsL3V * 100);

}


