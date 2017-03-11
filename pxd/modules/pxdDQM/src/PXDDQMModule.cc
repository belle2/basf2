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

#include "pxd/modules/pxdDQM/PXDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

//#include <set>
#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMModule::PXDDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("UseDigits", m_UsePixels,
           "flag <0,1> for using digits only, no cluster information will be required, default = 0 ", m_UsePixels);
  addParam("SaveOtherHistos", m_SaveOtherHistos,
           "flag <0,1> for creation of advance plots, default = 0 ", m_SaveOtherHistos);
  addParam("CutPXDCharge", m_CutPXDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 22 ", m_CutPXDCharge);

}


PXDDQMModule::~PXDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;

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

  c_nPXDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        c_nPXDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  TDirectory* DirPXDBasic = NULL;
  TDirectory* DirPXDAdvance = NULL;
  DirPXDBasic = oldDir->mkdir("PXD_Basic");
  if (m_SaveOtherHistos) {
    DirPXDAdvance = oldDir->mkdir("PXD_Advance");
  }

  m_fired = new TH1F*[c_nPXDSensors];
  m_clusters = new TH1F*[c_nPXDSensors];
  m_hitMapU = new TH1F*[c_nPXDSensors];
  m_hitMapV = new TH1F*[c_nPXDSensors];
  m_hitMap = new TH2F*[c_nPXDSensors];
  m_hitMapUCl = new TH1F*[c_nPXDSensors];
  m_hitMapVCl = new TH1F*[c_nPXDSensors];
  m_hitMapCl = new TH2F*[c_nPXDSensors];
  m_charge = new TH1F*[c_nPXDSensors];
  m_chargePix = new TH1F*[c_nPXDSensors];
  m_seed = new TH1F*[c_nPXDSensors];
  m_sizeU = new TH1F*[c_nPXDSensors];
  m_sizeV = new TH1F*[c_nPXDSensors];
  m_size = new TH1F*[c_nPXDSensors];
  m_startRow = new TH1F*[c_nPXDSensors];
  m_chargStartRow = new TH1F*[c_nPXDSensors];
  m_StartRowCount = new TH1F*[c_nPXDSensors];
  for (int i = 0; i < c_nPXDSensors; i++) {
    m_fired[i] = NULL;
    m_clusters[i] = NULL;
    m_hitMapU[i] = NULL;
    m_hitMapV[i] = NULL;
    m_hitMap[i] = NULL;
    m_hitMapUCl[i] = NULL;
    m_hitMapVCl[i] = NULL;
    m_hitMapCl[i] = NULL;
    m_charge[i] = NULL;
    m_chargePix[i] = NULL;
    m_seed[i] = NULL;
    m_sizeU[i] = NULL;
    m_sizeV[i] = NULL;
    m_size[i] = NULL;
    m_startRow[i] = NULL;
    m_chargStartRow[i] = NULL;
    m_StartRowCount[i] = NULL;
  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    DirPXDBasic->cd();
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);


    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    string name = str(format("PXD_%1%_Fired") % sensorDescr);
    string title = str(format("PXD Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired u pixels");
    m_fired[i]->GetYaxis()->SetTitle("count");
    if (!m_UsePixels) {
      //----------------------------------------------------------------
      // Number of clusters per frame
      //----------------------------------------------------------------
      name = str(format("PXD_%1%_Clusters") % sensorDescr);
      title = str(format("PXD Sensor %1% Number of clusters") % sensorDescr);
      m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clusters[i]->GetXaxis()->SetTitle("# of u clusters");
      m_clusters[i]->GetYaxis()->SetTitle("count");
    }

    if (m_SaveOtherHistos) {
      DirPXDAdvance->cd();
      //----------------------------------------------------------------
      // Hitmaps: Number of pixels by coordinate
      //----------------------------------------------------------------
      // Hitmaps in U
      name = str(format("PXD_%1%_PixelHitmapU") % sensorDescr);
      title = str(format("PXD Sensor %1% Pixel Hitmap in U") % sensorDescr);
      int nPixels = SensorInfo.getUCells();
      m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
      m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapU[i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("PXD_%1%_PixelHitmapV") % sensorDescr);
      title = str(format("PXD Sensor %1% Pixel Hitmap in V") % sensorDescr);
      nPixels = SensorInfo.getVCells();
      m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
      m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapV[i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in UV
      name = str(format("PXD_%1%_PixelHitmap") % sensorDescr);
      title = str(format("PXD Sensor %1% Pixel Hitmap") % sensorDescr);
      nPixels = SensorInfo.getUCells();
      int nPixelsV = SensorInfo.getVCells();
      m_hitMap[i] = new TH2F(name.c_str(), title.c_str(), nPixels, 0, nPixels, nPixelsV, 0, nPixelsV);
      m_hitMap[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMap[i]->GetYaxis()->SetTitle("v position [pitch units]");
      m_hitMap[i]->GetZaxis()->SetTitle("hits");

      //----------------------------------------------------------------
      // Start row distribution
      //----------------------------------------------------------------
      name = str(format("PXD_%1%_StartRow") % sensorDescr);
      title = str(format("PXD Sensor %1% Start row distribution") % sensorDescr);
      nPixels = SensorInfo.getVCells();
      m_startRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
      m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
      m_startRow[i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster seed charge by distance from the start row
      //----------------------------------------------------------------
      name = str(format("PXD_%1%_AverageSeedByStartRow") % sensorDescr);
      title = str(format("PXD Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
      m_chargStartRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
      m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
      m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
      name = str(format("PXD_%1%_SeedCountsByStartRow") % sensorDescr);
      title = str(format("PXD Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
      m_StartRowCount[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
      m_StartRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
      m_StartRowCount[i]->GetYaxis()->SetTitle("count");

    }
    DirPXDBasic->cd();
    if (!m_UsePixels) {
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate
      //----------------------------------------------------------------
      // Hitmaps in U
      name = str(format("PXD_%1%_HitmapClstU") % sensorDescr);
      title = str(format("PXD Sensor %1% Hitmap Clusters in U") % sensorDescr);
      int nPixels = SensorInfo.getUCells();
      m_hitMapUCl[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
      m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("PXD_%1%_HitmapClstV") % sensorDescr);
      title = str(format("PXD Sensor %1% Hitmap Clusters in V") % sensorDescr);
      nPixels = SensorInfo.getVCells();
      m_hitMapVCl[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
      m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in UV
      name = str(format("PXD_%1%_HitmapClst") % sensorDescr);
      title = str(format("PXD Sensor %1% Hitmap Clusters") % sensorDescr);
      nPixels = SensorInfo.getUCells();
      int nPixelsV = SensorInfo.getVCells();
      m_hitMapCl[i] = new TH2F(name.c_str(), title.c_str(), nPixels, 0, nPixels, nPixelsV, 0, nPixelsV);
      m_hitMapCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapCl[i]->GetYaxis()->SetTitle("v position [pitch units]");
      m_hitMapCl[i]->GetZaxis()->SetTitle("hits");
      //----------------------------------------------------------------
      // Charge of clusters
      //----------------------------------------------------------------
      name = str(format("PXD_%1%_ClusterCharge") % sensorDescr);
      title = str(format("PXD Sensor %1% Cluster charge") % sensorDescr);
      m_charge[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
      m_charge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
      m_charge[i]->GetYaxis()->SetTitle("count");
    }
    if (m_SaveOtherHistos) {
      DirPXDAdvance->cd();
      //----------------------------------------------------------------
      // Charge of pixels
      //----------------------------------------------------------------
      name = str(format("PXD_%1%_PixelCharge") % sensorDescr);
      title = str(format("PXD Sensor %1% Pixel charge") % sensorDescr);
      m_chargePix[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
      m_chargePix[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
      m_chargePix[i]->GetYaxis()->SetTitle("count");
    }
    DirPXDBasic->cd();
    if (!m_UsePixels) {
      if (m_SaveOtherHistos) {
        DirPXDAdvance->cd();
        //----------------------------------------------------------------
        // Cluster seed charge distribution
        //----------------------------------------------------------------
        name = str(format("PXD_%1%_Seed") % sensorDescr);
        title = str(format("PXD Sensor %1% Seed charge") % sensorDescr);
        m_seed[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
        m_seed[i]->GetXaxis()->SetTitle("seed charge of clusters [ADU]");
        m_seed[i]->GetYaxis()->SetTitle("count");
      }
      DirPXDBasic->cd();
      //----------------------------------------------------------------
      // Cluster size distribution : hClusterSize[U/V][LayerNo]
      //----------------------------------------------------------------
      // u size by sensor
      name = str(format("PXD_%1%_ClusterSizeU") % sensorDescr);
      title = str(format("PXD Sensor %1% Cluster size in U") % sensorDescr);
      m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeU[i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizeU[i]->GetYaxis()->SetTitle("count");
      // v size by sensor
      name = str(format("PXD_%1%_ClusterSizeV") % sensorDescr);
      title = str(format("PXD Sensor %1% Cluster size in V") % sensorDescr);
      m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeV[i]->GetXaxis()->SetTitle("size of v clusters");
      m_sizeV[i]->GetYaxis()->SetTitle("count");
      // cluster size by sensor
      name = str(format("PXD_%1%_ClusterSize") % sensorDescr);
      title = str(format("PXD Sensor %1% Cluster size") % sensorDescr);
      m_size[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_size[i]->GetXaxis()->SetTitle("size of clusters");
      m_size[i]->GetYaxis()->SetTitle("count");
    }
  }

  oldDir->cd();
}


void PXDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  m_storePXDClustersName = storePXDClusters.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

  //Store names to speed up creation later
  m_storePXDDigitsName = storePXDDigits.getName();

  StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  m_storeFramesName = storeFrames.getName();

}

void PXDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nPXDSensors; i++) {
    if (m_fired[i] != NULL) m_fired[i]->Reset();
    if (m_clusters[i] != NULL) m_clusters[i]->Reset();
    if (m_hitMapU[i] != NULL) m_hitMapU[i]->Reset();
    if (m_hitMapV[i] != NULL) m_hitMapV[i]->Reset();
    if (m_hitMap[i] != NULL) m_hitMap[i]->Reset();
    if (m_hitMapUCl[i] != NULL) m_hitMapUCl[i]->Reset();
    if (m_hitMapVCl[i] != NULL) m_hitMapVCl[i]->Reset();
    if (m_hitMapCl[i] != NULL) m_hitMapCl[i]->Reset();
    if (m_charge[i] != NULL) m_charge[i]->Reset();
    if (m_chargePix[i] != NULL) m_chargePix[i]->Reset();
    if (m_seed[i] != NULL) m_seed[i]->Reset();
    if (m_sizeU[i] != NULL) m_sizeU[i]->Reset();
    if (m_sizeV[i] != NULL) m_sizeV[i]->Reset();
    if (m_size[i] != NULL) m_size[i]->Reset();
    if (m_startRow[i] != NULL) m_startRow[i]->Reset();
    if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Reset();
    if (m_StartRowCount[i] != NULL) m_StartRowCount[i]->Reset();
  }
}


void PXDDQMModule::event()
{
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  // PXD basic histograms:
  // Fired strips
  vector< set<int> > Pixels(c_nPXDSensors); // sets to eliminate multiple samples per strip
  for (const PXDDigit& digit : storePXDDigits) {
    int iLayer = digit.getSensorID().getLayerNumber();
    if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
    int iLadder = digit.getSensorID().getLadderNumber();
    int iSensor = digit.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels.at(index).insert(digit.getUniqueChannelID());
    if (m_chargePix[index] != NULL) m_chargePix[index]->Fill(digit.getCharge());
    if ((m_hitMapU[index] != NULL) && (digit.getCharge() > m_CutPXDCharge))
      m_hitMapU[index]->Fill(digit.getUCellID());
    if ((m_hitMapV[index] != NULL) && (digit.getCharge() > m_CutPXDCharge))
      m_hitMapV[index]->Fill(digit.getVCellID());
    if ((m_hitMap[index] != NULL) && (digit.getCharge() > m_CutPXDCharge))
      m_hitMap[index]->Fill(digit.getUCellID(), digit.getVCellID());
  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    if ((m_fired[i] != NULL) && (Pixels[i].size() > 0)) m_fired[i]->Fill(Pixels[i].size());
  }

  vector< set<int> > counts(c_nPXDSensors);
  if (!m_UsePixels) {
    // Hitmaps, Charge, Seed, Size, Time
    for (const PXDCluster& cluster : storePXDClusters) {
      int iLayer = cluster.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = cluster.getSensorID().getLadderNumber();
      int iSensor = cluster.getSensorID().getSensorNumber();
      int index = getSensorIndex(iLayer, iLadder, iSensor);
      VxdID sensorID(iLayer, iLadder, iSensor);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      counts.at(index).insert(cluster.GetUniqueID());
      if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(
          SensorInfo.getUCellID(cluster.getU()));
      if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(
          SensorInfo.getVCellID(cluster.getV()));
      if (m_hitMapCl[index] != NULL) m_hitMapCl[index]->Fill(
          SensorInfo.getUCellID(cluster.getU()),
          SensorInfo.getVCellID(cluster.getV()));
      if (m_charge[index] != NULL) m_charge[index]->Fill(cluster.getCharge());
      if (m_seed[index] != NULL) m_seed[index]->Fill(cluster.getSeedCharge());
      if (m_sizeU[index] != NULL) m_sizeU[index]->Fill(cluster.getUSize());
      if (m_sizeV[index] != NULL) m_sizeV[index]->Fill(cluster.getVSize());
      if (m_size[index] != NULL) m_size[index]->Fill(cluster.getSize());
    }
    for (int i = 0; i < c_nPXDSensors; i++) {
      if ((m_clusters[i] != NULL) && (counts[i].size() > 0))
        m_clusters[i]->Fill(counts[i].size());
    }
  }
  if (storeFrames && storeFrames.getEntries()) {
    // Start rows

    for (const PXDFrame& frame : storeFrames) {
      int iLayer = frame.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = frame.getSensorID().getLadderNumber();
      int iSensor = frame.getSensorID().getSensorNumber();
      int index = getSensorIndex(iLayer, iLadder, iSensor);
      if (m_startRow[index] != NULL) m_startRow[index]->Fill(frame.getStartRow());
    }
    // Cluster seed charge by start row
    std::map<VxdID, unsigned short> startRows;
    for (auto frame : storeFrames)
      startRows.insert(std::make_pair(frame.getSensorID(), frame.getStartRow()));
    for (auto cluster : storePXDClusters) {
      int iLayer = cluster.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = cluster.getSensorID().getLadderNumber();
      int iSensor = cluster.getSensorID().getSensorNumber();
      int index = getSensorIndex(iLayer, iLadder, iSensor);
      VxdID sensorID(iLayer, iLadder, iSensor);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      float fDistance = SensorInfo.getVCellID(cluster.getV()) - startRows[cluster.getSensorID()];
      if (fDistance < 0) fDistance += SensorInfo.getVCells();
      if (m_chargStartRow[index] != NULL) m_chargStartRow[index]->Fill(fDistance, cluster.getSeedCharge());
      if (m_StartRowCount[index] != NULL) m_StartRowCount[index]->Fill(fDistance);
      //m_averageSeedByV[index]->Fill(SensorInfo.getVCellID(cluster.getV()), cluster.getSeedCharge());
      //m_seedCountsByV[index]->Fill(SensorInfo.getVCellID(cluster.getV()));
      //m_averageSeedByU[index]->Fill(SensorInfo.getUCellID(cluster.getU()), cluster.getSeedCharge());
      //m_seedCountsByU[index]->Fill(SensorInfo.getUCellID(cluster.getU()));
    }

  }
}


void PXDDQMModule::endRun()
{
  // Make average value on histogram
  for (int i = 0; i < c_nPXDSensors; i++) {
    if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Divide(m_StartRowCount[i]);
    //m_averageSeedByU[i]->Divide(m_seedCountsByU[i]);
    //m_averageSeedByV[i]->Divide(m_seedCountsByV[i]);
  }
}


void PXDDQMModule::terminate()
{
}

int PXDDQMModule::getSensorIndex(int Layer, int Ladder, int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  // printf("  --> PXD uvnitr sensor %i: %i_%i_%i\n", tempcounter, Layer, Ladder, Sensor);
  return tempcounter;
}

void PXDDQMModule::getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          *Layer = layer.getLayerNumber();
          *Ladder = ladder.getLadderNumber();
          *Sensor = sensor.getSensorNumber();
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  // printf("  --> PXD sensor %i: %i_%i_%i\n", Index, *Layer, *Ladder, *Sensor);
}
