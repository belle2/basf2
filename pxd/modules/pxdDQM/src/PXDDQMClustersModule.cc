/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Björn Spruck,                                            *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/modules/pxdDQM/PXDDQMClustersModule.h"

#include <framework/core/HistoModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/geometry/SensorInfo.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>
#include <pxd/unpacking/PXDMappingLookup.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

#include <boost/format.hpp>

#include "TDirectory.h"

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
  addParam("CutMinCharge", m_CutMinCharge,
           "cut on pixel charge for accepting good pixel, default >= 12", 12);
  addParam("CutMinSeedCharge", m_CutMinSeedCharge,
           "cut on cluster seed for accepting good cluster, default >= 12", 12);
  addParam("CutMaxClusterSize", m_CutMaxClusterSize,
           "cut on cluster size accepting good cluster, default <= 4", 4);
  addParam("CutMinClusterCharge", m_CutMinClusterCharge,
           "cut on cluster charge for accepting good cluster, default >= 12", 12);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDDQMClusters"));
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
    B2WARNING("Missing geometry for PXD, PXD-DQM is skipped.");
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
  m_hitMapCounts = new TH1D("DQM_PXD_PixelHitmapCounts", "PXD Integrated number of fired pixels per sensor",
                            nPXDSensors, 0, nPXDSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");

  m_hitMapFilterCounts = new TH1D("DQM_PXD_PixelHitmapFilterCounts", "PXD Integrated number of filtered pixels per sensor",
                                  nPXDSensors, 0, nPXDSensors);
  m_hitMapFilterCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapFilterCounts->GetYaxis()->SetTitle("counts");

  m_hitMapClCounts = new TH1D("DQM_PXD_ClusterHitmapCounts", "PXD Integrated number of clusters per sensor",
                              nPXDSensors, 0, nPXDSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");

  m_hitMapClFilterCounts = new TH1D("DQM_PXD_ClusterHitmapFilterCounts", "PXD Integrated number of filtered clusters per sensor",
                                    nPXDSensors, 0, nPXDSensors);
  m_hitMapClFilterCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClFilterCounts->GetYaxis()->SetTitle("counts");

  // basic counters per chip:
  m_hitMapCountsChip = new TH1D("DQM_PXD_PixelHitmapCountsChip", "PXD Integrated number of fired pixels per chip",
                                nPXDChips, 0, nPXDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1D("DQM_PXD_ClusterHitmapCountsChip", "PXD Integrated number of clusters per chip",
                                  nPXDChips, 0, nPXDChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Chip ID");
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

  m_fired.resize(nPXDSensors);
  m_goodfired.resize(nPXDSensors);
  m_clusters.resize(nPXDSensors);
  m_goodclusters.resize(nPXDSensors);
  m_startRow.resize(nPXDSensors);
  m_chargStartRow.resize(nPXDSensors);
  m_startRowCount.resize(nPXDSensors);
  m_clusterCharge.resize(nPXDSensors);
  m_clusterEnergy.resize(nPXDSensors);
  m_pixelSignal.resize(nPXDSensors);
  m_clusterSizeU.resize(nPXDSensors);
  m_clusterSizeV.resize(nPXDSensors);
  m_clusterSizeUV.resize(nPXDSensors);

  m_hitMapU.resize(nPXDSensors);
  m_hitMapV.resize(nPXDSensors);
  m_hitMap.resize(nPXDSensors);
  m_hitMapUCl.resize(nPXDSensors);
  m_hitMapVCl.resize(nPXDSensors);
  m_hitMapCl.resize(nPXDSensors);
  m_seed.resize(nPXDSensors);
  for (int i = 0; i < nPXDSensors; i++) {
    VxdID id = gTools->getSensorIDFromPXDIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    auto nUPixels = SensorInfo.getUCells();/** Number of pixels on PXD u direction */
    auto nVPixels = SensorInfo.getVCells();/** Number of pixels on PXD v direction */
    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    string name = str(format("DQM_PXD_%1%_Fired") % sensorDescr);
    string title = str(format("PXD Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = new TH1D(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of good fired pixels per frame
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_GoodFired") % sensorDescr);
    title = str(format("PXD Sensor %1% Good Fired pixels") % sensorDescr);
    m_goodfired[i] = new TH1D(name.c_str(), title.c_str(), 50, 0, 50);
    m_goodfired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_goodfired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_Clusters") % sensorDescr);
    title = str(format("PXD Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = new TH1D(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of good clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_GoodClusters") % sensorDescr);
    title = str(format("PXD Sensor %1% Number of good clusters") % sensorDescr);
    m_goodclusters[i] = new TH1D(name.c_str(), title.c_str(), 20, 0, 20);
    m_goodclusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_goodclusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_StartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Start row distribution") % sensorDescr);

    m_startRow[i] = new TH1D(name.c_str(), title.c_str(), nVPixels / 4, 0.0, nVPixels);
    m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_AverageSeedByStartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    m_chargStartRow[i] = new TH1D(name.c_str(), title.c_str(), nVPixels / 4, 0.0, nVPixels);
    m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    name = str(format("DQM_PXD_%1%_SeedCountsByStartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    m_startRowCount[i] = new TH1D(name.c_str(), title.c_str(), nVPixels / 4, 0.0, nVPixels);
    m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterCharge") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1D(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Energy
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterEnergy") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Energy") % sensorDescr);
    m_clusterEnergy[i] = new TH1D(name.c_str(), title.c_str(), 100, 0, 50);
    m_clusterEnergy[i]->GetXaxis()->SetTitle("energy of clusters [keV]");
    m_clusterEnergy[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_PixelSignal") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1D(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("DQM_PXD_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");

    //----------------------------------------------------------------
    // Hitmaps: Number of pixels by coordinate
    //----------------------------------------------------------------
    // Hitmaps in U
    name = str(format("PXD_%1%_PixelHitmapU") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Hitmap in U") % sensorDescr);
    m_hitMapU[i] = new TH1D(name.c_str(), title.c_str(), nUPixels, 0, nUPixels);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in V
    name = str(format("PXD_%1%_PixelHitmapV") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Hitmap in V") % sensorDescr);
    m_hitMapV[i] = new TH1D(name.c_str(), title.c_str(), nVPixels, 0, nVPixels);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in UV
    name = str(format("PXD_%1%_PixelHitmap") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Hitmap") % sensorDescr);
    m_hitMap[i] = new TH2D(name.c_str(), title.c_str(), nUPixels, 0, nUPixels, nVPixels, 0, nVPixels);
    m_hitMap[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMap[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMap[i]->GetZaxis()->SetTitle("hits");

    //----------------------------------------------------------------
    // Hitmaps: Number of clusters by coordinate
    //----------------------------------------------------------------
    // Hitmaps in U
    name = str(format("PXD_%1%_HitmapClstU") % sensorDescr);
    title = str(format("PXD Sensor %1% Hitmap Clusters in U") % sensorDescr);
    m_hitMapUCl[i] = new TH1D(name.c_str(), title.c_str(), nUPixels, 0, nUPixels);
    m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in V
    name = str(format("PXD_%1%_HitmapClstV") % sensorDescr);
    title = str(format("PXD Sensor %1% Hitmap Clusters in V") % sensorDescr);
    m_hitMapVCl[i] = new TH1D(name.c_str(), title.c_str(), nVPixels, 0, nVPixels);
    m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in UV
    name = str(format("PXD_%1%_HitmapClst") % sensorDescr);
    title = str(format("PXD Sensor %1% Hitmap Clusters") % sensorDescr);
    m_hitMapCl[i] = new TH2D(name.c_str(), title.c_str(), nUPixels, 0, nUPixels, nVPixels, 0, nVPixels);
    m_hitMapCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapCl[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMapCl[i]->GetZaxis()->SetTitle("hits");

    //----------------------------------------------------------------
    // Cluster seed charge distribution
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_Seed") % sensorDescr);
    title = str(format("PXD Sensor %1% Seed charge") % sensorDescr);
    m_seed[i] = new TH1D(name.c_str(), title.c_str(), 256, 0, 256);
    m_seed[i]->GetXaxis()->SetTitle("seed charge of clusters [ADU]");
    m_seed[i]->GetYaxis()->SetTitle("count");

  }
  oldDir->cd();
}


void PXDDQMClustersModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  m_storeDAQEvtStats.isOptional();

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

  if (m_hitMapCounts != nullptr) m_hitMapCounts->Reset();
  if (m_hitMapFilterCounts != nullptr) m_hitMapFilterCounts->Reset();
  if (m_hitMapClCounts != nullptr) m_hitMapClCounts->Reset();
  if (m_hitMapClFilterCounts != nullptr) m_hitMapClFilterCounts->Reset();
  if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Reset();

  for (int i = 0; i < gTools->getNumberOfPXDSensors(); i++) {
    if (m_fired[i] != nullptr) m_fired[i]->Reset();
    if (m_goodfired[i] != nullptr) m_goodfired[i]->Reset();
    if (m_clusters[i] != nullptr) m_clusters[i]->Reset();
    if (m_goodclusters[i] != nullptr) m_goodclusters[i]->Reset();
    if (m_startRow[i] != nullptr) m_startRow[i]->Reset();
    if (m_chargStartRow[i] != nullptr) m_chargStartRow[i]->Reset();
    if (m_startRowCount[i] != nullptr) m_startRowCount[i]->Reset();
    if (m_clusterCharge[i] != nullptr) m_clusterCharge[i]->Reset();
    if (m_clusterEnergy[i] != nullptr) m_clusterEnergy[i]->Reset();
    if (m_pixelSignal[i] != nullptr) m_pixelSignal[i]->Reset();
    if (m_clusterSizeU[i] != nullptr) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != nullptr) m_clusterSizeV[i]->Reset();
    if (m_clusterSizeUV[i] != nullptr) m_clusterSizeUV[i]->Reset();

    if (m_hitMapU[i] != nullptr) m_hitMapU[i]->Reset();
    if (m_hitMapV[i] != nullptr) m_hitMapV[i]->Reset();
    if (m_hitMap[i] != nullptr) m_hitMap[i]->Reset();
    if (m_hitMapUCl[i] != nullptr) m_hitMapUCl[i]->Reset();
    if (m_hitMapVCl[i] != nullptr) m_hitMapVCl[i]->Reset();
    if (m_hitMapCl[i] != nullptr) m_hitMapCl[i]->Reset();
    if (m_seed[i] != nullptr) m_seed[i]->Reset();

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
  vector< int > Pixels(nPXDSensors);
  vector< int > GoodPixels(nPXDSensors);
  for (const PXDDigit& digit : storePXDDigits) {
    int iLayer = digit.getSensorID().getLayerNumber();
    if ((iLayer < firstPXDLayer) || (iLayer > lastPXDLayer)) continue;
    int iLadder = digit.getSensorID().getLadderNumber();
    int iSensor = digit.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels[index]++;
    int iChip = PXDMappingLookup::getDCDID(digit.getUCellID(), digit.getVCellID(), sensorID);
    int indexChip = gTools->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(digit.getVCellID());
    indexChip = gTools->getPXDChipIndex(sensorID, kFALSE, iChip);
    if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Fill(indexChip);
    if (m_pixelSignal[index] != nullptr) m_pixelSignal[index]->Fill(digit.getCharge());
    if (m_hitMapCounts != nullptr) m_hitMapCounts->Fill(index);
    // filter for pixel charge
    if (digit.getCharge() >= m_CutMinCharge && digit.getCharge() < 255) {
      GoodPixels[index]++;
      if (m_hitMapFilterCounts != nullptr) m_hitMapFilterCounts->Fill(index);
      if (m_hitMapU[index] != nullptr) m_hitMapU[index]->Fill(digit.getUCellID());
      if (m_hitMapV[index] != nullptr) m_hitMapV[index]->Fill(digit.getVCellID());
      if (m_hitMap[index] != nullptr) m_hitMap[index]->Fill(digit.getUCellID(), digit.getVCellID());
    }
  }
  for (int i = 0; i < nPXDSensors; i++) {
    if (m_fired[i] != nullptr && Pixels[i] > 0) m_fired[i]->Fill(Pixels[i]);
    if (m_goodfired[i] != nullptr && GoodPixels[i] > 0) m_goodfired[i]->Fill(GoodPixels[i]);
  }

  // Hitmaps, Charge, Seed, Size, ...
  vector< int > Clusters(nPXDSensors);
  vector< int > GoodClusters(nPXDSensors);
  for (const PXDCluster& cluster : storePXDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < firstPXDLayer) || (iLayer > lastPXDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getPXDSensorIndex(sensorID);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Clusters[index]++;
    int iChip = PXDMappingLookup::getDCDID(SensorInfo.getUCellID(cluster.getU()), SensorInfo.getVCellID(cluster.getV()), sensorID);
    int indexChip = gTools->getPXDChipIndex(sensorID, kTRUE, iChip);
    if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Fill(indexChip);
    iChip = PXDMappingLookup::getSWBID(SensorInfo.getVCellID(cluster.getV()));
    indexChip = gTools->getPXDChipIndex(sensorID, kFALSE, iChip);
    if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Fill(indexChip);
    if (m_hitMapClCounts != nullptr) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != nullptr) m_clusterCharge[index]->Fill(cluster.getCharge());
    // FIXME: The cluster charge is stored in ADU. We have to extract the
    // area dependent conversion factor ADU->eV. Assume this is the same for all pixels of the
    // cluster.
    auto cluster_uID = SensorInfo.getUCellID(cluster.getU());
    auto cluster_vID = SensorInfo.getVCellID(cluster.getV());
    auto ADUToEnergy =  PXDGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
    if (m_clusterEnergy[index] != nullptr) m_clusterEnergy[index]->Fill(cluster.getCharge()* ADUToEnergy / Unit::keV);
    if (m_clusterSizeU[index] != nullptr) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != nullptr) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != nullptr) m_clusterSizeUV[index]->Fill(cluster.getSize());
    if (m_seed[index] != nullptr) m_seed[index]->Fill(cluster.getSeedCharge());

    // filter for cluster size and seed pixel
    if (cluster.getSize() <= m_CutMaxClusterSize  && cluster.getCharge() >= m_CutMinClusterCharge
        && cluster.getSeedCharge() >= m_CutMinSeedCharge && cluster.getSeedCharge() < 255) {
      GoodClusters[index]++;
      if (m_hitMapClFilterCounts != nullptr) m_hitMapClFilterCounts->Fill(index);

      if (m_hitMapUCl[index] != nullptr) m_hitMapUCl[index]->Fill(
          SensorInfo.getUCellID(cluster.getU()));
      if (m_hitMapVCl[index] != nullptr) m_hitMapVCl[index]->Fill(
          SensorInfo.getVCellID(cluster.getV()));
      if (m_hitMapCl[index] != nullptr) m_hitMapCl[index]->Fill(
          SensorInfo.getUCellID(cluster.getU()),
          SensorInfo.getVCellID(cluster.getV()));
    }
  }
  for (int i = 0; i < nPXDSensors; i++) {
    if (m_clusters[i] != nullptr && Clusters[i] > 0)  m_clusters[i]->Fill(Clusters[i]);
    if (m_goodclusters[i] != nullptr && GoodClusters[i] > 0) m_goodclusters[i]->Fill(GoodClusters[i]);
  }

  // Only fill start row (triggergate) histos when data is available
  if (m_storeDAQEvtStats.isValid()) {
    std::map<VxdID, unsigned short> startRows;
    for (int index = 0; index < nPXDSensors; index++) {
      VxdID id = gTools->getSensorIDFromPXDIndex(index);

      const PXDDAQDHEStatus* dhe = (*m_storeDAQEvtStats).findDHE(id);
      if (dhe != nullptr) {
        auto startRow = dhe->getStartRow();
        if (m_startRow[index] != nullptr) m_startRow[index]->Fill(startRow);
        startRows.insert(std::make_pair(id, startRow));
      } else {
        B2WARNING("No PXDDAQDHEStatus for VXD Sensor " << id << " found.");
      }
    }

    // Cluster seed charge by start row
    for (auto& cluster : storePXDClusters) {
      VxdID sensorID = cluster.getSensorID();
      int index = gTools->getPXDSensorIndex(sensorID);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      float fDistance = SensorInfo.getVCellID(cluster.getV()) - startRows[cluster.getSensorID()];
      if (fDistance < 0) fDistance += SensorInfo.getVCells();
      if (m_chargStartRow[index] != nullptr) m_chargStartRow[index]->Fill(fDistance, cluster.getSeedCharge());
      if (m_startRowCount[index] != nullptr) m_startRowCount[index]->Fill(fDistance);
    }
  }

}
