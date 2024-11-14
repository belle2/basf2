/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackingDQM/TrackingAbortDQMModule.h"

#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackingAbortDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingAbortDQMModule::TrackingAbortDQMModule() : HistoModule()
{
  setDescription("DQM Module to monitor Tracking Aborts and detector-related quantities.");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("TrackingAbort"));

  setPropertyFlags(c_ParallelProcessingCertified);
}


TrackingAbortDQMModule::~TrackingAbortDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingAbortDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  //histogram index:
  // 0 if the event is triggered OUTSIDE the active_veto window
  std::string tag[2] = {"OUT", "IN"};
  std::string title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};


  //number of events with and without at least one abort
  //outside active_veto window:
  std::string histoName = "EventsWithAborts";
  std::string histoTitle = "Events With at Least one Abort";
  m_nEventsWithAbort[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                   TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                   2, -0.5, 1.5);
  m_nEventsWithAbort[0]->GetYaxis()->SetTitle("Number of Events");
  m_nEventsWithAbort[0]->GetXaxis()->SetBinLabel(1, "No Abort");
  m_nEventsWithAbort[0]->GetXaxis()->SetBinLabel(2, "At Least One Abort");
  m_nEventsWithAbort[0]->SetMinimum(0.1);

  //inside active_veto window:
  m_nEventsWithAbort[1] = new TH1F(*m_nEventsWithAbort[0]);
  m_nEventsWithAbort[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_nEventsWithAbort[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));

  //abort flag reason
  //outside active_veto window:
  histoName = "TrkAbortReason";
  histoTitle = "Tracking Abort Reason";
  m_trackingErrorFlagsReasons[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                            TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                            5, -0.5, 4.5);
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetTitle("Type of error occurred");
  m_trackingErrorFlagsReasons[0]->GetYaxis()->SetTitle("Number of Events");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(1, "Unspecified PR");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(2, "VXDTF2");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(3, "SVDCKF");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(4, "PXDCKF");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(5, "SpacePoint");
  //inside active_veto window:
  m_trackingErrorFlagsReasons[1] = new TH1F(*m_trackingErrorFlagsReasons[0]);
  m_trackingErrorFlagsReasons[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_trackingErrorFlagsReasons[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));


  //SVD L3 occupancy - see SVDDQMDose module for details
  histoName = "SVDL3UOcc";
  histoTitle = "SVD L3 u-side ZS5 Occupancy (%)";
  //outside active_veto window:
  m_svdL3uZS5Occupancy[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                     TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                     90, 0, 100.0 / 1536.0 * 90);
  m_svdL3uZS5Occupancy[0]->GetXaxis()->SetTitle("occupancy [%]");
  m_svdL3uZS5Occupancy[0]->GetYaxis()->SetTitle("Number of Events");
  //inside active_veto window:
  m_svdL3uZS5Occupancy[1] = new TH1F(*m_svdL3uZS5Occupancy[0]);
  m_svdL3uZS5Occupancy[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_svdL3uZS5Occupancy[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));


  //CDC extra hits
  histoName = "nCDCExtraHits";
  histoTitle = "Number of CDC Extra Hits";
  //outside active_veto window:
  m_nCDCExtraHits[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                200, 0, 5000);
  m_nCDCExtraHits[0]->GetXaxis()->SetTitle("nCDCExtraHits");
  m_nCDCExtraHits[0]->GetYaxis()->SetTitle("Number of Events");
  //inside active_veto window:
  m_nCDCExtraHits[1] = new TH1F(*m_nCDCExtraHits[0]);
  m_nCDCExtraHits[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_nCDCExtraHits[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));

  //SVD L3 v-side cluster time
  histoName = "svdL3VTime";
  histoTitle = "Layer3 v-side Cluster Time Distribution";
  //outside active_veto window:
  m_svdTime[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                          TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                          300, -150, 150);
  m_svdTime[0]->GetXaxis()->SetTitle("cluster time (ns)");
  m_svdTime[0]->GetYaxis()->SetTitle("Number of Events");
  //inside active_veto window:
  m_svdTime[1] = new TH1F(*m_svdTime[0]);
  m_svdTime[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_svdTime[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));

  //SVD, CDC Averages
  histoName = "averages";
  histoTitle = "Averages from SVD and CDC";
  //outside active_veto window:
  m_integratedAverages[0] = new TH1D(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                     TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                     5, 0, 5);
  m_integratedAverages[0]->GetYaxis()->SetTitle("Number of X [bin-dependent]");
  m_integratedAverages[0]->GetXaxis()->SetBinLabel(1, "nCDCHitsInner");
  m_integratedAverages[0]->GetXaxis()->SetBinLabel(2, "nCDCHitsOuter");
  m_integratedAverages[0]->GetXaxis()->SetBinLabel(3, "nStripsZS5_L3V");
  m_integratedAverages[0]->GetXaxis()->SetBinLabel(4, "nStripsZS5_L4U");
  m_integratedAverages[0]->GetXaxis()->SetBinLabel(5, "nStripsZS5_L4V");
  //inside active_veto window:
  m_integratedAverages[1] = new TH1D(*m_integratedAverages[0]);
  m_integratedAverages[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_integratedAverages[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));

  oldDir->cd();

}

