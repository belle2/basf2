/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDDQMExpressRecoModule.h"

#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>

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
REG_MODULE(SVDDQMExpressReco);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMExpressRecoModule::SVDDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("Original SVD DQM module for ExpressReco.");

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("offlineZSShaperDigits", m_storeSVDShaperDigitsName, "ShaperDigits StoreArray name - usually ZS5 strips.",
           std::string("SVDShaperDigitsZS5"));
  addParam("ShaperDigits", m_storeNoZSSVDShaperDigitsName, "not zero-suppressed ShaperDigits StoreArray name.",
           std::string("SVDShaperDigits"));
  addParam("Clusters", m_storeSVDClustersName, "Cluster StoreArray name.",
           std::string("SVDClusters"));
  addParam("skipHLTRejectedEvents", m_skipRejectedEvents, "If True, skip events rejected by HLT.", bool(true));
  addParam("ShowAllHistos", m_ShowAllHistos, "Flag to show all histos in DQM, default = 0.", int(0));
  addParam("desynchronizeSVDTime", m_desynchSVDTime,
           "if True, svd time back in SVD time reference.", bool(false));
  addParam("CutSVDCharge", m_CutSVDCharge,
           "minimum charge (ADC) to fill the strip-hitmap histogram.", float(0));
  addParam("CutSVDClusterCharge", m_CutSVDClusterCharge,
           "minimum charge (in e-) to fill the cluster-hitmap histogram.", float(0));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("SVDExpReco"));
  addParam("additionalPlots", m_additionalPlots, "Flag to produce additional plots",
           bool(false));

  m_histoList = new TList();
}


