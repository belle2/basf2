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

#include "pxd/modules/pxdDQM/PXDDQMClustersModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <pxd/unpacking/PXDMappingLookup.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>

#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TVectorD.h"
#include "TF1.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMClusters)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMClustersModule::PXDDQMClustersModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM module "
                 "Recommended Number of events for monito is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CutPXDCharge", m_CutPXDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 0.0 ", m_CutPXDCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDDQM"));
}


PXDDQMClustersModule::~PXDDQMClustersModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMClustersModule::defineHisto()
{
  if (m_utils->getNumberOfLayers() == 0) {
    B2FATAL("Missing geometry for VXD, check steering file.");
  }
  if (m_utils->getNumberOfPXDLayers() != 0) {
    B2WARNING("Missing geometry for PXD, PXD-DQM is skiped.");
    return;
  }

  // Create a separate histogram directories and cd into it.
  m_oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    m_oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    m_oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // basic constants presets:
  int c_firstPXDLayer = m_utils->getFirstPXDLayer();
  int c_nPXDSensors = m_utils->getNumberOfPXDSensors();
  int c_nPXDChips =  c_nPXDSensors * (m_utils->getNumberOfPXDUSideChips(c_firstPXDLayer) + m_utils->getNumberOfPXDVSideChips(
                                        c_firstPXDLayer));

  // Create basic histograms:
  m_hitMapCounts = new TH1I("DQM_PXD_PixelHitmapCounts", "DQM PXD Integrated number of fired pixels per sensor",
                            c_nPXDSensors, 0, c_nPXDSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");
  m_hitMapClCounts = new TH1I("DQM_PXD_ClusterHitmapCounts", "DQM PXD Integrated number of clusters per sensor",
                              c_nPXDSensors, 0, c_nPXDSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");
  // basic counters per chip:
  m_hitMapCountsChip = new TH1I("DQM_PXD_PixelHitmapCountsChip", "DQM PXD Integrated number of fired pixels per chip",
                                c_nPXDChips, 0, c_nPXDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1I("DQM_PXD_ClusterHitmapCountsChip", "DQM PXD Integrated number of clusters per chip",
                                  c_nPXDChips, 0, c_nPXDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  for (int i = 0; i < c_nPXDChips; i++) {
    VxdID id = m_utils->getChipIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    int iChip = m_utils->getPXDChipNumber(id);
    int IsU = m_utils->isPXDSideU(id);
    TString AxisTicks = Form("%i_%i_%i_u%iDCD", iLayer, iLadder, iSensor, iChip);
    if (!IsU)
      AxisTicks = Form("%i_%i_%i_v%iSWB", iLayer, iLadder, iSensor, iChip);
    m_hitMapCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  for (int i = 0; i < c_nPXDSensors; i++) {
    VxdID id = m_utils->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  m_fired = new TH1F*[c_nPXDSensors];
  m_clusters = new TH1F*[c_nPXDSensors];
  m_startRow = new TH1F*[c_nPXDSensors];
  m_chargStartRow = new TH1F*[c_nPXDSensors];
  m_startRowCount = new TH1F*[c_nPXDSensors];
  m_clusterCharge = new TH1F*[c_nPXDSensors];
  m_pixelSignal = new TH1F*[c_nPXDSensors];
  m_clusterSizeU = new TH1F*[c_nPXDSensors];
  m_clusterSizeV = new TH1F*[c_nPXDSensors];
  m_clusterSizeUV = new TH1F*[c_nPXDSensors];

  m_hitMapU = new TH1F*[c_nPXDSensors];
  m_hitMapV = new TH1F*[c_nPXDSensors];
  m_hitMap = new TH2F*[c_nPXDSensors];
  m_hitMapUCl = new TH1F*[c_nPXDSensors];
  m_hitMapVCl = new TH1F*[c_nPXDSensors];
  m_hitMapCl = new TH2F*[c_nPXDSensors];
  m_seed = new TH1F*[c_nPXDSensors];
  for (int i = 0; i < c_nPXDSensors; i++) {
    VxdID id = m_utils->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    string name = str(format("DQM_PXD_%1%_Fired") % sensorDescr);
    string title = str(format("DQM PXD Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = NULL;
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired u pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_Clusters") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = NULL;
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_StartRow") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Start row distribution") % sensorDescr);

    int nPixels;/** Number of pixels on PXD v direction */
    nPixels = SensorInfo.getVCells();
    m_startRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_AverageSeedByStartRow") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    m_chargStartRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    name = str(format("DQM_PXD_%1%_SeedCountsByStartRow") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    m_startRowCount[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterCharge") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_PixelSignal") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");

    //----------------------------------------------------------------
    // Hitmaps: Number of pixels by coordinate
    //----------------------------------------------------------------
    // Hitmaps in U
    name = str(format("PXD_%1%_PixelHitmapU") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Hitmap in U") % sensorDescr);
    nPixels = SensorInfo.getUCells();
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
    // Hitmaps: Number of clusters by coordinate
    //----------------------------------------------------------------
    // Hitmaps in U
    name = str(format("PXD_%1%_HitmapClstU") % sensorDescr);
    title = str(format("PXD Sensor %1% Hitmap Clusters in U") % sensorDescr);
    nPixels = SensorInfo.getUCells();
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
    nPixelsV = SensorInfo.getVCells();
    m_hitMapCl[i] = new TH2F(name.c_str(), title.c_str(), nPixels, 0, nPixels, nPixelsV, 0, nPixelsV);
    m_hitMapCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapCl[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMapCl[i]->GetZaxis()->SetTitle("hits");

    //----------------------------------------------------------------
    // Cluster seed charge distribution
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_Seed") % sensorDescr);
    title = str(format("PXD Sensor %1% Seed charge") % sensorDescr);
    m_seed[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_seed[i]->GetXaxis()->SetTitle("seed charge of clusters [ADU]");
    m_seed[i]->GetYaxis()->SetTitle("count");

  }
  m_oldDir->cd();
}


void PXDDQMClustersModule::initialize()
{

  m_utils = new DQMCommonUtils();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  if (m_utils->getNumberOfPXDLayers() != 0) {
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
}

void PXDDQMClustersModule::beginRun()
{
  if (m_utils->getNumberOfPXDLayers() == 0) return;

  if (m_hitMapCounts != NULL) m_hitMapCounts->Reset();
  if (m_hitMapClCounts != NULL) m_hitMapClCounts->Reset();
  if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Reset();

  for (int i = 0; i < m_utils->getNumberOfPXDSensors(); i++) {
    if (m_fired[i] != NULL) m_fired[i]->Reset();
    if (m_clusters[i] != NULL) m_clusters[i]->Reset();
    if (m_startRow[i] != NULL) m_startRow[i]->Reset();
    if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Reset();
    if (m_startRowCount[i] != NULL) m_startRowCount[i]->Reset();
    if (m_clusterCharge[i] != NULL) m_clusterCharge[i]->Reset();
    if (m_pixelSignal[i] != NULL) m_pixelSignal[i]->Reset();
    if (m_clusterSizeU[i] != NULL) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != NULL) m_clusterSizeV[i]->Reset();
    if (m_clusterSizeUV[i] != NULL) m_clusterSizeUV[i]->Reset();

    if (m_hitMapU[i] != NULL) m_hitMapU[i]->Reset();
    if (m_hitMapV[i] != NULL) m_hitMapV[i]->Reset();
    if (m_hitMap[i] != NULL) m_hitMap[i]->Reset();
    if (m_hitMapUCl[i] != NULL) m_hitMapUCl[i]->Reset();
    if (m_hitMapVCl[i] != NULL) m_hitMapVCl[i]->Reset();
    if (m_hitMapCl[i] != NULL) m_hitMapCl[i]->Reset();
    if (m_seed[i] != NULL) m_seed[i]->Reset();

  }
}


void PXDDQMClustersModule::event()
{
  if (m_utils->getNumberOfPXDLayers() == 0) return;

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  int c_firstPXDLayer = m_utils->getFirstPXDLayer();
  int c_lastPXDLayer = m_utils->getLastPXDLayer();
  int c_nPXDSensors = m_utils->getNumberOfPXDSensors();

  // PXD basic histograms:
  // Fired strips
  vector< set<int> > Pixels(c_nPXDSensors); // sets to eliminate multiple samples per strip
  for (const PXDDigit& digit2 : storePXDDigits) {
    int iLayer = digit2.getSensorID().getLayerNumber();
    if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
    int iLadder = digit2.getSensorID().getLadderNumber();
    int iSensor = digit2.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = m_utils->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels.at(index).insert(digit2.getUniqueChannelID());
    int iChip = PXDMappingLookup::getDCDID(digit2.getUCellID(), digit2.getVCellID(), sensorID);
    int indexChip = m_utils->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(digit2.getVCellID());
    indexChip = m_utils->getPXDChipIndex(sensorID, kFALSE, iChip);
    if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);

    if (m_pixelSignal[index] != NULL) m_pixelSignal[index]->Fill(digit2.getCharge());
    if ((m_hitMapCounts != NULL) && (digit2.getCharge() > m_CutPXDCharge))
      m_hitMapCounts->Fill(index);

    if (digit2.getCharge() < m_CutPXDCharge) continue;

    if (m_hitMapU[index] != NULL) m_hitMapU[index]->Fill(digit2.getUCellID());
    if (m_hitMapV[index] != NULL) m_hitMapV[index]->Fill(digit2.getVCellID());
    if (m_hitMap[index] != NULL) m_hitMap[index]->Fill(digit2.getUCellID(), digit2.getVCellID());

  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    if ((m_fired[i] != NULL) && (Pixels[i].size() > 0)) m_fired[i]->Fill(Pixels[i].size());
  }

  vector< set<int> > counts(c_nPXDSensors);
  // Hitmaps, Charge, Seed, Size, ...
  for (const PXDCluster& cluster : storePXDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = m_utils->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    counts.at(index).insert(cluster.GetUniqueID());
    int iChip = PXDMappingLookup::getDCDID(SensorInfo.getUCellID(cluster.getU()), SensorInfo.getVCellID(cluster.getV()), sensorID);
    int indexChip = m_utils->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(SensorInfo.getVCellID(cluster.getV()));
    indexChip = m_utils->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
    if (m_hitMapClCounts != NULL) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != NULL) m_clusterCharge[index]->Fill(cluster.getCharge());
    if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != NULL) m_clusterSizeUV[index]->Fill(cluster.getSize());

    if (m_seed[index] != NULL) m_seed[index]->Fill(cluster.getSeedCharge());
    if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()));
    if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(
        SensorInfo.getVCellID(cluster.getV()));
    if (m_hitMapCl[index] != NULL) m_hitMapCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()),
        SensorInfo.getVCellID(cluster.getV()));

  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    if ((m_clusters[i] != NULL) && (counts[i].size() > 0))
      m_clusters[i]->Fill(counts[i].size());
  }
  if (storeFrames && storeFrames.getEntries()) {
    // Start rows
    for (const PXDFrame& frame : storeFrames) {
      int iLayer = frame.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = frame.getSensorID().getLadderNumber();
      int iSensor = frame.getSensorID().getSensorNumber();
      int index = m_utils->getPXDSensorIndex(iLayer, iLadder, iSensor);
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
      int index = m_utils->getPXDSensorIndex(iLayer, iLadder, iSensor);
      VxdID sensorID(iLayer, iLadder, iSensor);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      float fDistance = SensorInfo.getVCellID(cluster.getV()) - startRows[cluster.getSensorID()];
      if (fDistance < 0) fDistance += SensorInfo.getVCells();
      if (m_chargStartRow[index] != NULL) m_chargStartRow[index]->Fill(fDistance, cluster.getSeedCharge());
      if (m_startRowCount[index] != NULL) m_startRowCount[index]->Fill(fDistance);
    }
  }
}


