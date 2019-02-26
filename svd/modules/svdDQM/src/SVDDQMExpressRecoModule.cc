/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDDQMExpressRecoModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
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
  setDescription("SVD DQM module for Express Reco"
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
                 "Container for histograms for off-line analysis with any granulation base on request "
                 "Call all histograms set ShowAllHistos=1 ."
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("ShaperDigits", m_storeSVDShaperDigitsName, "ShaperDigits StoreArray name", std::string("SVDShaperDigits"));
  addParam("noZSShaperDigits", m_storeNoZSSVDShaperDigitsName, "not zero-suppressed ShaperDigits StoreArray name",
           std::string("SVDShaperDigits"));

  addParam("ShowAllHistos", m_ShowAllHistos, "Flag to show all histos in DQM, default = 0 ", m_ShowAllHistos);
  addParam("CutSVDCharge", m_CutSVDCharge,
           "cut for accepting to hitmap histogram, using strips only", m_CutSVDCharge);
  addParam("CutSVDClusterCharge", m_CutSVDClusterCharge,
           "cut for accepting clusters to hitmap histogram", m_CutSVDClusterCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDExpReco"));

  m_cumHistos = new TList();
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
  m_nEvents = new TH1F("SVDDQM_nEvents", "SVD Number of Events", 1, -0.5, 0.5);
  m_nEvents->GetYaxis()->SetTitle("N events");
  m_cumHistos->Add(m_nEvents);

  // Create basic histograms:
  // basic counters per sensor:
  m_hitMapCountsU = new TH1F("SVDDQM_StripCountsU", "SVD Integrated Number of ZS5 Fired U-Strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_cumHistos->Add(m_hitMapCountsU);
  m_hitMapCountsV = new TH1F("SVDDQM_StripCountsV", "SVD Integrated Number of ZS5 Fired V-Strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_cumHistos->Add(m_hitMapCountsV);
  m_hitMapClCountsU = new TH1F("SVDDQM_ClusterCountsU", "SVD Integrated Number of U-Clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_cumHistos->Add(m_hitMapClCountsU);
  m_hitMapClCountsV = new TH1F("SVDDQM_ClusterCountsV", "SVD Integrated Number of V-Clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");
  m_cumHistos->Add(m_hitMapClCountsV);
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
  m_cumHistos->Add(m_hitMapCountsChip);
  m_hitMapClCountsChip = new TH1F("SVDDQM_ClusterCountsChip", "SVD Integrated Number of Clusters per chip",
                                  nSVDChips, 0, nSVDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  m_cumHistos->Add(m_hitMapClCountsChip);

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
  int TimeBins = 50;
  float TimeMin = -100;
  float TimeMax = 100;

  int MaxBinBins = 6;
  int MaxBinMax = 6;
  //----------------------------------------------------------------
  // Charge of clusters for all sensors
  //----------------------------------------------------------------
  string name = str(format("SVDDQM_ClusterChargeUAll"));
  string title = str(format("SVD U-Cluster Charge for all sensors"));
  m_clusterChargeUAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeUAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeUAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeUAll);
  name = str(format("SVDDQM_ClusterChargeVAll"));
  title = str(format("SVD V-Cluster Charge for all sensors"));
  m_clusterChargeVAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeVAll->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeVAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeVAll);
  //----------------------------------------------------------------
  // Charge of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterChargeU3"));
  title = str(format("SVD U-Cluster Charge for layer 3 sensors"));
  m_clusterChargeU3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeU3);
  name = str(format("SVDDQM_ClusterChargeV3"));
  title = str(format("SVD V-Cluster Charge for layer 3 sensors"));
  m_clusterChargeV3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV3->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeV3);

  name = str(format("SVDDQM_ClusterChargeU456"));
  title = str(format("SVD U-Cluster Charge for layers 4,5,6 sensors"));
  m_clusterChargeU456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeU456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeU456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeU456);

  name = str(format("SVDDQM_ClusterChargeV456"));
  title = str(format("SVD V-Cluster Charge for layers 4,5,6 sensors"));
  m_clusterChargeV456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeV456->GetXaxis()->SetTitle("cluster charge [ke-]");
  m_clusterChargeV456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterChargeV456);

  //----------------------------------------------------------------
  // SNR of clusters for all sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterSNRUAll"));
  title = str(format("SVD U-Cluster SNR for all sensors"));
  m_clusterSNRUAll = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);  // max = ~ 60
  m_clusterSNRUAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRUAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRUAll);
  name = str(format("SVDDQM_ClusterSNRVAll"));
  title = str(format("SVD V-Cluster SNR for all sensors"));
  m_clusterSNRVAll = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRVAll->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRVAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRVAll);
  //----------------------------------------------------------------
  // SNR of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterSNRU3"));
  title = str(format("SVD U-Cluster SNR for layer 3 sensors"));
  m_clusterSNRU3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRU3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRU3);
  name = str(format("SVDDQM_ClusterSNRV3"));
  title = str(format("SVD V-Cluster SNR for layer 3 sensors"));
  m_clusterSNRV3 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRV3->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRV3);

  name = str(format("SVDDQM_ClusterSNRU456"));
  title = str(format("SVD U-Cluster SNR for layers 4,5,6 sensors"));
  m_clusterSNRU456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRU456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRU456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRU456);
  name = str(format("SVDDQM_ClusterSNRV456"));
  title = str(format("SVD V-Cluster SNR for layers 4,5,6 sensors"));
  m_clusterSNRV456 = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
  m_clusterSNRV456->GetXaxis()->SetTitle("cluster SNR");
  m_clusterSNRV456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterSNRV456);
  //----------------------------------------------------------------
  // Cluster time distribution for all sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterTimeUAll"));
  title = str(format("SVD U-Cluster Time for all sensors"));
  m_clusterTimeUAll = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeUAll->GetXaxis()->SetTitle("cluster time [ns]");
  m_clusterTimeUAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeUAll);
  name = str(format("SVDDQM_ClusterTimeVAll"));
  title = str(format("SVD V-Cluster Time for all sensors"));
  m_clusterTimeVAll = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeVAll->GetXaxis()->SetTitle("cluster time [ns]");
  m_clusterTimeVAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeVAll);
  //----------------------------------------------------------------
  // Time of clusters for L3/L456 sensors
  //----------------------------------------------------------------
  name = str(format("SVDDQM_ClusterTimeU3"));
  title = str(format("SVD U-Cluster Time for layer 3 sensors"));
  m_clusterTimeU3 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU3->GetXaxis()->SetTitle("clusters time [ns]");
  m_clusterTimeU3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeU3);
  name = str(format("SVDDQM_ClusterTimeV3"));
  title = str(format("SVD V-Cluster Time for layer 3 sensors"));
  m_clusterTimeV3 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV3->GetXaxis()->SetTitle("cluster time [ns]");
  m_clusterTimeV3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeV3);

  name = str(format("SVDDQM_ClusterTimeU456"));
  title = str(format("SVD U-Cluster Time for layers 4,5,6 sensors"));
  m_clusterTimeU456 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeU456->GetXaxis()->SetTitle("cluster time [ns]");
  m_clusterTimeU456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeU456);
  name = str(format("SVDDQM_ClusterTimeV456"));
  title = str(format("SVD V-Cluster Time for layers 4,5,6 sensors"));
  m_clusterTimeV456 = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeV456->GetXaxis()->SetTitle("cluster time [ns]");
  m_clusterTimeV456->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_clusterTimeV456);

  //----------------------------------------------------------------
  // MaxBin of strips for all sensors (offline ZS)
  //----------------------------------------------------------------
  name = str(format("SVDDQM_StripMaxBinUAll"));
  title = str(format("SVD U-Strip MaxBin for all sensors"));
  m_stripMaxBinUAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinUAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinUAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinUAll);
  name = str(format("SVDDQM_StripMaxBinVAll"));
  title = str(format("SVD V-Strip MaxBin for all sensors"));
  m_stripMaxBinVAll = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinVAll->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinVAll->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinVAll);

  name = str(format("SVDDQM_StripMaxBinU3"));
  title = str(format("SVD U-Strip MaxBin for layer 3 sensors"));
  m_stripMaxBinU3 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinU3);
  name = str(format("SVDDQM_StripMaxBinV3"));
  title = str(format("SVD V-Strip MaxBin for layer 3 sensors"));
  m_stripMaxBinV3 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinV3->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinV3->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinV3);

  name = str(format("SVDDQM_StripMaxBinU6"));
  title = str(format("SVD U-Strip MaxBin for layer 6 sensors"));
  m_stripMaxBinU6 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinU6->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinU6->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinU6);
  name = str(format("SVDDQM_StripMaxBinV6"));
  title = str(format("SVD V-Strip MaxBin for layer 6 sensors"));
  m_stripMaxBinV6 = new TH1F(name.c_str(), title.c_str(), MaxBinBins, 0, MaxBinMax);
  m_stripMaxBinV6->GetXaxis()->SetTitle("max bin");
  m_stripMaxBinV6->GetYaxis()->SetTitle("count");
  m_cumHistos->Add(m_stripMaxBinV6);

  for (int i = 0; i < nSVDSensors; i++) {
    VxdID id = gTools->getSensorIDFromSVDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired strips per sensor
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_FiredU") % sensorDescr);
    title = str(format("SVD Sensor %1% Number of Fired U-Strips") % sensorDescr);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# fired strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_FiredV") % sensorDescr);
    title = str(format("SVD Sensor %1% Number of Fired V-Strips") % sensorDescr);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# fired strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Number of clusters per sensor
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClustersU") % sensorDescr);
    title = str(format("SVD Sensor %1% Number of U-Clusters") % sensorDescr);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ClustersV") % sensorDescr);
    title = str(format("SVD Sensor %1% Number of V-Clusters") % sensorDescr);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of clusters
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterChargeU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster Charge") % sensorDescr);
    m_clusterChargeU[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ClusterChargeV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster Charge") % sensorDescr);
    m_clusterChargeV[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("cluster charge [ke-]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // SNR of clusters
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterSNRU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster SNR") % sensorDescr);
    m_clusterSNRU[i] = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
    m_clusterSNRU[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ClusterSNRV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster SNR") % sensorDescr);
    m_clusterSNRV[i] = new TH1F(name.c_str(), title.c_str(), SNRBins, 0, SNRMax);
    m_clusterSNRV[i]->GetXaxis()->SetTitle("cluster SNR");
    m_clusterSNRV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of strips
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ADCStripU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Strip signal in ADC Counts, all 6 APV samples") % sensorDescr);
    m_stripSignalU[i] = new TH1F(name.c_str(), title.c_str(), 256, -0.5, 255.5);
    m_stripSignalU[i]->GetXaxis()->SetTitle("signal ADC");
    m_stripSignalU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ADCStripV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Strip signal in ADC Counts, all 6 APV samples") % sensorDescr);
    m_stripSignalV[i] = new TH1F(name.c_str(), title.c_str(), 256, -0.5, 255.5);
    m_stripSignalV[i]->GetXaxis()->SetTitle("signal ADC");
    m_stripSignalV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Strips Counts
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_StripCountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired U-Strip vs Strip Number") % sensorDescr);
    m_stripCountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_StripCountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of ZS5 Fired V-Strip vs Strip Number") % sensorDescr);
    m_stripCountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_stripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_stripCountV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Strips Counts with online ZS
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_OnlineZSStripCountU") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired U-Strip vs Strip Number") % sensorDescr);
    m_onlineZSstripCountU[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstripCountU[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_OnlineZSStripCountV") % sensorDescr);
    title = str(format("SVD Sensor %1% Integrated Number of online-ZS Fired V-Strip vs Strip Number") % sensorDescr);
    m_onlineZSstripCountV[i] = new TH1F(name.c_str(), title.c_str(), 768, -0.5, 767.5);
    m_onlineZSstripCountV[i]->GetXaxis()->SetTitle("cellID");
    m_onlineZSstripCountV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster size distribution
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster Size") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 9, 1, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster Size") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 9, 1, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("cluster size");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster time distribution
    //----------------------------------------------------------------
    name = str(format("SVDDQM_%1%_ClusterTimeU") % sensorDescr);
    title = str(format("SVD Sensor %1% U-Cluster Time") % sensorDescr);
    m_clusterTimeU[i] = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("time [ns]");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("SVDDQM_%1%_ClusterTimeV") % sensorDescr);
    title = str(format("SVD Sensor %1% V-Cluster Time") % sensorDescr);
    m_clusterTimeV[i] = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeV[i]->GetXaxis()->SetTitle("time [ns]");
    m_clusterTimeV[i]->GetYaxis()->SetTitle("count");
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
    TDirectory* dirShowAll = NULL;
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
      name = str(format("SVD_%1%_StripHitmapV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapV[i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, SVDShaperDigit::c_nAPVSamples, 0,
                              SVDShaperDigit::c_nAPVSamples);
      m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapV[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapV[i]->GetZaxis()->SetTitle("hits");
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate
      //----------------------------------------------------------------
      name = str(format("SVD_%1%_HitmapClstU") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in U") % sensorDescr);
      nStrips = SensorInfo.getUCells();
      m_hitMapUCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
      name = str(format("SVD_%1%_HitmapClstV") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapVCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
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
    StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
    StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
    m_storeSVDClustersName = storeSVDClusters.getName();

    storeSVDClusters.isOptional();
    storeSVDShaperDigits.isOptional();

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

  // Add experiment and run number to the title of selected histograms (CR shifter plots)
  TString runID = TString::Format(" ~ Exp%d Run%d", m_expNumber, m_runNumber);
  TObject* obj;
  TIter nextH(m_cumHistos);
  while ((obj = nextH()))
    if (obj->InheritsFrom("TH1"))
      ((TH1F*)obj)->SetTitle(obj->GetTitle() + runID);

  // Just to make sure, reset all the histograms.
  if (m_nEvents != NULL) m_nEvents->Reset();
  if (m_hitMapCountsU != NULL) m_hitMapCountsU->Reset();
  if (m_hitMapCountsV != NULL) m_hitMapCountsV->Reset();
  if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Reset();
  if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Reset();

  if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Reset();

  if (m_clusterChargeUAll != NULL) m_clusterChargeUAll->Reset();
  if (m_clusterChargeVAll != NULL) m_clusterChargeVAll->Reset();
  if (m_clusterSNRUAll != NULL) m_clusterSNRUAll->Reset();
  if (m_clusterSNRVAll != NULL) m_clusterSNRVAll->Reset();
  if (m_clusterTimeUAll != NULL) m_clusterTimeUAll->Reset();
  if (m_clusterTimeVAll != NULL) m_clusterTimeVAll->Reset();
  if (m_clusterChargeU3 != NULL) m_clusterChargeU3->Reset();
  if (m_clusterChargeV3 != NULL) m_clusterChargeV3->Reset();
  if (m_clusterChargeU456 != NULL) m_clusterChargeU456->Reset();
  if (m_clusterChargeV456 != NULL) m_clusterChargeV456->Reset();
  if (m_clusterSNRU3 != NULL) m_clusterSNRU3->Reset();
  if (m_clusterSNRV3 != NULL) m_clusterSNRV3->Reset();
  if (m_clusterSNRU456 != NULL) m_clusterSNRU456->Reset();
  if (m_clusterSNRV456 != NULL) m_clusterSNRV456->Reset();
  if (m_clusterTimeU3 != NULL) m_clusterTimeU3->Reset();
  if (m_clusterTimeV3 != NULL) m_clusterTimeV3->Reset();
  if (m_clusterTimeU456 != NULL) m_clusterTimeU456->Reset();
  if (m_clusterTimeV456 != NULL) m_clusterTimeV456->Reset();
  if (m_stripMaxBinUAll != NULL) m_stripMaxBinUAll->Reset();
  if (m_stripMaxBinVAll != NULL) m_stripMaxBinVAll->Reset();
  for (int i = 0; i < gTools->getNumberOfSVDSensors(); i++) {
    if (m_firedU[i] != NULL) m_firedU[i]->Reset();
    if (m_firedV[i] != NULL) m_firedV[i]->Reset();
    if (m_clustersU[i] != NULL) m_clustersU[i]->Reset();
    if (m_clustersV[i] != NULL) m_clustersV[i]->Reset();
    if (m_clusterChargeU[i] != NULL) m_clusterChargeU[i]->Reset();
    if (m_clusterChargeV[i] != NULL) m_clusterChargeV[i]->Reset();
    if (m_clusterSNRU[i] != NULL) m_clusterSNRU[i]->Reset();
    if (m_clusterSNRV[i] != NULL) m_clusterSNRV[i]->Reset();
    if (m_stripSignalU[i] != NULL) m_stripSignalU[i]->Reset();
    if (m_stripSignalV[i] != NULL) m_stripSignalV[i]->Reset();
    if (m_stripCountU[i] != NULL) m_stripCountU[i]->Reset();
    if (m_stripCountV[i] != NULL) m_stripCountV[i]->Reset();
    if (m_onlineZSstripCountU[i] != NULL) m_onlineZSstripCountU[i]->Reset();
    if (m_onlineZSstripCountV[i] != NULL) m_onlineZSstripCountV[i]->Reset();
    if (m_clusterSizeU[i] != NULL) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != NULL) m_clusterSizeV[i]->Reset();
    if (m_clusterTimeU[i] != NULL) m_clusterTimeU[i]->Reset();
    if (m_clusterTimeV[i] != NULL) m_clusterTimeV[i]->Reset();
  }
  if (m_ShowAllHistos == 1) {
    for (int i = 0; i < gTools->getNumberOfSVDSensors(); i++) {
      if (m_hitMapU[i] != NULL) m_hitMapU[i]->Reset();
      if (m_hitMapV[i] != NULL) m_hitMapV[i]->Reset();
      if (m_hitMapUCl[i] != NULL) m_hitMapUCl[i]->Reset();
      if (m_hitMapVCl[i] != NULL) m_hitMapVCl[i]->Reset();
    }
  }
}

void SVDDQMExpressRecoModule::event()
{

  //increase the numbe rof processed events
  m_nEvents->Fill(0);

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  const StoreArray<SVDShaperDigit> storeNoZSSVDShaperDigits(m_storeNoZSSVDShaperDigitsName);

  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);

  if (!storeSVDShaperDigits || !storeSVDShaperDigits.getEntries()) {
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
      if (m_stripCountU[index] != NULL) m_stripCountU[index]->Fill(digitIn.getCellID());

      //fill max bin
      if (m_stripMaxBinUAll != NULL) m_stripMaxBinUAll->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 3)
        if (m_stripMaxBinU3 != NULL) m_stripMaxBinU3->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 6)
        if (m_stripMaxBinU6 != NULL) m_stripMaxBinU6->Fill(digitIn.getMaxTimeBin());

      uStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip()) + 1;
      int indexChip = gTools->getSVDChipIndex(sensorID, kTRUE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      int isSample = 0;
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalU[index] != NULL) m_stripSignalU[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          isSample = 1;
          if (m_ShowAllHistos == 1) {
            if (m_hitMapU[index] != NULL) m_hitMapU[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
      if (isSample) {
        if (m_hitMapCountsU != NULL) m_hitMapCountsU->Fill(index);
        if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
      }
    } else {
      //fill strip count first
      if (m_stripCountV[index] != NULL) m_stripCountV[index]->Fill(digitIn.getCellID());

      //fill max bin
      if (m_stripMaxBinVAll != NULL) m_stripMaxBinVAll->Fill(digitIn.getMaxTimeBin());

      if (iLayer == 3)
        if (m_stripMaxBinV3 != NULL) m_stripMaxBinV3->Fill(digitIn.getMaxTimeBin());
      if (iLayer == 6)
        if (m_stripMaxBinV6 != NULL) m_stripMaxBinV6->Fill(digitIn.getMaxTimeBin());

      vStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip()) + 1;
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      int isSample = 0;
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalV[index] != NULL) m_stripSignalV[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          isSample = 1;
          if (m_ShowAllHistos == 1) {
            if (m_hitMapV[index] != NULL) m_hitMapV[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
      if (isSample) {
        if (m_hitMapCountsV != NULL) m_hitMapCountsV->Fill(index);
        if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
      }
    }
  }
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_firedU[i] != NULL) && (uStrips[i].size() > 0))
      m_firedU[i]->Fill(uStrips[i].size());
    if ((m_firedV[i] != NULL) && (vStrips[i].size() > 0))
      m_firedV[i]->Fill(vStrips[i].size());
  }

  // Fired strips ONLINE ZS
  for (const SVDShaperDigit& digitIn : storeNoZSSVDShaperDigits) {
    int iLayer = digitIn.getSensorID().getLayerNumber();
    if ((iLayer < firstSVDLayer) || (iLayer > lastSVDLayer)) continue;
    int iLadder = digitIn.getSensorID().getLadderNumber();
    int iSensor = digitIn.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getSVDSensorIndex(sensorID);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (digitIn.isUStrip()) {
      if (m_onlineZSstripCountU[index] != NULL) m_onlineZSstripCountU[index]->Fill(digitIn.getCellID());
    } else {
      if (m_onlineZSstripCountV[index] != NULL) m_onlineZSstripCountV[index]->Fill(digitIn.getCellID());
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
    if (cluster.isUCluster()) {
      countsU.at(index).insert(SensorInfo.getUCellID(cluster.getPosition()));
      int indexChip = gTools->getSVDChipIndex(sensorID, kTRUE,
                                              (int)(SensorInfo.getUCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()) + 1);
      if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Fill(index);
      if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeU[index] != NULL) m_clusterChargeU[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRU[index] != NULL) m_clusterSNRU[index]->Fill(cluster.getSNR());
      if (m_clusterChargeUAll != NULL) m_clusterChargeUAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRUAll != NULL) m_clusterSNRUAll->Fill(cluster.getSNR());
      if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getSize());
      if (m_clusterTimeU[index] != NULL) m_clusterTimeU[index]->Fill(cluster.getClsTime());
      if (m_clusterTimeUAll != NULL) m_clusterTimeUAll->Fill(cluster.getClsTime());
      if (iLayer == 3) {
        if (m_clusterChargeU3 != NULL) m_clusterChargeU3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRU3 != NULL) m_clusterSNRU3->Fill(cluster.getSNR());
        if (m_clusterTimeU3 != NULL) m_clusterTimeU3->Fill(cluster.getClsTime());
      } else {
        if (m_clusterChargeU456 != NULL) m_clusterChargeU456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRU456 != NULL) m_clusterSNRU456->Fill(cluster.getSNR());
        if (m_clusterTimeU456 != NULL) m_clusterTimeU456->Fill(cluster.getClsTime());
      }

      if (m_ShowAllHistos == 1)
        if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(SensorInfo.getUCellID(cluster.getPosition()));

    } else {
      countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE,
                                              (int)(SensorInfo.getVCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()) + 1);
      if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Fill(index);
      if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeV[index] != NULL) m_clusterChargeV[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRV[index] != NULL) m_clusterSNRV[index]->Fill(cluster.getSNR());
      if (m_clusterChargeVAll != NULL) m_clusterChargeVAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSNRVAll != NULL) m_clusterSNRVAll->Fill(cluster.getSNR());
      if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getSize());
      if (m_clusterTimeV[index] != NULL) m_clusterTimeV[index]->Fill(cluster.getClsTime());
      if (m_clusterTimeVAll != NULL) m_clusterTimeVAll->Fill(cluster.getClsTime());
      if (iLayer == 3) {
        if (m_clusterChargeV3 != NULL) m_clusterChargeV3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRV3 != NULL) m_clusterSNRV3->Fill(cluster.getSNR());
        if (m_clusterTimeV3 != NULL) m_clusterTimeV3->Fill(cluster.getClsTime());
      } else {
        if (m_clusterChargeV456 != NULL) m_clusterChargeV456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_clusterSNRV456 != NULL) m_clusterSNRV456->Fill(cluster.getSNR());
        if (m_clusterTimeV456 != NULL) m_clusterTimeV456->Fill(cluster.getClsTime());
      }

      if (m_ShowAllHistos == 1)
        if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(SensorInfo.getVCellID(cluster.getPosition()));

    }
  }
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_clustersU[i] != NULL) && (countsU[i].size() > 0))
      m_clustersU[i]->Fill(countsU[i].size());
    if ((m_clustersV[i] != NULL) && (countsV[i].size() > 0))
      m_clustersV[i]->Fill(countsV[i].size());
  }
}
