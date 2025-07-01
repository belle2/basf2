/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDDQMClustersOnTrackModule.h"

#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace SoftwareTrigger;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMClustersOnTrack);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMClustersOnTrackModule::SVDDQMClustersOnTrackModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module for clusters related to tracks.");

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("skipHLTRejectedEvents", m_skipRejectedEvents, "If True, skip events rejected by HLT.", bool(true));
  addParam("TriggerBin", m_tb, "select events for a specific trigger bin, if -1 then no selection is applied (default)", int(-1));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("SVDClsTrk"));
  addParam("desynchronizeSVDTime", m_desynchSVDTime,
           "if True, svd time back in SVD time reference", bool(false));
  addParam("EventInfo", m_svdEventInfoName, "SVDEventInfo StoreArray name.", std::string(""));
  addParam("Clusters", m_svdClustersName, "SVDCluster StoreArray name.", std::string(""));
  addParam("RecoDigits", m_svdRecoDigitsName, "SVDRecoDigits StoreArray name.", std::string(""));
  addParam("ShaperDigits", m_svdShaperDigitsName, "SVDShaperDigits StoreArray name.", std::string(""));
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));

  m_histoList = new TList();

  m_ladderMap =  {
    {{3, 1}, 0}, {{3, 2}, 1},
    {{4, 1}, 2}, {{4, 2}, 3}, {{4, 3}, 4},
    {{5, 1}, 5}, {{5, 2}, 6}, {{5, 3}, 7}, {{5, 4}, 8},
    {{6, 1}, 9}, {{6, 2}, 10}, {{6, 3}, 11}, {{6, 4}, 12}, {{6, 5}, 13}
  };
}