void PXDDQMClustersModule::endRun()
{
  if (m_utils->getNumberOfPXDLayers() != 0) return;

  // Make average value on histogram
  for (int i = 0; i < m_utils->getNumberOfPXDSensors(); i++) {
//   if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Divide(m_StartRowCount[i]);
    //m_averageSeedByU[i]->Divide(m_seedCountsByU[i]);
    //m_averageSeedByV[i]->Divide(m_seedCountsByV[i]);
  }
  TVectorD* NoOfEvents;
  NoOfEvents = new TVectorD(1);
  double fNoOfEvents[1];
  fNoOfEvents[0] = m_NoOfEvents;
  NoOfEvents->SetElements(fNoOfEvents);
  TString nameBS = Form("DQMER_PXD_NoOfEvents");
  m_oldDir->cd();
  NoOfEvents->Write(nameBS.Data());

  m_oldDir->mkdir("PXD_Refs");
  m_oldDir->cd("PXD_Refs");
//   TDirectory* DirPXDRefs = gDirectory;
  // Load reference file of histograms:
  TVectorD* NoOfEventsRef;
  NoOfEventsRef = new TVectorD(1);
  double fNoOfEventsRef[1];

  int c_nPXDSensors = m_utils->getNumberOfPXDSensors();
  // Create flag histograms:
  m_oldDir->mkdir("PXDExpReco_Flags");
  m_oldDir->cd("PXDExpReco_Flags");
  TDirectory* DirPXDFlags = gDirectory;
  DirPXDFlags->cd();

  m_fFiredFlag = new TH1I("DQMER_PXD_FiredFlag", "DQM ER PXD Fired Flag",
                          c_nPXDSensors, 0, c_nPXDSensors);
  m_fFiredFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fFiredFlag->GetYaxis()->SetTitle("flag");
  m_fClustersFlag = new TH1I("DQMER_PXD_ClustersFlag", "DQM ER PXD Clusters Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fClustersFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClustersFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowFlag = new TH1I("DQMER_PXD_StartRowFlag", "DQM ER PXD Start Row Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fChargStartRowFlag = new TH1I("DQMER_PXD_ChargStartRowFlag", "DQM ER PXD Charg Start Row Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fChargStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fChargStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowCountFlag = new TH1I("DQMER_PXD_StartRowCountFlag", "DQM ER PXD Row Count Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowCountFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowCountFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapCountsFlag = new TH1I("DQMER_PXD_PixelHitmapCountsFlag", "DQM ER PXD Pixel Hitmaps Counts Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fHitMapCountsFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapCountsFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapClCountsFlag = new TH1I("DQMER_PXD_ClusterHitmapCountsFlag", "DQM ER PXD Cluster Hitmaps Counts Flag",
                                   c_nPXDSensors, 0, c_nPXDSensors);
  m_fHitMapClCountsFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapClCountsFlag->GetYaxis()->SetTitle("flag");
  m_fClusterChargeFlag = new TH1I("DQMER_PXD_ClusterChargeFlag", "DQM ER PXD Cluster Charge Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterChargeFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterChargeFlag->GetYaxis()->SetTitle("flag");
  m_fPixelSignalFlag = new TH1I("DQMER_PXD_PixelSignalFlag", "DQM ER PXD Pixel Signal Flag",
                                c_nPXDSensors, 0, c_nPXDSensors);
  m_fPixelSignalFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fPixelSignalFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUFlag = new TH1I("DQMER_PXD_ClasterSizeUFlag", "DQM ER PXD Cluster Size U Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeVFlag = new TH1I("DQMER_PXD_ClasterSizeVFlag", "DQM ER PXD Cluster Size V Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUVFlag = new TH1I("DQMER_PXD_ClasterSizeUVFlag", "DQM ER PXD Cluster Size UV Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUVFlag->GetYaxis()->SetTitle("flag");

  for (int i = 0; i < c_nPXDSensors; i++) {
    VxdID id = m_utils->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_fFiredFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClustersFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fChargStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowCountFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapClCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterChargeFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fPixelSignalFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }


  // Load reference file of histograms:
  TH1I* r_hitMapCounts;
  TH1I* r_hitMapClCounts;
  TH1F** r_fired = new TH1F*[c_nPXDSensors];
  TH1F** r_clusters = new TH1F*[c_nPXDSensors];
  TH1F** r_startRow = new TH1F*[c_nPXDSensors];
  TH1F** r_chargStartRow = new TH1F*[c_nPXDSensors];
  TH1F** r_startRowCount = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterCharge = new TH1F*[c_nPXDSensors];
  TH1F** r_pixelSignal = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeU = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeV = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeUV = new TH1F*[c_nPXDSensors];

  r_hitMapCounts = NULL;
  r_hitMapCounts = new TH1I(*m_hitMapCounts);
  r_hitMapCounts->Reset();
  r_hitMapClCounts = NULL;
  r_hitMapClCounts = new TH1I(*m_hitMapClCounts);
  r_hitMapClCounts->Reset();
  for (int i = 0; i < c_nPXDSensors; i++) {
    r_fired[i] = NULL;
    r_fired[i] = new TH1F(*m_fired[i]);
    r_fired[i]->Reset();
    r_clusters[i] = NULL;
    r_clusters[i] = new TH1F(*m_clusters[i]);
    r_clusters[i]->Reset();
    r_startRow[i] = NULL;
    r_startRow[i] = new TH1F(*m_startRow[i]);
    r_startRow[i]->Reset();
    r_chargStartRow[i] = NULL;
    r_chargStartRow[i] = new TH1F(*m_chargStartRow[i]);
    r_chargStartRow[i]->Reset();
    r_startRowCount[i] = NULL;
    r_startRowCount[i] = new TH1F(*m_startRowCount[i]);
    r_startRowCount[i]->Reset();
    r_clusterCharge[i] = NULL;
    r_clusterCharge[i] = new TH1F(*m_clusterCharge[i]);
    r_clusterCharge[i]->Reset();
    r_pixelSignal[i] = NULL;
    r_pixelSignal[i] = new TH1F(*m_pixelSignal[i]);
    r_pixelSignal[i]->Reset();
    r_clusterSizeU[i] = NULL;
    r_clusterSizeU[i] = new TH1F(*m_clusterSizeU[i]);
    r_clusterSizeU[i]->Reset();
    r_clusterSizeV[i] = NULL;
    r_clusterSizeV[i] = new TH1F(*m_clusterSizeV[i]);
    r_clusterSizeV[i]->Reset();
    r_clusterSizeUV[i] = NULL;
    r_clusterSizeUV[i] = new TH1F(*m_clusterSizeUV[i]);
    r_clusterSizeUV[i]->Reset();
  }

  if (m_NotUseDB == 1) {
    TFile* f_RefHistFile = new TFile(m_RefHistFileName.c_str(), "read");
    if (f_RefHistFile->IsOpen()) {
      B2INFO("Reference file name: " << m_RefHistFileName.c_str());
      TVectorD* NoOfEventsRef2 = NULL;
      f_RefHistFile->GetObject("NoOfEvents", NoOfEventsRef2);
      m_NoOfEventsRef = (int)NoOfEventsRef2->GetMatrixArray()[0];
      //    m_NoOfEventsRef = 2;
      string name = str(format("PXDExpReco/PixelHitmapCounts;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapCounts);
      if (r_hitMapCounts == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/ClusterHitmapCounts;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapClCounts);
      if (r_hitMapClCounts == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      for (int i = 0; i < c_nPXDSensors; i++) {
        VxdID id = m_utils->getSensorIDFromPXDIndex(i);
        int iLayer = id.getLayerNumber();
        int iLadder = id.getLadderNumber();
        int iSensor = id.getSensorNumber();
        string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
        name = str(format("PXDExpReco/PXD_%1%_Fired") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_fired[i]);
        if (r_fired[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_Clusters") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusters[i]);
        if (r_clusters[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_StartRow") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_startRow[i]);
        if (r_startRow[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_AverageSeedByStartRow") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_chargStartRow[i]);
        if (r_chargStartRow[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_SeedCountsByStartRow") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_startRowCount[i]);
        if (r_startRowCount[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_ClusterCharge") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterCharge[i]);
        if (r_clusterCharge[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_PixelSignal") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_pixelSignal[i]);
        if (r_pixelSignal[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_ClusterSizeU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterSizeU[i]);
        if (r_clusterSizeU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_ClusterSizeV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterSizeV[i]);
        if (r_clusterSizeV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("PXDExpReco/PXD_%1%_ClusterSizeUV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterSizeUV[i]);
        if (r_clusterSizeUV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
      }
      // f_RefHistFile->Close();
    } else {
      B2INFO("File of reference histograms: " << m_RefHistFileName.c_str() << " is not available, please check it!");
      return;
    }
  } else {
    if (m_CreateDB == 1) {
      IntervalOfValidity iov(0, 0, -1, -1);
      TString Name = Form("DQMER_PXD_NoOfEvents_Ref");
      DBImportObjPtr<TVectorD> DQMER_PXD_NoOfEvents(Name.Data());
      DQMER_PXD_NoOfEvents.construct(1);
      DQMER_PXD_NoOfEvents->SetElements(fNoOfEvents);
      DQMER_PXD_NoOfEvents.import(iov);

      m_utils->CreateDBHisto(m_hitMapCounts);
      m_utils->CreateDBHisto(m_hitMapClCounts);
      m_utils->CreateDBHistoGroup(m_fired, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_clusters, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_startRow, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_chargStartRow, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_startRowCount, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_clusterCharge, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_pixelSignal, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_clusterSizeU, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_clusterSizeV, c_nPXDSensors);
      m_utils->CreateDBHistoGroup(m_clusterSizeUV, c_nPXDSensors);
    } else {
      TString Name = Form("DQMER_PXD_NoOfEvents_Ref");
      DBObjPtr<TVectorD> DQMER_PXD_NoOfEvents_Ref(Name.Data());
      if (DQMER_PXD_NoOfEvents_Ref.isValid()) {
        m_NoOfEventsRef = (int)DQMER_PXD_NoOfEvents_Ref->GetMatrixArray()[0];
      } else {
        B2INFO("ERROR to open reference counter: DQMER_PXD_NoOfEvents_Ref");
        return;
      }

      int load = 1;
      if (!m_utils->LoadDBHisto(r_hitMapCounts)) load = 0;
      if (!m_utils->LoadDBHisto(r_hitMapClCounts)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_fired, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_clusters, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_startRow, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_chargStartRow, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_startRowCount, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_clusterCharge, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_pixelSignal, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_clusterSizeU, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_clusterSizeV, c_nPXDSensors)) load = 0;
      if (!m_utils->LoadDBHistoGroup(r_clusterSizeUV, c_nPXDSensors)) load = 0;
      if (!load) {
        B2INFO("ERROR to open of reference information");
        return;
      }
    }
  }

  fNoOfEventsRef[0] = m_NoOfEventsRef;
  NoOfEventsRef->SetElements(fNoOfEventsRef);
  NoOfEventsRef->Write("DQMER_PXD_NoOfEvents_Ref");

  // Compare histograms with reference histograms and create flags:
  for (int i = 0; i < c_nPXDSensors; i++) {
    double pars[2];
    pars[0] = 0.01;
    pars[1] = 0.05;
    m_utils->SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_hitMapCounts, r_hitMapCounts, m_fHitMapCountsFlag);
    m_utils->SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_hitMapClCounts, r_hitMapClCounts, m_fHitMapClCountsFlag);
    m_utils->SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_fired[i], r_fired[i], m_fFiredFlag);
    m_utils->SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_clusters[i], r_clusters[i], m_fClustersFlag);
    m_utils->SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_startRow[i], r_startRow[i], m_fStartRowFlag);
    m_utils->SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_chargStartRow[i], r_chargStartRow[i], m_fChargStartRowFlag);
    m_utils->SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_startRowCount[i], r_startRowCount[i], m_fStartRowCountFlag);
    m_utils->SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_clusterCharge[i], r_clusterCharge[i], m_fClusterChargeFlag);
    m_utils->SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_pixelSignal[i], r_pixelSignal[i], m_fPixelSignalFlag);
    m_utils->SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_clusterSizeU[i], r_clusterSizeU[i], m_fClusterSizeUFlag);
    m_utils->SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_clusterSizeV[i], r_clusterSizeV[i], m_fClusterSizeVFlag);
    m_utils->SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
                     m_clusterSizeUV[i], r_clusterSizeUV[i], m_fClusterSizeUVFlag);
  }

}

