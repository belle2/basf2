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

#include "svd/modules/svdDQM/SVDDQMExpressRecoMinModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDAQDiagnostic.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMExpressRecoMin)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMExpressRecoMinModule::SVDDQMExpressRecoMinModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module for Express Reco"
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("CutSVDCharge", m_CutSVDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 22 ", m_CutSVDCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDExpReco"));
}


SVDDQMExpressRecoMinModule::~SVDDQMExpressRecoMinModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMExpressRecoMinModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // basic constants presets:
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  c_nVXDLayers = geo.getLayers().size();
  c_firstVXDLayer = 1;  // counting start from 1...
  c_lastVXDLayer = c_nVXDLayers;
  c_nPXDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size();
  c_firstPXDLayer = c_firstVXDLayer;
  c_lastPXDLayer = c_nPXDLayers;
  c_nSVDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::SVD).size();
  c_firstSVDLayer = c_nPXDLayers + c_firstPXDLayer;
  c_lastSVDLayer = c_firstSVDLayer + c_nSVDLayers;

  c_nSVDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() > c_lastPXDLayer) {  // SVD
        c_nSVDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  // Create basic histograms:
  m_hitMapCountsU = new TH1I("DQMER_SVD_StripHitmapCountsU", "DQM ER SVD U Strip Hitmaps Counts",
                             c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapCountsV = new TH1I("DQMER_SVD_StripHitmapCountsV", "DQM ER SVD V Strip Hitmaps Counts",
                             c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsU = new TH1I("DQMER_SVD_ClusterHitmapCountsU", "DQM ER SVD U Cluster Hitmaps Counts",
                               c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsV = new TH1I("DQMER_SVD_ClusterHitmapCountsV", "DQM ER SVD V Cluster Hitmaps Counts",
                               c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");

  m_firedU = new TH1F*[c_nSVDSensors];
  m_firedV = new TH1F*[c_nSVDSensors];
  m_clustersU = new TH1F*[c_nSVDSensors];
  m_clustersV = new TH1F*[c_nSVDSensors];

  m_clusterChargeU = new TH1F*[c_nSVDSensors];
  m_clusterChargeV = new TH1F*[c_nSVDSensors];
  m_stripSignalU = new TH1F*[c_nSVDSensors];
  m_stripSignalV = new TH1F*[c_nSVDSensors];
  m_clusterSizeU = new TH1F*[c_nSVDSensors];
  m_clusterSizeV = new TH1F*[c_nSVDSensors];
  m_clusterTimeU = new TH1F*[c_nSVDSensors];
  m_clusterTimeV = new TH1F*[c_nSVDSensors];

  for (int i = 0; i < c_nSVDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired strips per sensor
    //----------------------------------------------------------------
    string name = str(format("DQMER_SVD_%1%_FiredU") % sensorDescr);
    string title = str(format("DQM ER SVD Sensor %1% Fired strips in U") % sensorDescr);
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
    m_clusterChargeU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 300000);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("charge of u clusters [el]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster charge in V") % sensorDescr);
    m_clusterChargeV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 300000);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("charge of v clusters [el]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of strips
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_StripChargeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in U") % sensorDescr);
    m_stripSignalU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
    m_stripSignalU[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
    m_stripSignalU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_StripChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in V") % sensorDescr);
    m_stripSignalV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
    m_stripSignalV[i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
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
    m_clusterTimeU[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("time of u clusters [ns]");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterTimeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster time in V") % sensorDescr);
    m_clusterTimeV[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_clusterTimeV[i]->GetXaxis()->SetTitle("time of v clusters [ns]");
    m_clusterTimeV[i]->GetYaxis()->SetTitle("count");
  }

  for (int i = 0; i < c_nSVDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, iLayer, iLadder, iSensor);
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  oldDir->cd();
}


void SVDDQMExpressRecoMinModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDDAQDiagnostic> storeDAQDiagnostics(m_svdDAQDiagnosticsListName);
  StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  m_storeSVDClustersName = storeSVDClusters.getName();

  m_svdDAQDiagnosticsListName = storeDAQDiagnostics.getName();
  storeSVDClusters.registerInDataStore();
  storeSVDShaperDigits.isRequired();

  //Store names to speed up creation later
  m_storeSVDShaperDigitsName = storeSVDShaperDigits.getName();

}

void SVDDQMExpressRecoMinModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (m_hitMapCountsU != NULL) m_hitMapCountsU->Reset();
  if (m_hitMapCountsV != NULL) m_hitMapCountsV->Reset();
  if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Reset();
  if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Reset();
  for (int i = 0; i < c_nSVDSensors; i++) {
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
}


void SVDDQMExpressRecoMinModule::event()
{

  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDAQDiagnostic> DAQDiagnostics(m_svdDAQDiagnosticsListName);

  //vector<SVDDAQDiagnostic*> diagnosticVector;
  //SVDDAQDiagnostic* currentDAQDiagnostic;
  //map<SVDShaperDigit, SVDDAQDiagnostic*> diagnosticMap;

  if (!storeSVDShaperDigits || !storeSVDShaperDigits.getEntries()) {
    return;
  }

  // SVD basic histograms:
  // DAQ Diagnostic
  //SVDDAQDiagnostic SVDDiag;
  //SVDDiag->getAPVError();
//  for (const SVDShaperDigit& digitIn : storeSVDShaperDigits) {
  //printf("--> kuk \n");
//    printf("--> kuk %i \n", DAQDiagnostics.size());
//   for (auto& DAQDiag : DAQDiagnostics) {
  //printf("--> kuk1 %i \n", (int)DAQDiag.getAPVError());
//   }
//    for (auto& DAQDiag : diagnosticMap) {
//      DAQDiag.getAPVError();
//    }
//  }

  // Fired strips
  vector< set<int> > uStrips(c_nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDSensors);
  for (const SVDShaperDigit& digitIn : storeSVDShaperDigits) {
    int iLayer = digitIn.getSensorID().getLayerNumber();
    if ((iLayer < c_firstSVDLayer) || (iLayer > c_lastSVDLayer)) continue;
    int iLadder = digitIn.getSensorID().getLadderNumber();
    int iSensor = digitIn.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (digitIn.isUStrip()) {
      uStrips.at(index).insert(digitIn.getCellID());
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalU[index] != NULL) m_stripSignalU[index]->Fill(samples[i]);
        if ((m_hitMapCountsU != NULL) && (samples[i] > m_CutSVDCharge))
          m_hitMapCountsU->Fill(index);
      }
    } else {
      vStrips.at(index).insert(digitIn.getCellID());
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalV[index] != NULL) m_stripSignalV[index]->Fill(samples[i]);
        if ((m_hitMapCountsV != NULL) && (samples[i] > m_CutSVDCharge))
          m_hitMapCountsV->Fill(index);
      }
    }
  }
  for (int i = 0; i < c_nSVDSensors; i++) {
    if ((m_firedU[i] != NULL) && (uStrips[i].size() > 0))
      m_firedU[i]->Fill(uStrips[i].size());
    if ((m_firedV[i] != NULL) && (vStrips[i].size() > 0))
      m_firedV[i]->Fill(vStrips[i].size());
  }

  vector< set<int> > countsU(c_nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > countsV(c_nSVDSensors);
  // Hitmaps, Charge, Seed, Size, Time, ...
  for (const SVDCluster& cluster : storeSVDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < c_firstSVDLayer) || (iLayer > c_lastSVDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (cluster.isUCluster()) {
      countsU.at(index).insert(SensorInfo.getUCellID(cluster.getPosition()));
      if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Fill(index);
      if (m_clusterChargeU[index] != NULL) m_clusterChargeU[index]->Fill(cluster.getCharge());
      if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getSize());
      if (m_clusterTimeU[index] != NULL) m_clusterTimeU[index]->Fill(cluster.getClsTime());
    } else {
      countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
      if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Fill(index);
      if (m_clusterChargeV[index] != NULL) m_clusterChargeV[index]->Fill(cluster.getCharge());
      if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getSize());
      if (m_clusterTimeV[index] != NULL) m_clusterTimeV[index]->Fill(cluster.getClsTime());
    }
  }
  for (int i = 0; i < c_nSVDSensors; i++) {
    if ((m_clustersU[i] != NULL) && (countsU[i].size() > 0))
      m_clustersU[i]->Fill(countsU[i].size());
    if ((m_clustersV[i] != NULL) && (countsV[i].size() > 0))
      m_clustersV[i]->Fill(countsV[i].size());
  }
}


void SVDDQMExpressRecoMinModule::endRun()
{
}

void SVDDQMExpressRecoMinModule::terminate()
{
}

int SVDDQMExpressRecoMinModule::getSensorIndex(int Layer, int Ladder, int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          return tempcounter;
        }
        tempcounter++;
      }
    }
  }
  return tempcounter;
}

void SVDDQMExpressRecoMinModule::getIDsFromIndex(int Index, int& Layer, int& Ladder, int& Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          return;
        }
        tempcounter++;
      }
    }
  }
}