SVDDQMClustersOnTrackModule::~SVDDQMClustersOnTrackModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMClustersOnTrackModule::defineHisto()
{

  // for test
  m_addSensorLabel.push_back("L4.1.1");
  m_addSensorLabel.push_back("L5.2.1");

  if (m_addSensorLabel.size() != 0)
    m_addSensorPlots = true;

  // geometry checks
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2FATAL("Missing geometry for VXD, check steering file.");
  }
  if (gTools->getNumberOfSVDLayers() == 0) {
    B2WARNING("Missing geometry for SVD, SVD-DQM is skipped.");
    return;
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // basic constants presets:
  int nSVDSensors = gTools->getNumberOfSVDSensors();

  if (m_addSensorPlots) {
    m_clustrkChargeU = new TH1F*[m_addSensorLabel.size()];
    m_clustrkChargeV = new TH1F*[m_addSensorLabel.size()];
    m_clustrkSNRU    = new TH1F*[m_addSensorLabel.size()];
    m_clustrkSNRV    = new TH1F*[m_addSensorLabel.size()];
    m_clustrkTimeU   = new TH1F*[m_addSensorLabel.size()];
    m_clustrkTimeV   = new TH1F*[m_addSensorLabel.size()];
  }

  int ChargeBins = 80;
  float ChargeMax = 160;
  int SNRBins = 50;
  float SNRMax = 100;
  int TimeBins = 300;
  float TimeMin = -150;
  float TimeMax = 150;

  int MaxBinBins = 6;
  int MaxBinMax = 6;

  TString refFrame = "in FTSW reference";
  if (m_desynchSVDTime)
    refFrame = "in SVD reference";


  //----------------------------------------------------------------
  // Charge of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  TString name = "SVDTRK_ClusterChargeU3";
  TString title = "SVD U-Cluster-on-Track Charge for layer 3 sensors";
  m_clsTrkChargeU3 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeU3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeU3);
  name = "SVDTRK_ClusterChargeV3";
  title = "SVD V-Cluster-on-Track Charge for layer 3 sensors";
  m_clsTrkChargeV3 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeV3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeV3);

  name = "SVDTRK_ClusterChargeU456";
  title = "SVD U-Cluster-on-Track Charge for layers 4,5,6 sensors";
  m_clsTrkChargeU456 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeU456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeU456);

  name = "SVDTRK_ClusterChargeV456";
  title = "SVD V-Cluster-on-Track Charge for layers 4,5,6 sensors";
  m_clsTrkChargeV456 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clsTrkChargeV456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clsTrkChargeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkChargeV456);


  m_clsTrkCharge = new TH1F*[m_ladderMap.size()];
  m_clsTrkSNR    = new TH1F*[m_ladderMap.size()];

  for (const auto& it : m_ladderMap) {
    std::pair<int, int> p = it.first;
    int layer = p.first;
    int sensor = p.second;
    int idx = it.second;
    name = Form("SVDTRK_ClusterCharge_L%d.x.%d", layer, sensor);
    title = Form("SVD Cluster-on-Track Charge for L%d.x.%d", layer, sensor);
    m_clsTrkCharge[idx] = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
    m_clsTrkCharge[idx]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clsTrkCharge[idx]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clsTrkCharge[idx]);

    //printf("name %s layer %d sensor %d index %d\n", name.Data(), layer, sensor, idx);
    name = Form("SVDTRK_ClusterSNR_L%d.x.%d", layer, sensor);
    title = Form("SVD Cluster-on-Track SNR for L%d.x.%d", layer, sensor);
    m_clsTrkSNR[idx] = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
    m_clsTrkSNR[idx]->GetXaxis()->SetTitle("cluster SNR");
    m_clsTrkSNR[idx]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clsTrkSNR[idx]);
  }

  //----------------------------------------------------------------
  // SNR of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = "SVDTRK_ClusterSNRU3";
  title = "SVD U-Cluster-on-Track SNR for layer 3 sensors";
  m_clsTrkSNRU3 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clsTrkSNRU3->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRU3);
  name = "SVDTRK_ClusterSNRV3";
  title = "SVD V-Cluster-on-Track SNR for layer 3 sensors";
  m_clsTrkSNRV3 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clsTrkSNRV3->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRV3);

  name = "SVDTRK_ClusterSNRU456";
  title = "SVD U-Cluster-on-Track SNR for layers 4,5,6 sensors";
  m_clsTrkSNRU456 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clsTrkSNRU456->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRU456);
  name = "SVDTRK_ClusterSNRV456";
  title = "SVD V-Cluster-on-Track SNR for layers 4,5,6 sensors";
  m_clsTrkSNRV456 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clsTrkSNRV456->GetXaxis()->SetTitle("cluster SNR");
  m_clsTrkSNRV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkSNRV456);

  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = "SVDTRK_ClusterTimeU3";
  title = Form("SVD U-Cluster-on-Track Time %s for layer 3 sensors", refFrame.Data());
  m_clsTrkTimeU3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeU3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clsTrkTimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeU3);
  name = "SVDTRK_ClusterTimeV3";
  title = Form("SVD V-Cluster-on-Track Time %s for layer 3 sensors", refFrame.Data());
  m_clsTrkTimeV3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeV3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clsTrkTimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeV3);

  if (m_3Samples) {
    name = "SVDTRK_Cluster3SampleTimeU3";
    title = Form("SVD U-Cluster-on-Track Time %s for layer 3 sensors for 3 samples", refFrame.Data());
    m_cls3SampleTrkTimeU3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls3SampleTrkTimeU3->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls3SampleTrkTimeU3->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls3SampleTrkTimeU3);
    name = "SVDTRK_Cluster3SampleTimeV3";
    title = Form("SVD V-Cluster-on-Track Time %s for layer 3 sensors for 3 samples", refFrame.Data());
    m_cls3SampleTrkTimeV3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls3SampleTrkTimeV3->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls3SampleTrkTimeV3->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls3SampleTrkTimeV3);

    name = "SVDTRK_Cluster6SampleTimeU3";
    title = Form("SVD U-Cluster-on-Track Time %s for layer 3 sensors for 6 samples", refFrame.Data());
    m_cls6SampleTrkTimeU3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls6SampleTrkTimeU3->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls6SampleTrkTimeU3->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls6SampleTrkTimeU3);
    name = "SVDTRK_Cluster6SampleTimeV3";
    title = Form("SVD V-Cluster-on-Track Time %s for layer 3 sensors for 6 samples", refFrame.Data());
    m_cls6SampleTrkTimeV3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls6SampleTrkTimeV3->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls6SampleTrkTimeV3->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls6SampleTrkTimeV3);
  }

  name = "SVDTRK_ClusterTimeU456";
  title = Form("SVD U-Cluster-on-Track Time %s for layers 4,5,6 sensors", refFrame.Data());
  m_clsTrkTimeU456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeU456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clsTrkTimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeU456);
  name = "SVDTRK_ClusterTimeV456";
  title = Form("SVD V-Cluster-on-Track Time %s for layers 4,5,6 sensors", refFrame.Data());
  m_clsTrkTimeV456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clsTrkTimeV456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clsTrkTimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clsTrkTimeV456);

  if (m_3Samples) {
    name = "SVDTRK_Cluster3SampleTimeU456";
    title = Form("SVD U-Cluster-on-Track Time %s for layers 4,5,6 sensors for 3 samples", refFrame.Data());
    m_cls3SampleTrkTimeU456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls3SampleTrkTimeU456->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls3SampleTrkTimeU456->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls3SampleTrkTimeU456);
    name = "SVDTRK_Cluster3SampleTimeV456";
    title = Form("SVD V-Cluster-on-Track Time %s for layers 4,5,6 sensors for 3 samples", refFrame.Data());
    m_cls3SampleTrkTimeV456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls3SampleTrkTimeV456->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls3SampleTrkTimeV456->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls3SampleTrkTimeV456);

    name = "SVDTRK_Cluster6SampleTimeU456";
    title = Form("SVD U-Cluster-on-Track Time %s for layers 4,5,6 sensors for 6 samples", refFrame.Data());
    m_cls6SampleTrkTimeU456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls6SampleTrkTimeU456->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls6SampleTrkTimeU456->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls6SampleTrkTimeU456);
    name = "SVDTRK_Cluster6SampleTimeV456";
    title = Form("SVD V-Cluster-on-Track Time %s for layers 4,5,6 sensors for 6 samples", refFrame.Data());
    m_cls6SampleTrkTimeV456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_cls6SampleTrkTimeV456->GetXaxis()->SetTitle("cluster time (ns)");
    m_cls6SampleTrkTimeV456->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_cls6SampleTrkTimeV456);
  }

  //----------------------------------------------------------------
  // EventT0 vs Time of clusters for U and V sides
  //----------------------------------------------------------------
  name = "SVDTRK_ClusterTimeUvsEventT0";
  title = Form("SVD U-Cluster-on-Track Time vs EventT0 %s for layer 3 sensors", refFrame.Data());
  m_clsTrkTimeUEvtT0 = new TH2F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax, 100, -50, 50);
  m_clsTrkTimeUEvtT0->GetXaxis()->SetTitle("clusters time (ns)");
  m_clsTrkTimeUEvtT0->GetYaxis()->SetTitle("EventT0 (ns)");
  m_histoList->Add(m_clsTrkTimeUEvtT0);
  name = "SVDTRK_ClusterTimeVvsEventT0";
  title = Form("SVD V-Cluster-on-Track Time vs EventT0 %s for layer 3 sensors", refFrame.Data());
  m_clsTrkTimeVEvtT0 = new TH2F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax, 100, -50, 50);
  m_clsTrkTimeVEvtT0->GetXaxis()->SetTitle("cluster time (ns)");
  m_clsTrkTimeVEvtT0->GetYaxis()->SetTitle("EventT0 (ns)");
  m_histoList->Add(m_clsTrkTimeVEvtT0);

  //----------------------------------------------------------------
  // MaxBin of strips for all sensors (offline ZS)
  //----------------------------------------------------------------
  name = "SVDTRK_StripMaxBinUAll";
  title = "SVD U-Strip-on-Track MaxBin for all sensors";
  m_stripMaxBinUAll = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinUAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinUAll);
  name = "SVDTRK_StripMaxBinVAll";
  title = "SVD V-Strip-on-Track MaxBin for all sensors";
  m_stripMaxBinVAll = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinVAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinVAll);

  //----------------------------------------------------------------
  // Additional sensor plots
  //----------------------------------------------------------------
  if (m_addSensorPlots) {
    for (int i = 0; i < nSVDSensors; i++) {
      VxdID id = gTools->getSensorIDFromSVDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      VxdID sensorID(iLayer, iLadder, iSensor);
      string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
      string sensorId = str(format("L%1%.%2%.%3%") % iLayer % iLadder % iSensor);

      auto it = find(m_addSensorLabel.begin(), m_addSensorLabel.end(), sensorId);
      if (it == m_addSensorLabel.end()) continue;
      int idx = distance(m_addSensorLabel.begin(), it);

      //----------------------------------------------------------------
      // Charge of clusters
      //----------------------------------------------------------------
      name = str(format("SVDTRK_%1%_ClusterChargeU") % idx);
      title = str(format("SVD Sensor %1% U-Cluster-on-Track Charge") % sensorDescr);
      m_clustrkChargeU[idx] = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
      m_clustrkChargeU[idx]->GetXaxis()->SetTitle("cluster charge [ke-]");
      m_clustrkChargeU[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkChargeU[idx]);
      name = str(format("SVDTRK_%1%_ClusterChargeV") % idx);
      title = str(format("SVD Sensor %1% V-Cluster-on-Track Charge") % sensorDescr);
      m_clustrkChargeV[idx] = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
      m_clustrkChargeV[idx]->GetXaxis()->SetTitle("cluster charge [ke-]");
      m_clustrkChargeV[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkChargeV[idx]);
      //----------------------------------------------------------------
      // SNR of clusters
      //----------------------------------------------------------------
      name = str(format("SVDTRK_%1%_ClusterSNRU") % idx);
      title = str(format("SVD Sensor %1% U-Cluster-on-Track SNR") % sensorDescr);
      m_clustrkSNRU[idx] = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
      m_clustrkSNRU[idx]->GetXaxis()->SetTitle("cluster SNR");
      m_clustrkSNRU[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkSNRU[idx]);
      name = str(format("SVDTRK_%1%_ClusterSNRV") % idx);
      title = str(format("SVD Sensor %1% V-Cluster-on-Track SNR") % sensorDescr);
      m_clustrkSNRV[idx] = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
      m_clustrkSNRV[idx]->GetXaxis()->SetTitle("cluster SNR");
      m_clustrkSNRV[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkSNRV[idx]);
      //----------------------------------------------------------------
      // Cluster time distribution
      //----------------------------------------------------------------
      name = str(format("SVDTRK_%1%_ClusterTimeU") % idx);
      title = Form("SVD Sensor %s U-Cluster-on-Track Time %s", sensorDescr.c_str(), refFrame.Data());
      m_clustrkTimeU[idx] = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
      m_clustrkTimeU[idx]->GetXaxis()->SetTitle("cluster time (ns)");
      m_clustrkTimeU[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkTimeU[idx]);
      name = str(format("SVDTRK_%1%_ClusterTimeV") % idx);
      title = Form("SVD Sensor %s V-Cluster-on-Track Time %s", sensorDescr.c_str(), refFrame.Data());
      m_clustrkTimeV[idx] = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
      m_clustrkTimeV[idx]->GetXaxis()->SetTitle("cluster time (ns)");
      m_clustrkTimeV[idx]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustrkTimeV[idx]);
    }
  }
  oldDir->cd();
}


void SVDDQMClustersOnTrackModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() != 0) {

    m_svdEventInfo.isOptional(m_svdEventInfoName);
    m_eventT0.isOptional();
    m_tracks.isOptional();
    m_resultStoreObjectPointer.isOptional();

  }
}

void SVDDQMClustersOnTrackModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  // Add experiment and run number to the title of selected histograms (CR shifter plots)
  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();
  TString runID = TString::Format(" ~ Exp%d Run%d", m_expNumber, m_runNumber);

  //reset histograms
  TObject* obj;
  TIter nextH(m_histoList);
  while ((obj = nextH()))
    if (obj->InheritsFrom("TH1")) {
      ((TH1F*)obj)->Reset();

      TString tmp = (TString)obj->GetTitle();
      Int_t pos = tmp.Last('~');
      if (pos == -1) pos = tmp.Length() + 2;

      TString title = tmp(0, pos - 2);
      ((TH1F*)obj)->SetTitle(title + runID);
    }
}

void SVDDQMClustersOnTrackModule::event()
{

  if (!m_tracks.isValid()) {
    B2WARNING("Missing Tracks StoreArray. Skipping SVDDQMClustersOnTrack");
    return;
  }

  if (!m_svdEventInfo.isValid())
    m_tb = -1;
  else {
    if (m_tb != -1)
      if (m_svdEventInfo->getModeByte().getTriggerBin() != m_tb)
        return;
  }

  int nSamples = 0;
  if (m_svdEventInfo.isValid())
    nSamples = m_svdEventInfo->getNSamples();
  else
    return;

  // get EventT0 if present and valid
  double eventT0 = -1000;
  if (m_eventT0.isOptional())
    if (m_eventT0.isValid())
      if (m_eventT0->hasEventT0())
        eventT0 = m_eventT0->getEventT0();

  // if svd time in SVD time reference is shown, eventT0 is also synchronized with SVD reference frame, firstFrame = 0
  if (m_desynchSVDTime && m_svdEventInfo.isValid())
    eventT0 = eventT0 - m_svdEventInfo->getSVD2FTSWTimeShift(0);

  //check HLT decision and increase number of events only if the event has been accepted

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;


  for (const Track& track : m_tracks) {

    const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
    if (not tfr) continue;

    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();
    if (not recoTrack) continue;

    for (const SVDCluster& svdCluster : recoTrack->getRelationsWith<SVDCluster>(m_svdClustersName)) {

      int iLayer = svdCluster.getSensorID().getLayerNumber();
      int iLadder = svdCluster.getSensorID().getLadderNumber();
      int iSensor = svdCluster.getSensorID().getSensorNumber();

      if (m_addSensorPlots) {
        string sensorId = str(format("L%1%.%2%.%3%") % iLayer % iLadder % iSensor);

        auto it = find(m_addSensorLabel.begin(), m_addSensorLabel.end(), sensorId);
        if (it != m_addSensorLabel.end()) {
          int idx = distance(m_addSensorLabel.begin(), it);

          if (svdCluster.isUCluster()) {
            if (m_clustrkChargeU[idx]) m_clustrkChargeU[idx]->Fill(svdCluster.getCharge() / 1000.0);
            if (m_clustrkSNRU[idx]) m_clustrkSNRU[idx]->Fill(svdCluster.getSNR());
            if (m_clustrkTimeU[idx]) m_clustrkTimeU[idx]->Fill(svdCluster.getClsTime());
          } else {
            if (m_clustrkChargeV[idx]) m_clustrkChargeV[idx]->Fill(svdCluster.getCharge() / 1000.0);
            if (m_clustrkSNRV[idx]) m_clustrkSNRV[idx]->Fill(svdCluster.getSNR());
            if (m_clustrkTimeV[idx]) m_clustrkTimeV[idx]->Fill(svdCluster.getClsTime());
          }
        }
      }


      std::pair<int, int> p(iLayer, iSensor);
      int idx = m_ladderMap[p];

      if (m_clsTrkCharge[idx] != nullptr) m_clsTrkCharge[idx]->Fill(svdCluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clsTrkSNR[idx] != nullptr) m_clsTrkSNR[idx]->Fill(svdCluster.getSNR());

      float time = svdCluster.getClsTime();
      if (m_desynchSVDTime && m_svdEventInfo.isValid())
        time = time - m_svdEventInfo->getSVD2FTSWTimeShift(svdCluster.getFirstFrame());

      if (svdCluster.isUCluster()) {

        m_clsTrkTimeUEvtT0->Fill(time, eventT0);

        if (iLayer == 3) {
          if (m_clsTrkChargeU3 != nullptr) m_clsTrkChargeU3->Fill(svdCluster.getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRU3 != nullptr) m_clsTrkSNRU3->Fill(svdCluster.getSNR());
          if (m_clsTrkTimeU3 != nullptr) m_clsTrkTimeU3->Fill(time);

          if (m_3Samples) {
            if (nSamples == 3) {
              if (m_cls3SampleTrkTimeU3 != nullptr) m_cls3SampleTrkTimeU3->Fill(time);
            } else {
              if (m_cls6SampleTrkTimeU3 != nullptr) m_cls6SampleTrkTimeU3->Fill(time);
            }
          }
        } else {
          if (m_clsTrkChargeU456 != nullptr) m_clsTrkChargeU456->Fill(svdCluster.getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRU456 != nullptr) m_clsTrkSNRU456->Fill(svdCluster.getSNR());
          if (m_clsTrkTimeU456 != nullptr) m_clsTrkTimeU456->Fill(time);

          if (m_3Samples) {
            if (nSamples == 3) {
              if (m_cls3SampleTrkTimeU456 != nullptr) m_cls3SampleTrkTimeU456->Fill(time);
            } else {
              if (m_cls6SampleTrkTimeU456 != nullptr) m_cls6SampleTrkTimeU456->Fill(time);
            }
          }
        }

        for (const SVDRecoDigit& recoDigit : svdCluster.getRelationsTo<SVDRecoDigit>(m_svdRecoDigitsName)) {

          SVDShaperDigit* shaper = recoDigit.getRelatedTo<SVDShaperDigit>(m_svdShaperDigitsName);
          if (m_stripMaxBinUAll != nullptr and shaper != nullptr) m_stripMaxBinUAll->Fill(shaper->getMaxTimeBin());
        }


      } else {

        m_clsTrkTimeVEvtT0->Fill(time, eventT0);

        if (iLayer == 3) {
          if (m_clsTrkChargeV3 != nullptr) m_clsTrkChargeV3->Fill(svdCluster.getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRV3 != nullptr) m_clsTrkSNRV3->Fill(svdCluster.getSNR());
          if (m_clsTrkTimeV3 != nullptr) m_clsTrkTimeV3->Fill(time);
          if (m_3Samples) {
            if (nSamples == 3) {
              if (m_cls3SampleTrkTimeV3 != nullptr) m_cls3SampleTrkTimeV3->Fill(time);
            } else {
              if (m_cls6SampleTrkTimeV3 != nullptr) m_cls6SampleTrkTimeV3->Fill(time);
            }
          }
        } else {
          if (m_clsTrkChargeV456 != nullptr) m_clsTrkChargeV456->Fill(svdCluster.getCharge() / 1000.0);  // in kelectrons
          if (m_clsTrkSNRV456 != nullptr) m_clsTrkSNRV456->Fill(svdCluster.getSNR());
          if (m_clsTrkTimeV456 != nullptr) m_clsTrkTimeV456->Fill(time);
          if (m_3Samples) {
            if (nSamples == 3) {
              if (m_cls3SampleTrkTimeV456 != nullptr) m_cls3SampleTrkTimeV456->Fill(time);
            } else {
              if (m_cls6SampleTrkTimeV456 != nullptr) m_cls6SampleTrkTimeV456->Fill(time);
            }
          }
        }

        for (const SVDRecoDigit& recoDigit : svdCluster.getRelationsTo<SVDRecoDigit>(m_svdRecoDigitsName)) {

          SVDShaperDigit* shaper = recoDigit.getRelatedTo<SVDShaperDigit>(m_svdShaperDigitsName);
          if (m_stripMaxBinVAll != nullptr and shaper != nullptr) m_stripMaxBinVAll->Fill(shaper->getMaxTimeBin());
        }

      }

    }
  }
}


void SVDDQMClustersOnTrackModule::terminate()
{

  delete m_histoList;

}
