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
REG_MODULE(SVDDQMExpressReco)


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
    B2WARNING("Missing geometry for SVD, SVD-DQM is skiped.");
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
  TString title = TString::Format("SVD Number of Events ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_nEvents = new TH1F("SVDDQM_nEvents", title.Data(), 1, -0.5, 0.5);
  m_nEvents->GetYaxis()->SetTitle("N events");
  m_histoList->Add(m_nEvents);

  // Create basic histograms:
  // basic counters per sensor:
  title = TString::Format("SVD Integrated Number of ZS5 Fired U-Strips per sensor ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapCountsU = new TH1F("SVDDQM_StripCountsU", title.Data(),
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsU);

  title = TString::Format("SVD Integrated Number of ZS5 Fired V-Strips per sensor ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapCountsV = new TH1F("SVDDQM_StripCountsV", title.Data(),
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsV);

  title = TString::Format("SVD Integrated Number of U-Clusters per sensor ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapClCountsU = new TH1F("SVDDQM_ClusterCountsU", title.Data(),
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsU);

  title = TString::Format("SVD Integrated Number of V-Clusters per sensor ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapClCountsV = new TH1F("SVDDQM_ClusterCountsV", title.Data(),
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsV);
  for (int i = 0; i < nSVDSensors; i++) {
    VxdID id = gTools->getSensorIDFromSVDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = TString::Format("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  // basic counters per chip:
  title = TString::Format("SVD Integrated Number of ZS5 Fired Strips per chip ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapCountsChip = new TH1F("SVDDQM_StripCountsChip", title.Data(),
                                nSVDChips, 0, nSVDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapCountsChip);

  title = TString::Format("SVD Integrated Number of Clusters per chip ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_hitMapClCountsChip = new TH1F("SVDDQM_ClusterCountsChip", title.Data(),
                                  nSVDChips, 0, nSVDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  m_histoList->Add(m_hitMapClCountsChip);

  m_firedU = new TH1F*[nSVDSensors];
  m_firedV = new TH1F*[nSVDSensors];
  m_clustersU = new TH1F*[nSVDSensors];
  m_clustersV = new TH1F*[nSVDSensors];


  m_clusterChargeU = new TH1F*[nSVDSensors];
  m_clusterChargeV = new TH1F*[nSVDSensors];
  m_clusterSNRU = new TH1F*[nSVDSensors];
  m_clusterSNRV = new TH1F*[nSVDSensors];
  m_stripSignalU = new TH1F*[nSVDSensors];
  m_stripSignalV = new TH1F*[nSVDSensors];
  m_stripCountU = new TH1F*[nSVDSensors];
  m_stripCountV = new TH1F*[nSVDSensors];
  m_onlineZSstripCountU = new TH1F*[nSVDSensors];
  m_onlineZSstripCountV = new TH1F*[nSVDSensors];
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

  int MaxBinBins = 6;
  int MaxBinMax = 6;

  TString refFrame = "in FTSW reference";
  if (m_desynchSVDTime)
    refFrame = "in SVD reference";


  //----------------------------------------------------------------
  // Charge of clusters for all sensors
  //----------------------------------------------------------------
  TString name = "SVDDQM_ClusterChargeUAll";
  title = TString::Format("SVD U-Cluster Charge for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeUAll = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeUAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeUAll);

  name = "SVDDQM_ClusterChargeVAll";
  title = TString::Format("SVD V-Cluster Charge for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeVAll = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeVAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeVAll);
  //----------------------------------------------------------------
  // Charge of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = "SVDDQM_ClusterChargeU3";
  title = TString::Format("SVD U-Cluster Charge for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeU3 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeU3);

  name = "SVDDQM_ClusterChargeV3";
  title = TString::Format("SVD V-Cluster Charge for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeV3 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeV3);

  name = "SVDDQM_ClusterChargeU456";
  title = TString::Format("SVD U-Cluster Charge for layers 4,5,6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeU456 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeU456);

  name = "SVDDQM_ClusterChargeV456";
  title = TString::Format("SVD V-Cluster Charge for layers 4,5,6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterChargeV456 = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterChargeV456);

  //----------------------------------------------------------------
  // SNR of clusters for all sensors
  //----------------------------------------------------------------
  name = "SVDDQM_ClusterSNRUAll";
  title = TString::Format("SVD U-Cluster SNR for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRUAll = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);  // max = ~ 60
  m_clusterSNRUAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRUAll);

  name = "SVDDQM_ClusterSNRVAll";
  title = TString::Format("SVD V-Cluster SNR for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRVAll = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clusterSNRVAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRVAll);
  //----------------------------------------------------------------
  // SNR of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = "SVDDQM_ClusterSNRU3";
  title = TString::Format("SVD U-Cluster SNR for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRU3 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clusterSNRU3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRU3);

  name = "SVDDQM_ClusterSNRV3";
  title = TString::Format("SVD V-Cluster SNR for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRV3 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clusterSNRV3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRV3);

  name = "SVDDQM_ClusterSNRU456";
  title = TString::Format("SVD U-Cluster SNR for layers 4,5,6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRU456 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clusterSNRU456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRU456);

  name = "SVDDQM_ClusterSNRV456";
  title = TString::Format("SVD V-Cluster SNR for layers 4,5,6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_clusterSNRV456 = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
  m_clusterSNRV456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterSNRV456);
  //----------------------------------------------------------------
  // Cluster time distribution for all sensors
  //----------------------------------------------------------------
  name = "SVDDQM_ClusterTimeUAll";
  title = TString::Format("SVD U-Cluster Time %s for all sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeUAll = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeUAll->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeUAll);

  name = "SVDDQM_ClusterTimeVAll";
  title = TString::Format("SVD V-Cluster Time %s for all sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeVAll = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeVAll->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeVAll);
  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = "SVDDQM_ClusterTimeU3";
  title = TString::Format("SVD U-Cluster Time %s for layer 3 sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeU3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeU3);

  name = "SVDDQM_ClusterTimeV3";
  title = TString::Format("SVD V-Cluster Time %s for layer 3 sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeV3 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV3->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeV3);

  name = "SVDDQM_ClusterTimeU456";
  title = TString::Format("SVD U-Cluster Time %s for layers 4,5,6 sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeU456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeU456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeU456);

  name = "SVDDQM_ClusterTimeV456";
  title = TString::Format("SVD V-Cluster Time %s for layers 4,5,6 sensors ~ Exp%d Run%d", refFrame.Data(), m_expNumber, m_runNumber);
  m_clusterTimeV456 = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV456->GetXaxis()->SetTitle("cluster time (ns)");
  m_clusterTimeV456->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_clusterTimeV456);

  //----------------------------------------------------------------
  // MaxBin of strips for all sensors (offline ZS)
  //----------------------------------------------------------------
  name = "SVDDQM_StripMaxBinUAll";
  title = TString::Format("SVD U-Strip MaxBin for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinUAll = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinUAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinUAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinUAll);

  name = "SVDDQM_StripMaxBinVAll";
  title = TString::Format("SVD V-Strip MaxBin for all sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinVAll = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinVAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinVAll->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinVAll);

  name = "SVDDQM_StripMaxBinU3";
  title = TString::Format("SVD U-Strip MaxBin for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinU3 = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinU3);

  name = "SVDDQM_StripMaxBinV3";
  title = TString::Format("SVD V-Strip MaxBin for layer 3 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinV3 = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinV3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinV3->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinV3);

  name = "SVDDQM_StripMaxBinU6";
  title = TString::Format("SVD U-Strip MaxBin for layer 6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinU6 = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU6->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU6->GetYaxis()->SetTitle("count");
  m_histoList->Add(m_stripMaxBinU6);

  name = "SVDDQM_StripMaxBinV6";
  title = TString::Format("SVD V-Strip MaxBin for layer 6 sensors ~ Exp%d Run%d", m_expNumber, m_runNumber);
  m_stripMaxBinV6 = new TH1F(name.Data(), title.Data(), MaxBinBins, 0, MaxBinMax);
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
    TString sensorDescr = TString::Format("%d_%d_%d", iLayer, iLadder, iSensor);
    //----------------------------------------------------------------
    // Number of fired strips per sensor
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_FiredU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Number of Fired U-Strips ~ Exp%d Run%d", sensorDescr.Data(), m_expNumber, m_runNumber);
    m_firedU[i] = new TH1F(name.Data(), title.Data(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# fired strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_firedU[i]);

    name = TString::Format("SVDDQM_%s_FiredV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Number of Fired V-Strips", sensorDescr.Data());
    m_firedV[i] = new TH1F(name.Data(), title.Data(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# fired strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_firedV[i]);
    //----------------------------------------------------------------
    // Number of clusters per sensor
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ClustersU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Number of U-Clusters", sensorDescr.Data());
    m_clustersU[i] = new TH1F(name.Data(), title.Data(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clustersU[i]);

    name = TString::Format("SVDDQM_%s_ClustersV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Number of V-Clusters", sensorDescr.Data());
    m_clustersV[i] = new TH1F(name.Data(), title.Data(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clustersV[i]);
    //----------------------------------------------------------------
    // Charge of clusters
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ClusterChargeU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s U-Cluster Charge", sensorDescr.Data());
    m_clusterChargeU[i] = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterChargeU[i]);

    name = TString::Format("SVDDQM_%s_ClusterChargeV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s V-Cluster Charge", sensorDescr.Data());
    m_clusterChargeV[i] = new TH1F(name.Data(), title.Data(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterChargeV[i]);
    //----------------------------------------------------------------
    // SNR of clusters
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ClusterSNRU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s U-Cluster SNR", sensorDescr.Data());
    m_clusterSNRU[i] = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
    m_clusterSNRU[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSNRU[i]);

    name = TString::Format("SVDDQM_%s_ClusterSNRV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s V-Cluster SNR", sensorDescr.Data());
    m_clusterSNRV[i] = new TH1F(name.Data(), title.Data(), SNRBins, 0, SNRMax);
    m_clusterSNRV[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSNRV[i]);
    //----------------------------------------------------------------
    // Charge of strips
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ADCStripU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s U-Strip signal in ADC Counts, all 6 APV samples", sensorDescr.Data());
    m_stripSignalU[i] = new TH1F(name.Data(), title.Data(), 256, -0.5, 255.5);
    m_stripSignalU[i]->GetXaxis()->SetTitle("signal ADC");
    m_stripSignalU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripSignalU[i]);

    name = TString::Format("SVDDQM_%s_ADCStripV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s V-Strip signal in ADC Counts, all 6 APV samples", sensorDescr.Data());
    m_stripSignalV[i] = new TH1F(name.Data(), title.Data(), 256, -0.5, 255.5);
    m_stripSignalV[i]->GetXaxis()->SetTitle("signal ADC");
    m_stripSignalV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripSignalV[i]);
    //----------------------------------------------------------------
    // Strips Counts
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_StripCountU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Integrated Number of ZS5 Fired U-Strip vs Strip Number", sensorDescr.Data());
    m_stripCountU[i] = new TH1F(name.Data(), title.Data(), 768, -0.5, 767.5);
    m_stripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountU[i]);

    name = TString::Format("SVDDQM_%s_StripCountV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Integrated Number of ZS5 Fired V-Strip vs Strip Number", sensorDescr.Data());
    m_stripCountV[i] = new TH1F(name.Data(), title.Data(), 768, -0.5, 767.5);
    m_stripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_stripCountV[i]);
    //----------------------------------------------------------------
    // Strips Counts with online ZS
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_OnlineZSStripCountU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Integrated Number of online-ZS Fired U-Strip vs Strip Number", sensorDescr.Data());
    m_onlineZSstripCountU[i] = new TH1F(name.Data(), title.Data(), 768, -0.5, 767.5);
    m_onlineZSstripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstripCountU[i]);

    name = TString::Format("SVDDQM_%s_OnlineZSStripCountV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s Integrated Number of online-ZS Fired V-Strip vs Strip Number", sensorDescr.Data());
    m_onlineZSstripCountV[i] = new TH1F(name.Data(), title.Data(), 768, -0.5, 767.5);
    m_onlineZSstripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_onlineZSstripCountV[i]);
    //----------------------------------------------------------------
    // Cluster size distribution
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ClusterSizeU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s U-Cluster Size", sensorDescr.Data());
    m_clusterSizeU[i] = new TH1F(name.Data(), title.Data(), 9, 1, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSizeU[i]);

    name = TString::Format("SVDDQM_%s_ClusterSizeV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s V-Cluster Size", sensorDescr.Data());
    m_clusterSizeV[i] = new TH1F(name.Data(), title.Data(), 9, 1, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterSizeV[i]);
    //----------------------------------------------------------------
    // Cluster time distribution
    //----------------------------------------------------------------
    name = TString::Format("SVDDQM_%s_ClusterTimeU", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s U-Cluster Time %s", sensorDescr.Data(), refFrame.Data());
    m_clusterTimeU[i] = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("cluster time (ns)");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    m_histoList->Add(m_clusterTimeU[i]);

    name = TString::Format("SVDDQM_%s_ClusterTimeV", sensorDescr.Data());
    title = TString::Format("SVD Sensor %s V-Cluster Time %s", sensorDescr.Data(), refFrame.Data());
    m_clusterTimeV[i] = new TH1F(name.Data(), title.Data(), TimeBins, TimeMin, TimeMax);
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
    TString AxisTicks = TString::Format("%i_%i_%i_u%i", iLayer, iLadder, iSensor, iChip);
    if (!IsU)
      AxisTicks = TString::Format("%i_%i_%i_v%i", iLayer, iLadder, iSensor, iChip);
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
      TString sensorDescr = TString::Format("%i_%i_%i", iLayer, iLadder, iSensor);
      //----------------------------------------------------------------
      // Hitmaps: Number of strips by coordinate
      //----------------------------------------------------------------
      name = TString::Format("SVD_%s_StripHitmapU", sensorDescr.Data());
      title = TString::Format("SVD Sensor %s Strip Hitmap in U", sensorDescr.Data());

      int nStrips = SensorInfo.getUCells();
      m_hitMapU[i] = new TH2F(name.Data(), title.Data(), nStrips, 0, nStrips, SVDShaperDigit::c_nAPVSamples, 0,
                              SVDShaperDigit::c_nAPVSamples);
      m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapU[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapU[i]->GetZaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapU[i]);

      name = TString::Format("SVD_%s_StripHitmapV", sensorDescr.Data());
      title = TString::Format("SVD Sensor %s Strip Hitmap in V", sensorDescr.Data());
      nStrips = SensorInfo.getVCells();
      m_hitMapV[i] = new TH2F(name.Data(), title.Data(), nStrips, 0, nStrips, SVDShaperDigit::c_nAPVSamples, 0,
                              SVDShaperDigit::c_nAPVSamples);
      m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapV[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapV[i]->GetZaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapV[i]);
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate
      //----------------------------------------------------------------
      name = TString::Format("SVD_%s_HitmapClstU", sensorDescr.Data());
      title = TString::Format("SVD Sensor %s Hitmap Clusters in U", sensorDescr.Data());
      nStrips = SensorInfo.getUCells();
      m_hitMapUCl[i] = new TH1F(name.Data(), title.Data(), nStrips, 0, nStrips);
      m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
      m_histoList->Add(m_hitMapUCl[i]);

      name = TString::Format("SVD_%s_HitmapClstV", sensorDescr.Data());
      title = TString::Format("SVD Sensor %s Hitmap Clusters in V", sensorDescr.Data());
      nStrips = SensorInfo.getVCells();
      m_hitMapVCl[i] = new TH1F(name.Data(), title.Data(), nStrips, 0, nStrips);
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
}

void SVDDQMExpressRecoModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

}

void SVDDQMExpressRecoModule::event()
{

  //check HLT decision and increase number of events only if the event has been accepted

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }
  m_nEvents->Fill(0);

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

      //      B2DEBUG(29, digitIn.toString().c_str() );

      //fill strip count first
      if (m_stripCountU[index] != nullptr) m_stripCountU[index]->Fill(digitIn.getCellID());

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
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_firedU[i] != nullptr) && (uStrips[i].size() > 0))
      m_firedU[i]->Fill(uStrips[i].size());
    if ((m_firedV[i] != nullptr) && (vStrips[i].size() > 0))
      m_firedV[i]->Fill(vStrips[i].size());
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
      } else {
        if (m_onlineZSstripCountV[index] != nullptr) m_onlineZSstripCountV[index]->Fill(digitIn.getCellID());
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
      } else {
        if (m_clusterChargeU456 != nullptr) m_clusterChargeU456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRU456 != nullptr) m_clusterSNRU456->Fill(cluster.getSNR());
        if (m_clusterTimeU456 != nullptr) m_clusterTimeU456->Fill(time);
      }

      if (m_ShowAllHistos == 1)
        if (m_hitMapUCl[index] != nullptr) m_hitMapUCl[index]->Fill(SensorInfo.getUCellID(cluster.getPosition()));
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
      } else {
        if (m_clusterChargeV456 != nullptr) m_clusterChargeV456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRV456 != nullptr) m_clusterSNRV456->Fill(cluster.getSNR());
        if (m_clusterTimeV456 != nullptr) m_clusterTimeV456->Fill(time);
      }
      if (m_ShowAllHistos == 1)
        if (m_hitMapVCl[index] != nullptr) m_hitMapVCl[index]->Fill(SensorInfo.getVCellID(cluster.getPosition()));

    }
  }
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_clustersU[i] != nullptr) && (countsU[i].size() > 0))
      m_clustersU[i]->Fill(countsU[i].size());
    if ((m_clustersV[i] != nullptr) && (countsV[i].size() > 0))
      m_clustersV[i]->Fill(countsV[i].size());
  }
}


void SVDDQMExpressRecoModule::terminate()
{

  //  m_histoList->Delete();
  delete m_histoList;

}
