/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "pxd/modules/pxdDQM/PXDDQMExpressRecoModule.h"

#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>
#include <pxd/unpacking/PXDMappingLookup.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMExpressReco);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMExpressRecoModule::PXDDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM module for Express Reco "
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
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
           std::string("PXDER"));
}


//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMExpressRecoModule::defineHisto()
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
  m_hitMapCounts = new TH1D("DQMER_PXD_PixelHitmapCounts", "PXD Integrated number of fired pixels per sensor",
                            nPXDSensors, 0, nPXDSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");

  m_hitMapFilterCounts = new TH1D("DQMER_PXD_PixelHitmapFilterCounts", "PXD Integrated number of filtered pixels per sensor",
                                  nPXDSensors, 0, nPXDSensors);
  m_hitMapFilterCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapFilterCounts->GetYaxis()->SetTitle("counts");

  m_hitMapClCounts = new TH1D("DQMER_PXD_ClusterHitmapCounts", "PXD Integrated number of clusters per sensor",
                              nPXDSensors, 0, nPXDSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");

  m_hitMapClFilterCounts = new TH1D("DQMER_PXD_ClusterHitmapFilterCounts", "PXD Integrated number of filtered clusters per sensor",
                                    nPXDSensors, 0, nPXDSensors);
  m_hitMapClFilterCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClFilterCounts->GetYaxis()->SetTitle("counts");

  // basic counters per chip:
  m_hitMapCountsChip = new TH1D("DQMER_PXD_PixelHitmapCountsChip", "PXD Integrated number of fired pixels per chip",
                                nPXDChips, 0, nPXDChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1D("DQMER_PXD_ClusterHitmapCountsChip", "PXD Integrated number of clusters per chip",
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
  // FIXME: startrow histos are for experts not shifters
  //m_startRow.resize(nPXDSensors);
  //m_chargStartRow.resize(nPXDSensors);
  //m_startRowCount.resize(nPXDSensors);
  m_clusterCharge.resize(nPXDSensors);
  m_pixelSignal.resize(nPXDSensors);
  m_clusterSizeU.resize(nPXDSensors);
  m_clusterSizeV.resize(nPXDSensors);
  m_clusterSizeUV.resize(nPXDSensors);
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
    string name = str(format("DQMER_PXD_%1%_Fired") % sensorDescr);
    string title = str(format("PXD Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = new TH1D(name.c_str(), title.c_str(), 200, 0, 200);
    m_fired[i]->SetCanExtend(TH1::kAllAxes);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of good fired pixels per frame
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_GoodFired") % sensorDescr);
    title = str(format("PXD Sensor %1% Good pixels") % sensorDescr);
    m_goodfired[i] = new TH1D(name.c_str(), title.c_str(), 200, 0, 200);
    m_goodfired[i]->SetCanExtend(TH1::kAllAxes);
    m_goodfired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_goodfired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_Clusters") % sensorDescr);
    title = str(format("PXD Sensor %1% Clusters") % sensorDescr);
    m_clusters[i] = new TH1D(name.c_str(), title.c_str(), 200, 0, 200);
    m_clusters[i]->SetCanExtend(TH1::kAllAxes);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of good clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_GoodClusters") % sensorDescr);
    title = str(format("PXD Sensor %1% Good clusters") % sensorDescr);
    m_goodclusters[i] = new TH1D(name.c_str(), title.c_str(), 200, 0, 200);
    m_goodclusters[i]->SetCanExtend(TH1::kAllAxes);
    m_goodclusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_goodclusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    // FIXME: expert level, remove here at some point
    //----------------------------------------------------------------
    //name = str(format("DQMER_PXD_%1%_StartRow") % sensorDescr);
    //title = str(format("PXD Sensor %1% Start row distribution") % sensorDescr);

    //int nPixels;/** Number of pixels on PXD v direction */
    //nPixels = SensorInfo.getVCells();
    //m_startRow[i] = new TH1D(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    //m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    //name = str(format("DQMER_PXD_%1%_AverageSeedByStartRow") % sensorDescr);
    //title = str(format("PXD Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    //m_chargStartRow[i] = new TH1D(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    //m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    //name = str(format("DQMER_PXD_%1%_SeedCountsByStartRow") % sensorDescr);
    //title = str(format("PXD Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    //m_startRowCount[i] = new TH1D(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    //m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_ClusterCharge") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1D(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_PixelSignal") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1D(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("DQMER_PXD_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1D(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");
  }

  oldDir->cd();
}


void PXDDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() != 0) {
    //Register collections
    m_storePXDDigits.isOptional(m_storePXDDigitsName);
    m_storePXDClusters.isOptional(m_storePXDClustersName);
  }
}

void PXDDQMExpressRecoModule::beginRun()
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
    // FIXME: startrow is for expert only, not shifter
    //if (m_startRow[i] != nullptr) m_startRow[i]->Reset();
    //if (m_chargStartRow[i] != nullptr) m_chargStartRow[i]->Reset();
    //if (m_startRowCount[i] != nullptr) m_startRowCount[i]->Reset();
    if (m_clusterCharge[i] != nullptr) m_clusterCharge[i]->Reset();
    if (m_pixelSignal[i] != nullptr) m_pixelSignal[i]->Reset();
    if (m_clusterSizeU[i] != nullptr) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != nullptr) m_clusterSizeV[i]->Reset();
    if (m_clusterSizeUV[i] != nullptr) m_clusterSizeUV[i]->Reset();
  }

}


void PXDDQMExpressRecoModule::event()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfPXDLayers() == 0) return;

  // If there are no digits, leave
  if (!m_storePXDDigits || !m_storePXDDigits.getEntries()) return;

  int nPXDSensors = gTools->getNumberOfPXDSensors();

  // PXD basic histograms:
  // Fired pixels
  vector< int > Pixels(nPXDSensors);
  vector< int > GoodPixels(nPXDSensors);
  for (const PXDDigit& digit : m_storePXDDigits) {
    int iLayer = digit.getSensorID().getLayerNumber();
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
    }
  }
  for (int i = 0; i < nPXDSensors; i++) {
    if (m_fired[i] != nullptr && Pixels[i] > 0) m_fired[i]->Fill(Pixels[i]);
    if (m_goodfired[i] != nullptr && GoodPixels[i] > 0) m_goodfired[i]->Fill(GoodPixels[i]);
  }

  // Hitmaps, Charge, Seed, Size, ...
  vector< int > Clusters(nPXDSensors);
  vector< int > GoodClusters(nPXDSensors);
  for (const PXDCluster& cluster : m_storePXDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
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
    if (m_clusterSizeU[index] != nullptr) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != nullptr) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != nullptr) m_clusterSizeUV[index]->Fill(cluster.getSize());
    // filter for cluster size and seed pixel
    if (cluster.getSize() <= m_CutMaxClusterSize  && cluster.getCharge() >= m_CutMinClusterCharge
        && cluster.getSeedCharge() >= m_CutMinSeedCharge && cluster.getSeedCharge() < 255) {
      GoodClusters[index]++;
      if (m_hitMapClFilterCounts != nullptr) m_hitMapClFilterCounts->Fill(index);
    }
  }
  for (int i = 0; i < nPXDSensors; i++) {
    if (m_clusters[i] != nullptr && Clusters[i] > 0)  m_clusters[i]->Fill(Clusters[i]);
    if (m_goodclusters[i] != nullptr && GoodClusters[i] > 0) m_goodclusters[i]->Fill(GoodClusters[i]);
  }
}
