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
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>
#include <pxd/unpacking/PXDMappingLookup.h>

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
  setDescription("PXD DQM clusters module "
                 "Recommended Number of events for monitorin is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CutPXDCharge", m_CutPXDCharge,
           "cut on pixel or cluster charge for accepting to hitmap histogram, default = 0.0 ", m_CutPXDCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDDQMClusters"));
}


PXDDQMClustersModule::~PXDDQMClustersModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMClustersModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2FATAL("Missing geometry for VXD, check steering file.");
  }
  if (gTools->getNumberOfPXDLayers() == 0) {
    B2WARNING("Missing geometry for PXD, PXD-DQM is skiped.");
    return;
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // basic constants presets:
  int nPXDSensors = gTools->getNumberOfPXDSensors();
  int nPXDChips = gTools->getTotalPXDChips();

  // Create basic histograms:
  m_hitMapCounts = new TH1I("DQM_PXD_PixelHitmapCounts", "DQM PXD Integrated number of fired pixels per sensor",
                            nPXDSensors, 0, nPXDSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");
  m_hitMapClCounts = new TH1I("DQM_PXD_ClusterHitmapCounts", "DQM PXD Integrated number of clusters per sensor",
                              nPXDSensors, 0, nPXDSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");
  // basic counters per chip:
  m_hitMapCountsChip = new TH1I("DQM_PXD_PixelHitmapCountsChip", "DQM PXD Integrated number of fired pixels per chip",
                                nPXDChips, 0, nPXDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1I("DQM_PXD_ClusterHitmapCountsChip", "DQM PXD Integrated number of clusters per chip",
                                  nPXDChips, 0, nPXDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  for (int i = 0; i < nPXDChips; i++) {
    VxdID id = gTools->getChipIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    int iChip = gTools->getPXDChipNumber(id);
    int IsU = gTools->isPXDSideU(id);
    TString AxisTicks = Form("%i_%i_%i_u%iDCD", iLayer, iLadder, iSensor, iChip);
    if (!IsU)
      AxisTicks = Form("%i_%i_%i_v%iSWB", iLayer, iLadder, iSensor, iChip);
    m_hitMapCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  m_fired = new TH1F*[nPXDSensors];
  m_clusters = new TH1F*[nPXDSensors];
  m_startRow = new TH1F*[nPXDSensors];
  m_chargStartRow = new TH1F*[nPXDSensors];
  m_startRowCount = new TH1F*[nPXDSensors];
  m_clusterCharge = new TH1F*[nPXDSensors];
  m_pixelSignal = new TH1F*[nPXDSensors];
  m_clusterSizeU = new TH1F*[nPXDSensors];
  m_clusterSizeV = new TH1F*[nPXDSensors];
  m_clusterSizeUV = new TH1F*[nPXDSensors];

  m_hitMapU = new TH1F*[nPXDSensors];
  m_hitMapV = new TH1F*[nPXDSensors];
  m_hitMap = new TH2F*[nPXDSensors];
  m_hitMapUCl = new TH1F*[nPXDSensors];
  m_hitMapVCl = new TH1F*[nPXDSensors];
  m_hitMapCl = new TH2F*[nPXDSensors];
  m_seed = new TH1F*[nPXDSensors];
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
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
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_Clusters") % sensorDescr);
    title = str(format("DQM PXD Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = NULL;
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
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
  oldDir->cd();
}


void PXDDQMClustersModule::initialize()
{

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  m_storeDAQEvtStats.isRequired();

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() != 0) {
    //Register collections
    StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
    StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
    RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
    m_storePXDClustersName = storePXDClusters.getName();
    m_relPXDClusterDigitName = relPXDClusterDigits.getName();

    //Store names to speed up creation later
    m_storePXDDigitsName = storePXDDigits.getName();
  }
}

void PXDDQMClustersModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() == 0) return;

  if (m_hitMapCounts != NULL) m_hitMapCounts->Reset();
  if (m_hitMapClCounts != NULL) m_hitMapClCounts->Reset();
  if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Reset();

  for (int i = 0; i < gTools->getNumberOfPXDSensors(); i++) {
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
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() == 0) return;

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  int firstPXDLayer = gTools->getFirstPXDLayer();
  int lastPXDLayer = gTools->getLastPXDLayer();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  // PXD basic histograms:
  // Fired pixels
  vector< set<int> > Pixels(nPXDSensors); // sets to eliminate multiple samples per pixel
  for (const PXDDigit& digit2 : storePXDDigits) {
    int iLayer = digit2.getSensorID().getLayerNumber();
    if ((iLayer < firstPXDLayer) || (iLayer > lastPXDLayer)) continue;
    int iLadder = digit2.getSensorID().getLadderNumber();
    int iSensor = digit2.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels.at(index).insert(digit2.getUniqueChannelID());
    int iChip = PXDMappingLookup::getDCDID(digit2.getUCellID(), digit2.getVCellID(), sensorID);
    int indexChip = gTools->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(digit2.getVCellID());
    indexChip = gTools->getPXDChipIndex(sensorID, kFALSE, iChip);
    if (m_hitMapCountsChip != NULL) m_hitMapCountsChip->Fill(indexChip);
    if (m_pixelSignal[index] != NULL) m_pixelSignal[index]->Fill(digit2.getCharge());
    if (digit2.getCharge() < m_CutPXDCharge) continue;
    if (m_hitMapCounts != NULL) m_hitMapCounts->Fill(index);
    if (m_hitMapU[index] != NULL) m_hitMapU[index]->Fill(digit2.getUCellID());
    if (m_hitMapV[index] != NULL) m_hitMapV[index]->Fill(digit2.getVCellID());
    if (m_hitMap[index] != NULL) m_hitMap[index]->Fill(digit2.getUCellID(), digit2.getVCellID());

  }
  for (int i = 0; i < nPXDSensors; i++) {
    if ((m_fired[i] != NULL) && (Pixels[i].size() > 0)) m_fired[i]->Fill(Pixels[i].size());
  }

  vector< set<int> > counts(nPXDSensors);
  // Hitmaps, Charge, Seed, Size, ...
  for (const PXDCluster& cluster : storePXDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < firstPXDLayer) || (iLayer > lastPXDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    counts.at(index).insert(cluster.GetUniqueID());
    int iChip = PXDMappingLookup::getDCDID(SensorInfo.getUCellID(cluster.getU()), SensorInfo.getVCellID(cluster.getV()), sensorID);
    int indexChip = gTools->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(SensorInfo.getVCellID(cluster.getV()));
    indexChip = gTools->getPXDChipIndex(sensorID, kFALSE, iChip);
    if (m_hitMapClCountsChip != NULL) m_hitMapClCountsChip->Fill(indexChip);
    if (m_hitMapClCounts != NULL) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != NULL) m_clusterCharge[index]->Fill(cluster.getCharge());
    if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != NULL) m_clusterSizeUV[index]->Fill(cluster.getSize());
    if (m_seed[index] != NULL) m_seed[index]->Fill(cluster.getSeedCharge());
    if (cluster.getCharge() < m_CutPXDCharge) continue;
    if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()));
    if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(
        SensorInfo.getVCellID(cluster.getV()));
    if (m_hitMapCl[index] != NULL) m_hitMapCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()),
        SensorInfo.getVCellID(cluster.getV()));

  }
  for (int i = 0; i < nPXDSensors; i++) {
    if ((m_clusters[i] != NULL) && (counts[i].size() > 0))
      m_clusters[i]->Fill(counts[i].size());
  }

  // Start rows
  std::map<VxdID, unsigned short> startRows;
  for (int index = 0; index < nPXDSensors; index++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(index);

    const PXDDAQDHEStatus* dhe = (*m_storeDAQEvtStats).findDHE(id);
    if (dhe == nullptr) {
      B2ERROR("No DHE found for SensorId: " << id);
      continue;
    }
    auto startRow = dhe->getStartRow();
    if (m_startRow[index] != NULL) m_startRow[index]->Fill(startRow);
    startRows.insert(std::make_pair(id, startRow));
  }

  // Cluster seed charge by start row
  for (auto& cluster : storePXDClusters) {
    VxdID sensorID = cluster.getSensorID();
    int index = gTools->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

    float fDistance = SensorInfo.getVCellID(cluster.getV()) - startRows[cluster.getSensorID()];
    if (fDistance < 0) fDistance += SensorInfo.getVCells();
    if (m_chargStartRow[index] != NULL) m_chargStartRow[index]->Fill(fDistance, cluster.getSeedCharge());
    if (m_startRowCount[index] != NULL) m_startRowCount[index]->Fill(fDistance);
  }

}