void TrackingAbortDQMModule::initialize()
{
  m_eventLevelTrackingInfo.isOptional();
  m_eventMetaData.isOptional();
  m_trgSummary.isOptional();
  m_strips.isOptional();
  m_cdcHits.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void TrackingAbortDQMModule::beginRun()
{

  if (m_trackingErrorFlagsReasons[0] != nullptr) m_trackingErrorFlagsReasons[0]->Reset();
  if (m_trackingErrorFlagsReasons[1] != nullptr) m_trackingErrorFlagsReasons[1]->Reset();
  if (m_nEventsWithAbort[0] != nullptr)  m_nEventsWithAbort[0]->Reset();
  if (m_nEventsWithAbort[1] != nullptr)  m_nEventsWithAbort[1]->Reset();
  if (m_svdL3uZS5Occupancy[0] != nullptr)  m_svdL3uZS5Occupancy[0]->Reset();
  if (m_svdL3uZS5Occupancy[1] != nullptr)  m_svdL3uZS5Occupancy[1]->Reset();
  if (m_nCDCExtraHits[0] != nullptr) m_nCDCExtraHits[0]->Reset();
  if (m_nCDCExtraHits[1] != nullptr) m_nCDCExtraHits[1]->Reset();
  if (m_svdTime[0] != nullptr) m_svdTime[0]->Reset();
  if (m_svdTime[1] != nullptr) m_svdTime[1]->Reset();
  if (m_integratedAverages[0] != nullptr) m_integratedAverages[0]->Reset();
  if (m_integratedAverages[1] != nullptr) m_integratedAverages[1]->Reset();
}


void TrackingAbortDQMModule::event()
{

  //skip events in which we do not have EventMetaData or TRGSummary
  if (!m_eventMetaData.isValid()) return;
  if (!m_trgSummary.isValid()) return;

  //skip the empty events
  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkPacketCRCError)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkEventCRCError)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_HLTCrash)
    return;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_ReconstructionAbort)
    return;

  //find out if we are in the passive veto (i=0) or in the active veto window (i=1)
  int index = 0; //events accepted in the passive veto window but not in the active
  try {
    if (m_trgSummary->testInput("passive_veto") == 1 &&  m_trgSummary->testInput("cdcecl_veto") == 0) index = 1;
  } catch (const std::exception&) {
    B2WARNING("Requested trigger bits for passive_veto or cdcecl_veto do not exist");
  }

  //fill the tracking abort reason histogram & nEvents with Abort
  if (m_eventLevelTrackingInfo.isValid()) {
    if (m_eventLevelTrackingInfo->hasAnErrorFlag()) {

      m_nEventsWithAbort[index]->Fill(1);

      if (m_eventLevelTrackingInfo->hasUnspecifiedTrackFindingFailure())
        m_trackingErrorFlagsReasons[index]->Fill(0);
      if (m_eventLevelTrackingInfo->hasVXDTF2AbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(1);
      if (m_eventLevelTrackingInfo->hasSVDCKFAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(2);
      if (m_eventLevelTrackingInfo->hasPXDCKFAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(3);
      if (m_eventLevelTrackingInfo->hasSVDSpacePointCreatorAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(4);
    } else { //EventLevelTrackingInfo valid but no error
      m_nEventsWithAbort[index]->Fill(0);
    }
  } else //EventLevelTrackingInfo not valid
    m_nEventsWithAbort[index]->Fill(0);

  //compute the number of ZS5 strips of L3 and L4, both sides
  float nStripsL3UZS5 = 0;
  float nStripsL3VZS5 = 0;
  float nStripsL4UZS5 = 0;
  float nStripsL4VZS5 = 0;
  for (const SVDShaperDigit& hit : m_strips) {
    const VxdID& sensorID = hit.getSensorID();
    if (sensorID.getLayerNumber() > 4) continue;
    const float noise = m_NoiseCal.getNoise(sensorID, hit.isUStrip(), hit.getCellID());
    const float cutMinSignal = std::round(5 * noise);

    if (hit.passesZS(1, cutMinSignal)) {
      if (sensorID.getLayerNumber() == 3) {
        if (hit.isUStrip()) nStripsL3UZS5++;
        else nStripsL3VZS5++;
      } else if (hit.isUStrip()) nStripsL4UZS5++;
      else nStripsL4VZS5++;
    }
  }

  //fill the SVD L3 v-side cluster time
  for (const SVDCluster& hit : m_clusters) {
    const VxdID& sensorID = hit.getSensorID();
    if (sensorID.getLayerNumber() != 3) continue;
    if (hit.isUCluster()) continue;

    m_svdTime[index]->Fill(hit.getClsTime());
  }

  // fill the svd L3 v ZS5 occupancy, add the overflow in the last bin to make them visible in the plot
  m_svdL3uZS5Occupancy[index]->Fill(std::min((double)nStripsL3UZS5 / m_nStripsL3U * 100, (double)5.82));

  //fill the nCDCExtraHits, add the overflow in the last bin to make them visible in the plot
  if (m_eventLevelTrackingInfo.isValid())
    m_nCDCExtraHits[index]->Fill(std::min((int)m_eventLevelTrackingInfo->getNCDCHitsNotAssigned(), (int)4999));

  //compute number of CDC hits in the inner and outer layers
  int nCDCHitsInner = 0;
  int nCDCHitsOuter = 0;
  for (const CDCHit& hit : m_cdcHits) {
    if (hit.getISuperLayer() == 0) nCDCHitsInner++;
    else nCDCHitsOuter++;
  }

  // fill the integrated averages TH1F
  // bin 1: nCDCHits Inner layers
  updateBinContent(index, 1, nCDCHitsInner);
  // bin 2: nCDCHits Outer layers
  updateBinContent(index, 2, nCDCHitsOuter);
  // bin 3: nStrips L3 V-side
  updateBinContent(index, 3, nStripsL3VZS5);
  // bin 4: nStrips L4 U-side
  updateBinContent(index, 4, nStripsL4UZS5);
  // bin 5: nStrips L4 V-side
  updateBinContent(index, 5, nStripsL4VZS5);


}

void TrackingAbortDQMModule::updateBinContent(int index, int bin, float valueToBeAdded)
{
  float oldValue = m_integratedAverages[index]->GetBinContent(bin);
  m_integratedAverages[index]->SetBinContent(bin, oldValue + valueToBeAdded);
}
