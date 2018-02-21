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

#include <svd/dataobjects/SVDDAQDiagnostic.h>
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

  addParam("ShowAllHistos", m_ShowAllHistos,
           "Flag to show all histot in DQM, default = 0 ", m_ShowAllHistos);
  addParam("CutSVDCharge", m_CutSVDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 22 ADU ", m_CutSVDCharge);
  addParam("CutSVDClusterCharge", m_CutSVDClusterCharge,
           "cut for accepting clusters to hitmap histogram, default = 12 ke- ", m_CutSVDClusterCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDExpReco"));
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

  // Create basic histograms:
  // basic counters per sensor:
  m_hitMapCountsU = new TH1I("DQMER_SVD_StripCountsU", "DQM ER SVD Integrated number of fired U strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapCountsV = new TH1I("DQMER_SVD_StripCountsV", "DQM ER SVD Integrated number of fired V strips per sensor",
                             nSVDSensors, 0, nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsU = new TH1I("DQMER_SVD_ClusterCountsU", "DQM ER SVD Integrated number of U clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsV = new TH1I("DQMER_SVD_ClusterCountsV", "DQM ER SVD Integrated number of V clusters per sensor",
                               nSVDSensors, 0, nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");
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
  m_hitMapCountsChip = new TH1I("DQMER_SVD_StripCountsChip", "DQM ER SVD Integrated number of fired strips per chip",
                                nSVDChips, 0, nSVDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1I("DQMER_SVD_ClusterCountsChip", "DQM ER SVD Integrated number of clusters per chip",
                                  nSVDChips, 0, nSVDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");


  m_firedU = new TH1F*[nSVDSensors];
  m_firedV = new TH1F*[nSVDSensors];
  m_clustersU = new TH1F*[nSVDSensors];
  m_clustersV = new TH1F*[nSVDSensors];

  /*
  m_CounterAPVErrors = new TH1I*[c_nFADC];
  m_CounterFTBErrors = new TH1I*[c_nFADC];
  m_CounterApvErrorORErrors = new TH1I*[c_nFADC];
  m_CounterFTBFlags = new TH1I*[c_nFADC];
  */

  m_clusterChargeU = new TH1F*[nSVDSensors];
  m_clusterChargeV = new TH1F*[nSVDSensors];
  m_stripSignalU = new TH1F*[nSVDSensors];
  m_stripSignalV = new TH1F*[nSVDSensors];
  m_clusterSizeU = new TH1F*[nSVDSensors];
  m_clusterSizeV = new TH1F*[nSVDSensors];
  m_clusterTimeU = new TH1F*[nSVDSensors];
  m_clusterTimeV = new TH1F*[nSVDSensors];

  int ChargeBins = 80;
  float ChargeMax = 80;
  int TimeBins = 50;
  float TimeMin = -100;
  float TimeMax = 100;
  //----------------------------------------------------------------
  // Charge of clusters for all sensors
  //----------------------------------------------------------------
  string name = str(format("DQMER_SVD_ClusterChargeUAll"));
  string title = str(format("DQM ER SVD Cluster charge in U for all sensors"));
  m_clusterChargeUAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeUAll->GetXaxis()->SetTitle("charge of u clusters [ke-]");
  m_clusterChargeUAll->GetYaxis()->SetTitle("count");
  name = str(format("DQMER_SVD_ClusterChargeVAll"));
  title = str(format("DQM ER SVD Cluster charge in V for all sensors"));
  m_clusterChargeVAll = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
  m_clusterChargeVAll->GetXaxis()->SetTitle("charge of v clusters [ke-]");
  m_clusterChargeVAll->GetYaxis()->SetTitle("count");

  //----------------------------------------------------------------
  // Cluster time distribution for all sensors
  //----------------------------------------------------------------
  name = str(format("DQMER_SVD_ClusterTimeUAll"));
  title = str(format("DQM ER SVD Cluster time in U for all sensors"));
  m_clusterTimeUAll = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeUAll->GetXaxis()->SetTitle("time of u clusters [ns]");
  m_clusterTimeUAll->GetYaxis()->SetTitle("count");
  name = str(format("DQMER_SVD_ClusterTimeVAll"));
  title = str(format("DQM ER SVD Cluster time in V for all sensors"));
  m_clusterTimeVAll = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
  m_clusterTimeVAll->GetXaxis()->SetTitle("time of v clusters [ns]");
  m_clusterTimeVAll->GetYaxis()->SetTitle("count");

  /*
  for (int i = 0; i < c_nFADC; i++) {
    //----------------------------------------------------------------
    // Counter of APV errors (16) per FADC
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_FADC_%1%_CounterOfAPVErrors") % i);
    title = str(format("DQM ER SVD FADC %1% Counter of APV errors") % i);
    m_CounterAPVErrors[i] = new TH1I(name.c_str(), title.c_str(), 16, 0, 16);
    m_CounterAPVErrors[i]->GetXaxis()->SetTitle("Error ID");
    m_CounterAPVErrors[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Counter of FTB errors (256) per FADC
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_FADC_%1%_CounterOfFTBErrors") % i);
    title = str(format("DQM ER SVD FADC %1% Counter of FTB errors") % i);
    m_CounterFTBErrors[i] = new TH1I(name.c_str(), title.c_str(), 256, 0, 256);
    m_CounterFTBErrors[i]->GetXaxis()->SetTitle("Error ID");
    m_CounterFTBErrors[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Counter of apvErrorOR (4) per FADC
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_FADC_%1%_CounterOfApvErrorOR") % i);
    title = str(format("DQM ER SVD FADC %1% Counter of ApvErrorOR") % i);
    m_CounterApvErrorORErrors[i] = new TH1I(name.c_str(), title.c_str(), 5, 0, 5);
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetTitle("Error ID");
    m_CounterApvErrorORErrors[i]->GetYaxis()->SetTitle("counts");
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetBinLabel(1, "No errors");
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetBinLabel(2, "APV error OR");
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetBinLabel(3, "FADC FIFO full OR");
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetBinLabel(4, "Frame error OR");
    m_CounterApvErrorORErrors[i]->GetXaxis()->SetBinLabel(5, "Detection error OR");
    //----------------------------------------------------------------
    // Counter of FTB Flags (32) per FADC
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_FADC_%1%_CounterFTBFlags") % i);
    title = str(format("DQM ER SVD FADC %1% Counter of FTB flags") % i);
    m_CounterFTBFlags[i] = new TH1I(name.c_str(), title.c_str(), 32, 0, 32);
    m_CounterFTBFlags[i]->GetXaxis()->SetTitle("Flag");
    m_CounterFTBFlags[i]->GetYaxis()->SetTitle("counts");
  }
  */

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
    name = str(format("DQMER_SVD_%1%_FiredU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Fired strips in U") % sensorDescr);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_FiredV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Fired strips in V") % sensorDescr);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Number of clusters per sensor
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClustersU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Number of clusters in U") % sensorDescr);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClustersV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Number of clusters in V") % sensorDescr);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of clusters
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterChargeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster charge in U") % sensorDescr);
    m_clusterChargeU[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("charge of u clusters [ke-]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster charge in V") % sensorDescr);
    m_clusterChargeV[i] = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("charge of v clusters [ke-]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of strips
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_StripChargeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in U") % sensorDescr);
    m_stripSignalU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 256);
    m_stripSignalU[i]->GetXaxis()->SetTitle("charge of u strips [ADU]");
    m_stripSignalU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_StripChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in V") % sensorDescr);
    m_stripSignalV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 256);
    m_stripSignalV[i]->GetXaxis()->SetTitle("charge of v strips [ADU]");
    m_stripSignalV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster size distribution
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster size in U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster size in V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster time distribution
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterTimeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster time in U") % sensorDescr);
    m_clusterTimeU[i] = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("time of u clusters [ns]");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterTimeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster time in V") % sensorDescr);
    m_clusterTimeV[i] = new TH1F(name.c_str(), title.c_str(), TimeBins, TimeMin, TimeMax);
    m_clusterTimeV[i]->GetXaxis()->SetTitle("time of v clusters [ns]");
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

    name = str(format("DQMER_SVD_ClusterChargeU3"));
    title = str(format("DQM ER SVD Cluster charge in U for layer 3 sensors"));
    m_clusterChargeU3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU3->GetXaxis()->SetTitle("charge of u clusters [ke-]");
    m_clusterChargeU3->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_ClusterChargeV3"));
    title = str(format("DQM ER SVD Cluster charge in V for layer 3 sensors"));
    m_clusterChargeV3 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV3->GetXaxis()->SetTitle("charge of v clusters [ke-]");
    m_clusterChargeV3->GetYaxis()->SetTitle("count");

    name = str(format("DQMER_SVD_ClusterChargeU456"));
    title = str(format("DQM ER SVD Cluster charge in U for layers 4,5,6 sensors"));
    m_clusterChargeU456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeU456->GetXaxis()->SetTitle("charge of u clusters [ke-]");
    m_clusterChargeU456->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_ClusterChargeV456"));
    title = str(format("DQM ER SVD Cluster charge in V for layers 4,5,6 sensors"));
    m_clusterChargeV456 = new TH1F(name.c_str(), title.c_str(), ChargeBins, 0, ChargeMax);
    m_clusterChargeV456->GetXaxis()->SetTitle("charge of v clusters [ke-]");
    m_clusterChargeV456->GetYaxis()->SetTitle("count");

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
    StoreArray<SVDDAQDiagnostic> storeDAQDiagnostics(m_svdDAQDiagnosticsListName);
    StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
    StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
    m_storeSVDClustersName = storeSVDClusters.getName();

    m_svdDAQDiagnosticsListName = storeDAQDiagnostics.getName();
    storeSVDClusters.isOptional();
    storeSVDShaperDigits.isOptional();

    //Store names to speed up creation later
    m_storeSVDShaperDigitsName = storeSVDShaperDigits.getName();
  }
}

void SVDDQMExpressRecoModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  // Just to make sure, reset all the histograms.
  if (m_hitMapCountsU != NULL) m_hitMapCountsU->Reset();
  if (m_hitMapCountsV != NULL) m_hitMapCountsV->Reset();
  if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Reset();
  if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Reset();

  if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Reset();

  /*  for (int i = 0; i < c_nFADC; i++) {
    if (m_CounterAPVErrors[i] != NULL) m_CounterAPVErrors[i]->Reset();
    if (m_CounterFTBErrors[i] != NULL) m_CounterFTBErrors[i]->Reset();
    if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Reset();
    if (m_CounterFTBFlags[i] != NULL) m_CounterFTBFlags[i]->Reset();
  }
  */
  if (m_clusterChargeUAll != NULL) m_clusterChargeUAll->Reset();
  if (m_clusterChargeVAll != NULL) m_clusterChargeVAll->Reset();
  if (m_clusterTimeUAll != NULL) m_clusterTimeUAll->Reset();
  if (m_clusterTimeVAll != NULL) m_clusterTimeVAll->Reset();
  for (int i = 0; i < gTools->getNumberOfSVDSensors(); i++) {
    if (m_firedU[i] != NULL) m_firedU[i]->Reset();
    if (m_firedV[i] != NULL) m_firedV[i]->Reset();
    if (m_clustersU[i] != NULL) m_clustersU[i]->Reset();
    if (m_clustersV[i] != NULL) m_clustersV[i]->Reset();
    if (m_clusterChargeU[i] != NULL) m_clusterChargeU[i]->Reset();
    if (m_clusterChargeV[i] != NULL) m_clusterChargeV[i]->Reset();
    if (m_stripSignalU[i] != NULL) m_stripSignalU[i]->Reset();
    if (m_stripSignalV[i] != NULL) m_stripSignalV[i]->Reset();
    if (m_clusterSizeU[i] != NULL) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != NULL) m_clusterSizeV[i]->Reset();
    if (m_clusterTimeU[i] != NULL) m_clusterTimeU[i]->Reset();
    if (m_clusterTimeV[i] != NULL) m_clusterTimeV[i]->Reset();
  }
  if (m_ShowAllHistos == 1) {
    if (m_clusterChargeU3 != NULL) m_clusterChargeU3->Reset();
    if (m_clusterChargeV3 != NULL) m_clusterChargeV3->Reset();
    if (m_clusterChargeU456 != NULL) m_clusterChargeU456->Reset();
    if (m_clusterChargeV456 != NULL) m_clusterChargeV456->Reset();
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
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfSVDLayers() == 0) return;

  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDAQDiagnostic> storeDAQDiagnostics(m_svdDAQDiagnosticsListName);

  //vector<SVDDAQDiagnostic*> diagnosticVector;
  //SVDDAQDiagnostic* currentDAQDiagnostic;
  //map<SVDShaperDigit, SVDDAQDiagnostic*> diagnosticMap;

  if (!storeSVDShaperDigits || !storeSVDShaperDigits.getEntries()) {
    return;
  }

  int firstSVDLayer = gTools->getFirstSVDLayer();
  int lastSVDLayer = gTools->getLastSVDLayer();
  int nSVDSensors = gTools->getNumberOfSVDSensors();

  // SVD basic histograms:
  // DAQ Diagnostic
  /*
  // if object exist and is mapped on expected size:
  if (storeDAQDiagnostics.getEntries() == c_nFADC) {
    int i = 0;
    for (auto& DAQDiag : storeDAQDiagnostics) {
      // for (int i = 0; i < c_nFADC; i++) {
      if (m_CounterAPVErrors[i] != NULL) m_CounterAPVErrors[i]->Fill((int)(DAQDiag.getAPVError()));
      if (m_CounterFTBErrors[i] != NULL) m_CounterFTBErrors[i]->Fill((int)DAQDiag.getFTBError());
      int APVErr = (int)DAQDiag.getAPVErrorOR();
      if (APVErr == 0)
        if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Fill(1);
      if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Fill(2, APVErr & 1);
      if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Fill(3, (APVErr & 2) >> 1);
      if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Fill(4, (APVErr & 4) >> 2);
      if (m_CounterApvErrorORErrors[i] != NULL) m_CounterApvErrorORErrors[i]->Fill(5, (APVErr & 8) >> 3);
      if (m_CounterFTBFlags[i] != NULL) m_CounterFTBFlags[i]->Fill((int)DAQDiag.getFTBFlags());
      i++;
    }
  }
  */

  // Fired strips
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
      uStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip());
      int indexChip = gTools->getSVDChipIndex(sensorID, kTRUE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalU[index] != NULL) m_stripSignalU[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          if (m_hitMapCountsU != NULL) m_hitMapCountsU->Fill(index);
          if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
          if (m_ShowAllHistos == 1) {
            if (m_hitMapU[index] != NULL) m_hitMapU[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
    } else {
      vStrips.at(index).insert(digitIn.getCellID());
      int Chip = (int)(digitIn.getCellID() / gTools->getSVDChannelsPerChip());
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE, Chip);
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalV[index] != NULL) m_stripSignalV[index]->Fill(samples[i]);
        if (samples[i] > m_CutSVDCharge) {
          if (m_hitMapCountsV != NULL) m_hitMapCountsV->Fill(index);
          if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
          if (m_ShowAllHistos == 1) {
            if (m_hitMapV[index] != NULL) m_hitMapV[index]->Fill(digitIn.getCellID(), i);
          }
        }
      }
    }
  }
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_firedU[i] != NULL) && (uStrips[i].size() > 0))
      m_firedU[i]->Fill(uStrips[i].size());
    if ((m_firedV[i] != NULL) && (vStrips[i].size() > 0))
      m_firedV[i]->Fill(vStrips[i].size());
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
                                              (int)(SensorInfo.getUCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()));
      if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Fill(index);
      if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeU[index] != NULL) m_clusterChargeU[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterChargeUAll != NULL) m_clusterChargeUAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getSize());
      if (m_clusterTimeU[index] != NULL) m_clusterTimeU[index]->Fill(cluster.getClsTime());
      if (m_clusterTimeUAll != NULL) m_clusterTimeUAll->Fill(cluster.getClsTime());
      if (m_ShowAllHistos == 1) {
        if (iLayer == 3) if (m_clusterChargeU3 != NULL) m_clusterChargeU3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (iLayer != 3) if (m_clusterChargeU456 != NULL) m_clusterChargeU456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(SensorInfo.getUCellID(cluster.getPosition()));
      }
    } else {
      countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
      int indexChip = gTools->getSVDChipIndex(sensorID, kFALSE,
                                              (int)(SensorInfo.getVCellID(cluster.getPosition()) / gTools->getSVDChannelsPerChip()));
      if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Fill(index);
      if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
      if (m_clusterChargeV[index] != NULL) m_clusterChargeV[index]->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterChargeVAll != NULL) m_clusterChargeVAll->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
      if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getSize());
      if (m_clusterTimeV[index] != NULL) m_clusterTimeV[index]->Fill(cluster.getClsTime());
      if (m_clusterTimeVAll != NULL) m_clusterTimeVAll->Fill(cluster.getClsTime());
      if (m_ShowAllHistos == 1) {
        if (iLayer == 3) if (m_clusterChargeV3 != NULL) m_clusterChargeV3->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (iLayer != 3) if (m_clusterChargeV456 != NULL) m_clusterChargeV456->Fill(cluster.getCharge() / 1000.0);  // in kelectrons
        if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(SensorInfo.getVCellID(cluster.getPosition()));
      }
    }
  }
  for (int i = 0; i < nSVDSensors; i++) {
    if ((m_clustersU[i] != NULL) && (countsU[i].size() > 0))
      m_clustersU[i]->Fill(countsU[i].size());
    if ((m_clustersV[i] != NULL) && (countsV[i].size() > 0))
      m_clustersV[i]->Fill(countsV[i].size());
  }
}