SVDDQMExpressRecoModule::~SVDDQMExpressRecoModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMExpressRecoModule::defineHisto()
{
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
  int nSVDChips = gTools->getTotalSVDChips();

  // number of events counter
  m_nEvents = new TH1F("SVDDQM_nEvents", "SVD Number of Events", 1, -0.5, 0.5);
  m_nEvents->GetYaxis()->SetTitle("N events");
  m_histoList->Add(m_nEvents);

  // Create basic histograms:
  // basic counters per sensor:
  m_hitMapCountsU = new TH1F("SVDDQM_StripCountsU", "SVD Integrated Number of ZS5 Fired U-Strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsU);
  m_hitMapCountsV = new TH1F("SVDDQM_StripCountsV", "SVD Integrated Number of ZS5 Fired V-Strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsV);
  m_hitMapClCountsU = new TH1F("SVDDQM_ClusterCountsU", "SVD Integrated Number of U-Clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsU);
  m_hitMapClCountsV = new TH1F("SVDDQM_ClusterCountsV", "SVD Integrated Number of V-Clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsV);
  for (int i = 0; i < nSVDSensors; i++) {
    VxdID id = gTools->getSensorIDFromSVDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  // basic counters per chip:
  m_hitMapCountsChip = new TH1F("SVDDQM_StripCountsChip", "SVD Integrated Number of ZS5 Fired Strips per chip",
                                nSVDChips, 0, nSVDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsChip);
  m_hitMapClCountsChip = new TH1F("SVDDQM_ClusterCountsChip", "SVD Integrated Number of Clusters per chip",
                                  nSVDChips, 0, nSVDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsChip);

  if (m_additionalPlots) {
    m_firedU = new TH1F*[nSVDSensors];
    m_firedV = new TH1F*[nSVDSensors];
    m_clustersU = new TH1F*[nSVDSensors];
    m_clustersV = new TH1F*[nSVDSensors];
    m_stripSignalU = new TH1F*[nSVDSensors];
    m_stripSignalV = new TH1F*[nSVDSensors];
  }

  m_clusterChargeU = new TH1F*[nSVDSensors];
  m_clusterChargeV = new TH1F*[nSVDSensors];
  m_clusterSNRU = new TH1F*[nSVDSensors];
  m_clusterSNRV = new TH1F*[nSVDSensors];

  m_stripCountU = new TH1F*[nSVDSensors];
  m_stripCountV = new TH1F*[nSVDSensors];
  m_strip3CountU = new TH1F*[nSVDSensors];
  m_strip3CountV = new TH1F*[nSVDSensors];
  m_strip6CountU = new TH1F*[nSVDSensors];
  m_strip6CountV = new TH1F*[nSVDSensors];

  m_stripCountGroupId0U = new TH1F*[nSVDSensors];
  m_stripCountGroupId0V = new TH1F*[nSVDSensors];

  m_onlineZSstripCountU = new TH1F*[nSVDSensors];
  m_onlineZSstripCountV = new TH1F*[nSVDSensors];
  m_onlineZSstrip3CountU = new TH1F*[nSVDSensors];
  m_onlineZSstrip3CountV = new TH1F*[nSVDSensors];
  m_onlineZSstrip6CountU = new TH1F*[nSVDSensors];
  m_onlineZSstrip6CountV = new TH1F*[nSVDSensors];

  m_clusterSizeU = new TH1F*[nSVDSensors];
  m_clusterSizeV = new TH1F*[nSVDSensors];
  m_clusterTimeU = new TH1F*[nSVDSensors];
  m_clusterTimeV = new TH1F*[nSVDSensors];

  int ChargeBins = 80;
  float ChargeMax = 80;
  int SNRBins = 50;
  float SNRMax = 100;
  int TimeBins = 300;
  float TimeMin = -150;
  float TimeMax = 150;

  int GroupIdBins = 21;
  float GroupIdMin = -1.5;
  float GroupIdMax = 19.5;

  int MaxBinBins = 6;
  int MaxBinMax = 6;

  TString refFrame = "in FTSW reference";
  if (m_desynchSVDTime)
    refFrame = "in SVD reference";


  //----------------------------------------------------------------
  // Charge of clusters for all sensors
  //----------------------------------------------------------------
  string name = str(format("SVDDQM_ClusterChargeUAll"));
  string title = str(format("SVD U-Cluster Charge for all sensors"));
  m_clusterChargeUAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeUAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeUAll);
  name = str(format("SVDDQM_ClusterChargeVAll"));
  title = str(format("SVD V-Cluster Charge for all sensors"));
  m_clusterChargeVAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeVAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeVAll);
  //----------------------------------------------------------------
  // Charge of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterChargeU3"));
  title = str(format("SVD U-Cluster Charge for layer 3 sensors"));
  m_clusterChargeU3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeU3);
  name = str(format("SVDDQM_ClusterChargeV3"));
  title = str(format("SVD V-Cluster Charge for layer 3 sensors"));
  m_clusterChargeV3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeV3);

  name = str(format("SVDDQM_ClusterChargeU456"));
  title = str(format("SVD U-Cluster Charge for layers 4,5,6 sensors"));
  m_clusterChargeU456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeU456);

  name = str(format("SVDDQM_ClusterChargeV456"));
  title = str(format("SVD V-Cluster Charge for layers 4,5,6 sensors"));
  m_clusterChargeV456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeV456);

  //----------------------------------------------------------------
  // SNR of clusters for all sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterSNRUAll"));
  title = str(format("SVD U-Cluster SNR for all sensors"));
  m_clusterSNRUAll = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);  // max = ~ 60
  m_clusterSNRUAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRUAll);
  name = str(format("SVDDQM_ClusterSNRVAll"));
  title = str(format("SVD V-Cluster SNR for all sensors"));
  m_clusterSNRVAll = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRVAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRVAll);
  //----------------------------------------------------------------
  // SNR of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterSNRU3"));
  title = str(format("SVD U-Cluster SNR for layer 3 sensors"));
  m_clusterSNRU3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRU3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRU3);
  name = str(format("SVDDQM_ClusterSNRV3"));
  title = str(format("SVD V-Cluster SNR for layer 3 sensors"));
  m_clusterSNRV3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRV3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRV3);

  name = str(format("SVDDQM_ClusterSNRU456"));
  title = str(format("SVD U-Cluster SNR for layers 4,5,6 sensors"));
  m_clusterSNRU456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRU456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRU456);
  name = str(format("SVDDQM_ClusterSNRV456"));
  title = str(format("SVD V-Cluster SNR for layers 4,5,6 sensors"));
  m_clusterSNRV456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRV456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRV456);
  //----------------------------------------------------------------
  // Cluster time distribution for all sensors
  //----------------------------------------------------------------
  TString Name = "SVDDQM_ClusterTimeUAll";
  TString Title = Form("SVD U-Cluster Time %s for all sensors", refFrame.Data());
  m_clusterTimeUAll = new TH1F(Name.Data(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeUAll->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeUAll);
  Name = "SVDDQM_ClusterTimeVAll";
  Title = Form("SVD V-Cluster Time %s for all sensors", refFrame.Data());
  m_clusterTimeVAll = new TH1F(Name.Data(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeVAll->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeVAll);
  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  Name = "SVDDQM_ClusterTimeU3";
  Title = Form("SVD U-Cluster Time %s for layer 3 sensors", refFrame.Data());
  m_clusterTimeU3 = new TH1F(Name.Data(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeU3);
  name = str(format("SVDDQM_ClusterTimeV3"));
  Title = Form("SVD V-Cluster Time %s for layer 3 sensors", refFrame.Data());
  m_clusterTimeV3 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeV3);

  name = str(format("SVDDQM_ClusterTimeU456"));
  Title = Form("SVD U-Cluster Time %s for layers 4,5,6 sensors", refFrame.Data());
  m_clusterTimeU456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeU456);
  name = str(format("SVDDQM_ClusterTimeV456"));
  Title = Form("SVD V-Cluster Time %s for layers 4,5,6 sensors", refFrame.Data());
  m_clusterTimeV456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeV456);

  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors for 3 samples
  //----------------------------------------------------------------
  Name = "SVDDQM_Cluster3TimeU3";
  Title = Form("SVD U-Cluster Time %s for layer 3 sensors for 3 samples", refFrame.Data());
  m_cluster3TimeU3 = new TH1F(Name.Data(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster3TimeU3->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster3TimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster3TimeU3);
  name = str(format("SVDDQM_Cluster3TimeV3"));
  Title = Form("SVD V-Cluster Time %s for layer 3 sensors for 3 samples", refFrame.Data());
  m_cluster3TimeV3 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster3TimeV3->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster3TimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster3TimeV3);

  name = str(format("SVDDQM_Cluster3TimeU456"));
  Title = Form("SVD U-Cluster Time %s for layers 4,5,6 sensors for 3 samples", refFrame.Data());
  m_cluster3TimeU456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster3TimeU456->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster3TimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster3TimeU456);
  name = str(format("SVDDQM_Cluster3TimeV456"));
  Title = Form("SVD V-Cluster Time %s for layers 4,5,6 sensors for 3 samples", refFrame.Data());
  m_cluster3TimeV456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster3TimeV456->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster3TimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster3TimeV456);

  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors for 6 samples
  //----------------------------------------------------------------
  Name = "SVDDQM_Cluster6TimeU3";
  Title = Form("SVD U-Cluster Time %s for layer 3 sensors for 6 samples", refFrame.Data());
  m_cluster6TimeU3 = new TH1F(Name.Data(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster6TimeU3->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster6TimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster6TimeU3);
  name = str(format("SVDDQM_Cluster6TimeV3"));
  Title = Form("SVD V-Cluster Time %s for layer 3 sensors for 6 samples", refFrame.Data());
  m_cluster6TimeV3 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster6TimeV3->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster6TimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster6TimeV3);

  name = str(format("SVDDQM_Cluster6TimeU456"));
  Title = Form("SVD U-Cluster Time %s for layers 4,5,6 sensors for 6 samples", refFrame.Data());
  m_cluster6TimeU456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster6TimeU456->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster6TimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster6TimeU456);
  name = str(format("SVDDQM_Cluster6TimeV456"));
  Title = Form("SVD V-Cluster Time %s for layers 4,5,6 sensors for 6 samples", refFrame.Data());
  m_cluster6TimeV456 = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
  m_cluster6TimeV456->GetXaxis()->SetTitle("cluster time (ns)");
  m_cluster6TimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_cluster6TimeV456);


  //----------------------------------------------------------------
  // Cluster time group Id vs cluster time for U/V sensors
  //----------------------------------------------------------------
  Name = "SVDDQM_ClusterTimeGroupIdU";
  Title = Form("SVD cluster Time Group Id %s vs cluster time for U/P Side", refFrame.Data());
  m_clusterTimeGroupIdU = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTimeGroupIdU->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeGroupIdU->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTimeGroupIdU);
  Name = "SVDDQM_ClusterTimeGroupIdV";
  Title =  Form("SVD cluster Time Group Id %s vs cluster time for V/N Side", refFrame.Data());
  m_clusterTimeGroupIdV = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTimeGroupIdV->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeGroupIdV->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTimeGroupIdV);

  //----------------------------------------------------------------
  // Cluster time group Id vs cluster time for U/V sensors for coarse and fine trigger
  //----------------------------------------------------------------
  Name = "SVDDQM_cluster6TimeGroupIdU";
  Title = Form("SVD cluster Time Group Id %s vs cluster time for U/P Side for coarse trigger", refFrame.Data());
  m_clusterTime6GroupIdU = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTime6GroupIdU->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTime6GroupIdU->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTime6GroupIdU);
  Name = "SVDDQM_cluster6TimeGroupIdV";
  Title =  Form("SVD cluster Time Group Id %s vs cluster time for V/N Side for coarse trigger", refFrame.Data());
  m_clusterTime6GroupIdV = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTime6GroupIdV->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTime6GroupIdV->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTime6GroupIdV);

  Name = "SVDDQM_cluster3TimeGroupIdU";
  Title = Form("SVD cluster Time Group Id %s vs cluster time for U/P Side for fine trigger", refFrame.Data());
  m_clusterTime3GroupIdU = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTime3GroupIdU->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTime3GroupIdU->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTime3GroupIdU);
  Name = "SVDDQM_cluster3TimeGroupIdV";
  Title =  Form("SVD cluster Time Group Id %s vs cluster time for V/N Side for fine trigger", refFrame.Data());
  m_clusterTime3GroupIdV = new TH2F(Name.Data(), Title.Data(), TimeBins / 2, TimeMin, TimeMax, GroupIdBins, GroupIdMin, GroupIdMax);
  m_clusterTime3GroupIdV->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTime3GroupIdV->GetYaxis()->SetTitle("cluster group id");
  m_histoList->Add(m_clusterTime3GroupIdV);

  //----------------------------------------------------------------
  // MaxBin of strips for all sensors (offline ZS)
  //----------------------------------------------------------------
  name = str(format("SVDDQM_StripMaxBinUAll"));
  title = str(format("SVD U-Strip MaxBin for all sensors"));
  m_stripMaxBinUAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinUAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinUAll);
  name = str(format("SVDDQM_StripMaxBinVAll"));
  title = str(format("SVD V-Strip MaxBin for all sensors"));
  m_stripMaxBinVAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinVAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinVAll);

  name = str(format("SVDDQM_StripMaxBinU3"));
  title = str(format("SVD U-Strip MaxBin for layer 3 sensors"));
  m_stripMaxBinU3 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinU3);
  name = str(format("SVDDQM_StripMaxBinV3"));
  title = str(format("SVD V-Strip MaxBin for layer 3 sensors"));
  m_stripMaxBinV3 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinV3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinV3);

  name = str(format("SVDDQM_StripMaxBinU6"));
  title = str(format("SVD U-Strip MaxBin for layer 6 sensors"));
  m_stripMaxBinU6 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU6->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU6->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinU6);
  name = str(format("SVDDQM_StripMaxBinV6"));
  title = str(format("SVD V-Strip MaxBin for layer 6 sensors"));
  m_stripMaxBinV6 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinV6->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinV6->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinV6);

  for (int i = 0; i < nSVDSensors; i++) {
    VxdID id = gTools->getSensorIDFromSVDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);

    if (m_additionalPlots) {
      //----------------------------------------------------------------
      // Number of fired strips per sensor
      //----------------------------------------------------------------
      name = str(format("SVDDQM_%1%_FiredU") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of Fired U-Strips") % sensorDescr);
      m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedU[i]->GetXaxis()->SetTitle("# fired strips");
      m_firedU[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_firedU[i]);
      name = str(format("SVDDQM_%1%_FiredV") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of Fired V-Strips") % sensorDescr);
      m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedV[i]->GetXaxis()->SetTitle("# fired strips");
      m_firedV[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_firedV[i]);
      //----------------------------------------------------------------
      // Number of clusters per sensor
      //----------------------------------------------------------------
      name = str(format("SVDDQM_%1%_ClustersU") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of U-Clusters") % sensorDescr);
      m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersU[i]->GetXaxis()->SetTitle("# clusters");
      m_clustersU[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustersU[i]);
      name = str(format("SVDDQM_%1%_ClustersV") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of V-Clusters") % sensorDescr);
      m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersV[i]->GetXaxis()->SetTitle("# clusters");
      m_clustersV[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_clustersV[i]);
      //----------------------------------------------------------------
      // Charge of strips
      //----------------------------------------------------------------
      name = str(format("SVDDQM_%1%_ADCStripU") % sensorDescr);
      title = str(format("SVD Sensor %1% U-Strip signal in ADC Counts, all 6 APV samples") % sensorDescr);
      m_stripSignalU[i] = new TH1F(name.c_str(), title.c_str(), 256, -0.5, 255.5);
      m_stripSignalU[i]->GetXaxis()->SetTitle("signal ADC");
      m_stripSignalU[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_stripSignalU[i]);
      name = str(format("SVDDQM_%1%_ADCStripV") % sensorDescr);
      title = str(format("SVD Sensor %1% V-Strip signal in ADC Counts, all 6 APV samples") % sensorDescr);
      m_stripSignalV[i] = new TH1F(name.c_str(), title.c_str(), 256, -0.5, 255.5);
      m_stripSignalV[i]->GetXaxis()->SetTitle("signal ADC");
      m_stripSignalV[i]->GetYaxis()->SetTitle("count");
      m_histoList->Add(m_stripSignalV[i]);
    }

    //----------------------------------------------------------------
    // Charge of clusters
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterChargeU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster Charge") % sensorDescr);
    m_clusterChargeU[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterChargeU[i]);
    name = str(format("SVDDQM_%1%_ClusterChargeV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster Charge") % sensorDescr);
    m_clusterChargeV[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterChargeV[i]);
    //----------------------------------------------------------------
    // SNR of clusters
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterSNRU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster SNR") % sensorDescr);
    m_clusterSNRU[i] = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
    m_clusterSNRU[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSNRU[i]);
    name = str(format("SVDDQM_%1%_ClusterSNRV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster SNR") % sensorDescr);
    m_clusterSNRV[i] = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
    m_clusterSNRV[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSNRV[i]);

    //----------------------------------------------------------------
    // Strips Counts
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_StripCountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired U-Strip vs Strip Number") % sensorDescr);
    m_stripCountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountU[i]);
    name = str(format("SVDDQM_%1%_StripCountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired V-Strip vs Strip Number") % sensorDescr);
    m_stripCountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountV[i]);
    //----------------------------------------------------------------
    // Strips Counts with online ZS
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_OnlineZSStripCountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired U-Strip vs Strip Number") % sensorDescr);
    m_onlineZSstripCountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstripCountU[i]);
    name = str(format("SVDDQM_%1%_OnlineZSStripCountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired V-Strip vs Strip Number") % sensorDescr);
    m_onlineZSstripCountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstripCountV[i]);

    //----------------------------------------------------------------
    // Strips Counts for 3 samples
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_Strip3CountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired U-Strip vs Strip Number for 3 samples") % sensorDescr);
    m_strip3CountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_strip3CountU[i]->GetXaxis()->SetTitle("cellID");
    m_strip3CountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_strip3CountU[i]);
    name = str(format("SVDDQM_%1%_Strip3CountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired V-Strip vs Strip Number for 3 samples") % sensorDescr);
    m_strip3CountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_strip3CountV[i]->GetXaxis()->SetTitle("cellID");
    m_strip3CountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_strip3CountV[i]);
    //----------------------------------------------------------------
    // Strips Counts with online ZS for 3 samples
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_OnlineZSStrip3CountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired U-Strip vs Strip Number for 3 samples") % sensorDescr);
    m_onlineZSstrip3CountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstrip3CountU[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstrip3CountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstrip3CountU[i]);
    name = str(format("SVDDQM_%1%_OnlineZSStrip3CountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired V-Strip vs Strip Number for 3 samples") % sensorDescr);
    m_onlineZSstrip3CountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstrip3CountV[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstrip3CountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstrip3CountV[i]);


    //----------------------------------------------------------------
    // Strips Counts for 6 samples
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_Strip6CountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired U-Strip vs Strip Number for 6 samples") % sensorDescr);
    m_strip6CountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_strip6CountU[i]->GetXaxis()->SetTitle("cellID");
    m_strip6CountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_strip6CountU[i]);
    name = str(format("SVDDQM_%1%_Strip6CountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired V-Strip vs Strip Number for 6 samples") % sensorDescr);
    m_strip6CountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_strip6CountV[i]->GetXaxis()->SetTitle("cellID");
    m_strip6CountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_strip6CountV[i]);
    //----------------------------------------------------------------
    // Strips Counts with online ZS for 6 samples
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_OnlineZSStrip6CountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired U-Strip vs Strip Number for 6 samples") % sensorDescr);
    m_onlineZSstrip6CountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstrip6CountU[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstrip6CountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstrip6CountU[i]);
    name = str(format("SVDDQM_%1%_OnlineZSStrip6CountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired V-Strip vs Strip Number for 6 samples") % sensorDescr);
    m_onlineZSstrip6CountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstrip6CountV[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstrip6CountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstrip6CountV[i]);


    //----------------------------------------------------------------
    // Strips Counts for cluster time group id = 0
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_StripCountGroupId0U") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated NumberFired U-Strip for group Id = 0 vs Strip Number") % sensorDescr);
    m_stripCountGroupId0U[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountGroupId0U[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountGroupId0U[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountGroupId0U[i]);
    name = str(format("SVDDQM_%1%_StripCountGroupId0V") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of Fired V-Strip for group Id = 0 vs Strip Number") % sensorDescr);
    m_stripCountGroupId0V[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountGroupId0V[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountGroupId0V[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountGroupId0V[i]);

    //----------------------------------------------------------------
    // Cluster size distribution
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster Size") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 9, 1, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSizeU[i]);
    name = str(format("SVDDQM_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster Size") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 9, 1, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSizeV[i]);
    //----------------------------------------------------------------
    // Cluster time distribution
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterTimeU") % sensorDescr);
    Title = Form("SVD Sensor %s U-Cluster Time %s", sensorDescr.c_str(), refFrame.Data());
    m_clusterTimeU[i] = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("cluster time (ns)");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterTimeU[i]);
    name = str(format("SVDDQM_%1%_ClusterTimeV") % sensorDescr);
    Title = Form("SVD Sensor %s V-Cluster Time %s", sensorDescr.c_str(), refFrame.Data());
    m_clusterTimeV[i] = new TH1F(name.c_str(), Title.Data(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeV[i]->GetXaxis()->SetTitle("cluster time (ns)");
    m_clusterTimeV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterTimeV[i]);
  }

  for (int i = 0; i < nSVDChips; i++) {
    VxdID id = gTools->getChipIDFromSVDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    int iChip = gTools->getSVDChipNumber(id);
    int IsU = gTools->isSVDSideU(id);
    TString AxisTicks = Form("%i_%i_%i_u%i", iLayer, iLadder, iSensor, iChip);
    if (!IsU)
      AxisTicks = Form("%i_%i_%i_v%i", iLayer, iLadder, iSensor, iChip);
    m_hitMapCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }



  //----------------------------------------------------------------
  // Additional histograms for out of ExpressReco
  //----------------------------------------------------------------

  if (m_ShowAllHistos == 1) {
    TDirectory* dirShowAll = nullptr;
    dirShowAll = oldDir->mkdir("SVDDQMAll");
    dirShowAll->cd();

    m_hitMapU = new TH2F*[nSVDSensors];
    m_hitMapV = new TH2F*[nSVDSensors];
    m_hitMapUCl = new TH1F*[nSVDSensors];
    m_hitMapVCl = new TH1F*[nSVDSensors];
    for (int i = 0; i < nSVDSensors; i++) {
      VxdID id = gTools->getSensorIDFromSVDIndex(i);
      int iLayer = id.getLayerNumber();
      int iLadder = id.getLadderNumber();
      int iSensor = id.getSensorNumber();
      VxdID sensorID(iLayer, iLadder, iSensor);
      SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
      //----------------------------------------------------------------
      // Hitmaps: Number of strips by coordinate
      //----------------------------------------------------------------
      name = str(format("SVD_%1%_StripHitmapU") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in U") % sensorDescr);
      int nStrips = SensorInfo.getUCells();
      m_hitMapU[i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, SVDShaperDigit::c_nAPVSamples, 0,
                              SVDShaperDigit::c_nAPVSamples);
      m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapU[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapU[i]->GetZaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapU[i]);
      name = str(format("SVD_%1%_StripHitmapV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapV[i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, SVDShaperDigit::c_nAPVSamples, 0,
                              SVDShaperDigit::c_nAPVSamples);
      m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapV[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapV[i]->GetZaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapV[i]);
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate
      //----------------------------------------------------------------
      name = str(format("SVD_%1%_HitmapClstU") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in U") % sensorDescr);
      nStrips = SensorInfo.getUCells();
      m_hitMapUCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapUCl[i]);
      name = str(format("SVD_%1%_HitmapClstV") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapVCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapVCl[i]);
    }
  }

  oldDir->cd();
}


void SVDDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() != 0) {
    //Register collections
    StoreArray<SVDShaperDigit> storeNoZSSVDShaperDigits(m_storeNoZSSVDShaperDigitsName);
    StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
    StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);

    storeSVDClusters.isOptional();
    storeSVDShaperDigits.isOptional();
    m_svdEventInfo.isOptional();
    storeNoZSSVDShaperDigits.isOptional();

    //Store names to speed up creation later
    m_storeSVDShaperDigitsName = storeSVDShaperDigits.getName();
  }

  m_objTrgSummary.isOptional();
}

void SVDDQMExpressRecoModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;


  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();

  // Add experiment and run number to the title of selected histograms (CR shifter plots)
  TString runID = TString::Format(" ~ Exp%d Run%d", m_expNumber, m_runNumber);
  TObject* obj;
  TIter nextH(m_histoList);
  while ((obj = nextH()))
    if (obj->InheritsFrom("TH1")) {

      TString tmp = (TString)obj->GetTitle();
      Int_t pos = tmp.Last('~');
      if (pos == -1) pos = tmp.Length() + 2;

      TString title = tmp(0, pos - 2);
      ((TH1F*)obj)->SetTitle(title + runID);
      ((TH1F*)obj)->Reset();
    }
}

void SVDDQMExpressRecoModule::event()
{


  //check HLT decision and increase number of events only if the event has been accepted

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }
  m_nEvents->Fill(0);

  int nSamples = 0;
  if (m_svdEventInfo.isValid())
    nSamples = m_svdEventInfo->getNSamples();
  else
    return;

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;


  const StoreArray<SVDShaperDigit> storeNoZSSVDShaperDigits(m_storeNoZSSVDShaperDigitsName);
  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);

  if (!storeSVDShaperDigits.isValid() || !storeSVDShaperDigits.getEntries()) {
    return;
  }

  int firstSVDLayer = gTools->getFirstSVDLayer();
  int lastSVDLayer = gTools->getLastSVDLayer();
  int nSVDSensors = gTools->getNumberOfSVDSensors();

  // Fired strips offline ZS
  vector< set<int> > uStrips(nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(nSVDSensors);
  for (const SVDShaperDigit& digitIn : storeSVDShaperDigits) {
    int iLayer = digitIn.getSensorID().getLayerNumber();
    if ((iLayer < firstSVDLayer) || (iLayer > lastSVDLayer)) continue;
    int iLadder = digitIn.getSensorID().getLadderNumber();
    int iSensor = digitIn.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getSVDSensorIndex(sensorID);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (digitIn.isUStrip()) {

      //fill strip count first
      if (m_stripCountU[index] != nullptr) m_stripCountU[index]->Fill(digitIn.getCellID());

      if (nSamples == 3) {
        if (m_strip3CountU[index] != nullptr) m_strip3CountU[index]->Fill(digitIn.getCellID());
      } else {
        if (m_strip6CountU[index] != nullptr) m_strip6CountU[index]->Fill(digitIn.getCellID());
      }

      //fill max bin
      if (m_stripMaxBinUAll != nullptr) m_stripMaxBinUAll->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 3)
        if (m_stripMaxBinU3 != nullptr) m_stripMaxBinU3->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 6)
        if (m_stripMaxBinU6 != nullptr) m_stripMaxBinU6->Fill(digitIn.getMaxTimeBin());

      uStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip()) + 1;
      int indexChip = gTools->getSVDChipIndex(sensorID, kTRUE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      int isSample = 0;
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_additionalPlots)
          if (m_stripSignalU[index] != nullptr) m_stripSignalU[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          isSample = 1;
          if (m_ShowAllHistos == 1) {
            if (m_hitMapU[index] != nullptr) m_hitMapU[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
      if (isSample) {
        if (m_hitMapCountsU != nullptr) m_hitMapCountsU->Fill(index);
        if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Fill(indexChip);
      }
    } else {
      //fill strip count first
      if (m_stripCountV[index] != nullptr) m_stripCountV[index]->Fill(digitIn.getCellID());

      if (nSamples == 3) {
        if (m_strip3CountV[index] != nullptr) m_strip3CountV[index]->Fill(digitIn.getCellID());
      } else {
        if (m_strip6CountV[index] != nullptr) m_strip6CountV[index]->Fill(digitIn.getCellID());
      }

      //fill max bin
      if (m_stripMaxBinVAll != nullptr) m_stripMaxBinVAll->Fill(digitIn.getMaxTimeBin());

      if (iLayer == 3)
        if (m_stripMaxBinV3 != nullptr) m_stripMaxBinV3->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 6)
        if (m_stripMaxBinV6 != nullptr) m_stripMaxBinV6->Fill(digitIn.getMaxTimeBin());

      vStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip()) + 1;
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      int isSample = 0;
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_additionalPlots)
          if (m_stripSignalV[index] != nullptr) m_stripSignalV[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          isSample = 1;
          if (m_ShowAllHistos == 1) {
            if (m_hitMapV[index] != nullptr) m_hitMapV[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
      if (isSample) {
        if (m_hitMapCountsV != nullptr) m_hitMapCountsV->Fill(index);
        if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Fill(indexChip);
      }
    }
  }
  if (m_additionalPlots) {
    for (int i = 0; i < nSVDSensors; i++) {
      if ((m_firedU[i] != nullptr) && (uStrips[i].size() > 0))
        m_firedU[i]->Fill(uStrips[i].size());
      if ((m_firedV[i] != nullptr) && (vStrips[i].size() > 0))
        m_firedV[i]->Fill(vStrips[i].size());
    }
  }

  // Fired strips ONLINE ZS
  if (storeNoZSSVDShaperDigits.isValid())
    for (const SVDShaperDigit& digitIn : storeNoZSSVDShaperDigits) {
      int iLayer = digitIn.getSensorID().getLayerNumber();
      if ((iLayer < firstSVDLayer) || (iLayer > lastSVDLayer)) continue;
      int iLadder = digitIn.getSensorID().getLadderNumber();
      int iSensor = digitIn.getSensorID().getSensorNumber();
      VxdID sensorID(iLayer, iLadder, iSensor);
      int index = gTools->getSVDSensorIndex(sensorID);
      SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      if (digitIn.isUStrip()) {
        if (m_onlineZSstripCountU[index] != nullptr) m_onlineZSstripCountU[index]->Fill(digitIn.getCellID());
        if (nSamples == 3) {
          if (m_onlineZSstrip3CountU[index] != nullptr) m_onlineZSstrip3CountU[index]->Fill(digitIn.getCellID());
        } else {
          if (m_onlineZSstrip6CountU[index] != nullptr) m_onlineZSstrip6CountU[index]->Fill(digitIn.getCellID());
        }
      } else {
        if (m_onlineZSstripCountV[index] != nullptr) m_onlineZSstripCountV[index]->Fill(digitIn.getCellID());
        if (nSamples == 3) {
          if (m_onlineZSstrip3CountV[index] != nullptr) m_onlineZSstrip3CountV[index]->Fill(digitIn.getCellID());
        } else {
          if (m_onlineZSstrip6CountV[index] != nullptr) m_onlineZSstrip6CountV[index]->Fill(digitIn.getCellID());
        }
      }
    }

  vector< set<int> > countsU(nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > countsV(nSVDSensors);
  // Hitmaps, Charge, Seed, Size, Time, ...
  for (const SVDCluster& cluster : storeSVDClusters) {
    if (cluster.getCharge() < m_CutSVDClusterCharge) continue;
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < firstSVDLayer) || (iLayer > lastSVDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getSVDSensorIndex(sensorID);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));

    float time = cluster.getClsTime();
    if (m_desynchSVDTime && m_svdEventInfo.isValid())
      time = time - m_svdEventInfo->getSVD2FTSWTimeShift(cluster.getFirstFrame());

    vector<int> vec = cluster.getTimeGroupId();
    auto minElement = min_element(vec.begin(), vec.end());
    int groupId  = -1;
    if (vec.size() > 0) {
      groupId = *minElement;


      if (cluster.isUCluster()) {
        if (m_clusterTimeGroupIdU != nullptr) m_clusterTimeGroupIdU->Fill(time, groupId);
        if (m_objTrgSummary.isValid()) {
          int trgQuality = m_objTrgSummary->getTimQuality();
          if (trgQuality == 1)
            if (m_clusterTime6GroupIdU != nullptr) m_clusterTime6GroupIdU->Fill(time, groupId);
          if (trgQuality == 2)
            if (m_clusterTime3GroupIdU != nullptr) m_clusterTime3GroupIdU->Fill(time, groupId);
        }


      } else {
        if (m_clusterTimeGroupIdV != nullptr) m_clusterTimeGroupIdV->Fill(time, groupId);
        if (m_objTrgSummary.isValid()) {
          int trgQuality = m_objTrgSummary->getTimQuality();
          if (trgQuality == 1)
            if (m_clusterTime6GroupIdV != nullptr) m_clusterTime6GroupIdV->Fill(time, groupId);
          if (trgQuality == 2)
            if (m_clusterTime3GroupIdV != nullptr) m_clusterTime3GroupIdV->Fill(time, groupId);
        }
      }
    }

    if (cluster.isUCluster()) {
      countsU.at(index).insert(SensorInfo.getUCellID(cluster.getPosition()));
      int indexChip = gTools->getSVDChipIndex(sensorID, kTRUE,
                                              (int)(SensorInfo.getUCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()) + 1);
      if (m_hitMapClCountsU != nullptr) m_hitMapClCountsU->Fill(index);
      if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeU[index] != nullptr) m_clusterChargeU[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRU[index] != nullptr) m_clusterSNRU[index]->Fill(cluster.getSNR());
      if (m_clusterChargeUAll != nullptr) m_clusterChargeUAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRUAll != nullptr) m_clusterSNRUAll->Fill(cluster.getSNR());
      if (m_clusterSizeU[index] != nullptr) m_clusterSizeU[index]->Fill(cluster.getSize());
      if (m_clusterTimeU[index] != nullptr) m_clusterTimeU[index]->Fill(time);
      if (m_clusterTimeUAll != nullptr) m_clusterTimeUAll->Fill(time);
      if (iLayer == 3) {
        if (m_clusterChargeU3 != nullptr) m_clusterChargeU3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRU3 != nullptr) m_clusterSNRU3->Fill(cluster.getSNR());
        if (m_clusterTimeU3 != nullptr) m_clusterTimeU3->Fill(time);
        if (nSamples == 3) {
          if (m_cluster3TimeU3 != nullptr) m_cluster3TimeU3->Fill(time);
        } else {
          if (m_cluster6TimeU3 != nullptr) m_cluster6TimeU3->Fill(time);
        }
      } else {
        if (m_clusterChargeU456 != nullptr) m_clusterChargeU456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRU456 != nullptr) m_clusterSNRU456->Fill(cluster.getSNR());
        if (m_clusterTimeU456 != nullptr) m_clusterTimeU456->Fill(time);
        if (nSamples == 3) {
          if (m_cluster3TimeU456 != nullptr) m_cluster3TimeU456->Fill(time);
        } else {
          if (m_cluster6TimeU456 != nullptr) m_cluster6TimeU456->Fill(time);
        }
      }

      if (m_ShowAllHistos == 1)
        if (m_hitMapUCl[index] != nullptr) m_hitMapUCl[index]->Fill(SensorInfo.getUCellID(cluster.getPosition()));

      // groupId for U side
      if (groupId == 0) {
        for (const SVDShaperDigit& digitIn : cluster.getRelationsTo<SVDShaperDigit>(m_storeSVDShaperDigitsName)) {
          if (m_stripCountGroupId0U != nullptr) m_stripCountGroupId0U[index]->Fill(digitIn.getCellID());
        }
      }
    } else {
      countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE,
                                              (int)(SensorInfo.getVCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()) + 1);
      if (m_hitMapClCountsV != nullptr) m_hitMapClCountsV->Fill(index);
      if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeV[index] != nullptr) m_clusterChargeV[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRV[index] != nullptr) m_clusterSNRV[index]->Fill(cluster.getSNR());
      if (m_clusterChargeVAll != nullptr) m_clusterChargeVAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRVAll != nullptr) m_clusterSNRVAll->Fill(cluster.getSNR());
      if (m_clusterSizeV[index] != nullptr) m_clusterSizeV[index]->Fill(cluster.getSize());
      if (m_clusterTimeV[index] != nullptr) m_clusterTimeV[index]->Fill(time);
      if (m_clusterTimeVAll != nullptr) m_clusterTimeVAll->Fill(time);
      if (iLayer == 3) {
        if (m_clusterChargeV3 != nullptr) m_clusterChargeV3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRV3 != nullptr) m_clusterSNRV3->Fill(cluster.getSNR());
        if (m_clusterTimeV3 != nullptr) m_clusterTimeV3->Fill(time);
        if (nSamples == 3) {
          if (m_cluster3TimeV3 != nullptr) m_cluster3TimeV3->Fill(time);
        } else {
          if (m_cluster6TimeV3 != nullptr) m_cluster6TimeV3->Fill(time);
        }
      } else {
        if (m_clusterChargeV456 != nullptr) m_clusterChargeV456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRV456 != nullptr) m_clusterSNRV456->Fill(cluster.getSNR());
        if (m_clusterTimeV456 != nullptr) m_clusterTimeV456->Fill(time);
        if (nSamples == 3) {
          if (m_cluster3TimeV456 != nullptr) m_cluster3TimeV456->Fill(time);
        } else {
          if (m_cluster6TimeV456 != nullptr) m_cluster6TimeV456->Fill(time);
        }
      }
      if (m_ShowAllHistos == 1)
        if (m_hitMapVCl[index] != nullptr) m_hitMapVCl[index]->Fill(SensorInfo.getVCellID(cluster.getPosition()));

      // groupId for V side
      if (groupId == 0) {
        for (const SVDShaperDigit& digitIn : cluster.getRelationsTo<SVDShaperDigit>(m_storeSVDShaperDigitsName)) {
          if (m_stripCountGroupId0V != nullptr) m_stripCountGroupId0V[index]->Fill(digitIn.getCellID());
        }
      }
    }
  }
  if (m_additionalPlots) {
    for (int i = 0; i < nSVDSensors; i++) {
      if ((m_clustersU[i] != nullptr) && (countsU[i].size() > 0))
        m_clustersU[i]->Fill(countsU[i].size());
      if ((m_clustersV[i] != nullptr) && (countsV[i].size() > 0))
        m_clustersV[i]->Fill(countsV[i].size());
    }
  }
}


void SVDDQMExpressRecoModule::terminate()
{

  //  m_histoList->Delete();
  delete m_histoList;

}
