/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "vtx/modules/vtxDQM/VTXDQMClustersModule.h"

#include <framework/core/HistoModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <vtx/dataobjects/VTXDigit.h>
#include <vtx/dataobjects/VTXCluster.h>
#include <vtx/geometry/SensorInfo.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>
#include <vtx/reconstruction/VTXGainCalibrator.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::VTX;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VTXDQMClusters)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VTXDQMClustersModule::VTXDQMClustersModule() : HistoModule()
{
  //Set module properties
  setDescription("VTX DQM clusters module "
                 "Recommended Number of events for monitorin is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CutVTXCharge", m_CutVTXCharge,
           "cut on pixel or cluster charge for accepting to hitmap histogram, default = 0 ", m_CutVTXCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("VTXDQMClusters"));
}




//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VTXDQMClustersModule::defineHisto()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) {
    B2WARNING("Missing geometry for VTX, VTX-DQM is skiped.");
    return;
  }

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  // basic constants presets:
  int nVTXSensors = gTools->getNumberOfSensors();
  int nVTXChips = gTools->getTotalVTXChips();

  // Create basic histograms:
  m_hitMapCounts = new TH1I("DQM_VTX_PixelHitmapCounts", "DQM VTX Integrated number of fired pixels per sensor",
                            nVTXSensors, 0, nVTXSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");
  m_hitMapClCounts = new TH1I("DQM_VTX_ClusterHitmapCounts", "DQM VTX Integrated number of clusters per sensor",
                              nVTXSensors, 0, nVTXSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");
  // basic counters per chip:
  m_hitMapCountsChip = new TH1I("DQM_VTX_PixelHitmapCountsChip", "DQM VTX Integrated number of fired pixels per chip",
                                nVTXChips, 0, nVTXChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1I("DQM_VTX_ClusterHitmapCountsChip", "DQM VTX Integrated number of clusters per chip",
                                  nVTXChips, 0, nVTXChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsChip->GetYaxis()->SetTitle("counts");
  for (int i = 0; i < nVTXChips; i++) {
    VxdID id = gTools->getChipIDFromVTXIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    int iChip = gTools->getVTXChipNumber(id);
    int IsU = gTools->isVTXSideU();
    TString AxisTicks = Form("%i_%i_%i_u%iDCD", iLayer, iLadder, iSensor, iChip);
    if (!IsU)
      AxisTicks = Form("%i_%i_%i_v%iSWB", iLayer, iLadder, iSensor, iChip);
    m_hitMapCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsChip->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  for (int i = 0; i < nVTXSensors; i++) {
    VxdID id = gTools->getSensorIDFromVTXIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }

  m_fired = new TH1F*[nVTXSensors];
  m_clusters = new TH1F*[nVTXSensors];
  m_startRow = new TH1F*[nVTXSensors];
  m_chargStartRow = new TH1F*[nVTXSensors];
  m_startRowCount = new TH1F*[nVTXSensors];
  m_clusterCharge = new TH1F*[nVTXSensors];
  m_clusterEnergy = new TH1F*[nVTXSensors];
  m_pixelSignal = new TH1F*[nVTXSensors];
  m_clusterSizeU = new TH1F*[nVTXSensors];
  m_clusterSizeV = new TH1F*[nVTXSensors];
  m_clusterSizeUV = new TH1F*[nVTXSensors];

  m_hitMapU = new TH1F*[nVTXSensors];
  m_hitMapV = new TH1F*[nVTXSensors];
  m_hitMap = new TH2F*[nVTXSensors];
  m_hitMapUCl = new TH1F*[nVTXSensors];
  m_hitMapVCl = new TH1F*[nVTXSensors];
  m_hitMapCl = new TH2F*[nVTXSensors];
  m_seed = new TH1F*[nVTXSensors];
  for (int i = 0; i < nVTXSensors; i++) {
    VxdID id = gTools->getSensorIDFromVTXIndex(i);
    int iLayer = id.getLayerNumber();
    int iLadder = id.getLadderNumber();
    int iSensor = id.getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    VTX::SensorInfo SensorInfo = dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    string name = str(format("DQM_VTX_%1%_Fired") % sensorDescr);
    string title = str(format("DQM VTX Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = nullptr;
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_Clusters") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = nullptr;
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_StartRow") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Start row distribution") % sensorDescr);

    int nPixels;/** Number of pixels on VTX v direction */
    nPixels = SensorInfo.getVCells();
    m_startRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_AverageSeedByStartRow") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    m_chargStartRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    name = str(format("DQM_VTX_%1%_SeedCountsByStartRow") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    m_startRowCount[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_ClusterCharge") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Energy
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_ClusterEnergy") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Cluster Energy") % sensorDescr);
    m_clusterEnergy[i] = new TH1F(name.c_str(), title.c_str(), 100, 0, 50);
    m_clusterEnergy[i]->GetXaxis()->SetTitle("energy of clusters [keV]");
    m_clusterEnergy[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_PixelSignal") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("DQM_VTX_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("DQM VTX Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");

    //----------------------------------------------------------------
    // Hitmaps: Number of pixels by coordinate
    //----------------------------------------------------------------
    // Hitmaps in U
    name = str(format("VTX_%1%_PixelHitmapU") % sensorDescr);
    title = str(format("VTX Sensor %1% Pixel Hitmap in U") % sensorDescr);
    nPixels = SensorInfo.getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in V
    name = str(format("VTX_%1%_PixelHitmapV") % sensorDescr);
    title = str(format("VTX Sensor %1% Pixel Hitmap in V") % sensorDescr);
    nPixels = SensorInfo.getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in UV
    name = str(format("VTX_%1%_PixelHitmap") % sensorDescr);
    title = str(format("VTX Sensor %1% Pixel Hitmap") % sensorDescr);
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
    name = str(format("VTX_%1%_HitmapClstU") % sensorDescr);
    title = str(format("VTX Sensor %1% Hitmap Clusters in U") % sensorDescr);
    nPixels = SensorInfo.getUCells();
    m_hitMapUCl[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in V
    name = str(format("VTX_%1%_HitmapClstV") % sensorDescr);
    title = str(format("VTX Sensor %1% Hitmap Clusters in V") % sensorDescr);
    nPixels = SensorInfo.getVCells();
    m_hitMapVCl[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
    // Hitmaps in UV
    name = str(format("VTX_%1%_HitmapClst") % sensorDescr);
    title = str(format("VTX Sensor %1% Hitmap Clusters") % sensorDescr);
    nPixels = SensorInfo.getUCells();
    nPixelsV = SensorInfo.getVCells();
    m_hitMapCl[i] = new TH2F(name.c_str(), title.c_str(), nPixels, 0, nPixels, nPixelsV, 0, nPixelsV);
    m_hitMapCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapCl[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMapCl[i]->GetZaxis()->SetTitle("hits");

    //----------------------------------------------------------------
    // Cluster seed charge distribution
    //----------------------------------------------------------------
    name = str(format("VTX_%1%_Seed") % sensorDescr);
    title = str(format("VTX Sensor %1% Seed charge") % sensorDescr);
    m_seed[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_seed[i]->GetXaxis()->SetTitle("seed charge of clusters [ADU]");
    m_seed[i]->GetYaxis()->SetTitle("count");

  }
  oldDir->cd();
}


void VTXDQMClustersModule::initialize()
{

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() != 0) {
    //Register collections
    StoreArray<VTXDigit> storeVTXDigits(m_storeVTXDigitsName);
    StoreArray<VTXCluster> storeVTXClusters(m_storeVTXClustersName);
    RelationArray relVTXClusterDigits(storeVTXClusters, storeVTXDigits);
    m_storeVTXClustersName = storeVTXClusters.getName();
    m_relVTXClusterDigitName = relVTXClusterDigits.getName();

    //Store names to speed up creation later
    m_storeVTXDigitsName = storeVTXDigits.getName();
  }
}

void VTXDQMClustersModule::beginRun()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) return;

  if (m_hitMapCounts != nullptr) m_hitMapCounts->Reset();
  if (m_hitMapClCounts != nullptr) m_hitMapClCounts->Reset();
  if (m_hitMapCountsChip != nullptr) m_hitMapCountsChip->Reset();
  if (m_hitMapClCountsChip != nullptr) m_hitMapClCountsChip->Reset();

  for (int i = 0; i < gTools->getNumberOfSensors(); i++) {
    if (m_fired[i] != nullptr) m_fired[i]->Reset();
    if (m_clusters[i] != nullptr) m_clusters[i]->Reset();
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


void VTXDQMClustersModule::event()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) return;

  const StoreArray<VTXDigit> storeVTXDigits(m_storeVTXDigitsName);
  const StoreArray<VTXCluster> storeVTXClusters(m_storeVTXClustersName);
  const RelationArray relVTXClusterDigits(storeVTXClusters, storeVTXDigits, m_relVTXClusterDigitName);

  // If there are no digits, leave
  if (!storeVTXDigits || !storeVTXDigits.getEntries()) return;

  int firstVTXLayer = gTools->getFirstLayer();
  int lastVTXLayer = gTools->getLastLayer();
  int nVTXSensors = gTools->getNumberOfSensors();

  // VTX basic histograms:
  // Fired pixels
  vector< int > Pixels(nVTXSensors);
  for (const VTXDigit& digit2 : storeVTXDigits) {
    int iLayer = digit2.getSensorID().getLayerNumber();
    if ((iLayer < firstVTXLayer) || (iLayer > lastVTXLayer)) continue;
    int iLadder = digit2.getSensorID().getLadderNumber();
    int iSensor = digit2.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getVTXSensorIndex(sensorID);
    VTX::SensorInfo SensorInfo = dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels[index]++;
    if (m_pixelSignal[index] != nullptr) m_pixelSignal[index]->Fill(digit2.getCharge());
    if (digit2.getCharge() < m_CutVTXCharge) continue;
    if (m_hitMapCounts != nullptr) m_hitMapCounts->Fill(index);
    if (m_hitMapU[index] != nullptr) m_hitMapU[index]->Fill(digit2.getUCellID());
    if (m_hitMapV[index] != nullptr) m_hitMapV[index]->Fill(digit2.getVCellID());
    if (m_hitMap[index] != nullptr) m_hitMap[index]->Fill(digit2.getUCellID(), digit2.getVCellID());

  }
  for (int i = 0; i < nVTXSensors; i++) {
    if ((m_fired[i] != nullptr) && (Pixels[i] > 0)) m_fired[i]->Fill(Pixels[i]);
  }

  // Hitmaps, Charge, Seed, Size, ...
  vector< int > counts(nVTXSensors);
  for (const VTXCluster& cluster : storeVTXClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < firstVTXLayer) || (iLayer > lastVTXLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getVTXSensorIndex(sensorID);
    VTX::SensorInfo SensorInfo = dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    counts[index]++;
    if (m_hitMapClCounts != nullptr) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != nullptr) m_clusterCharge[index]->Fill(cluster.getCharge());
    // FIXME: The cluster charge is stored in ADU. We have to extract the
    // area dependent conversion factor ADU->eV. Assume this is the same for all pixels of the
    // cluster.
    auto cluster_uID = SensorInfo.getUCellID(cluster.getU());
    auto cluster_vID = SensorInfo.getVCellID(cluster.getV());
    auto ADUToEnergy =  VTXGainCalibrator::getInstance().getADUToEnergy(sensorID, cluster_uID, cluster_vID);
    if (m_clusterEnergy[index] != nullptr) m_clusterEnergy[index]->Fill(cluster.getCharge()* ADUToEnergy / Unit::keV);
    if (m_clusterSizeU[index] != nullptr) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != nullptr) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != nullptr) m_clusterSizeUV[index]->Fill(cluster.getSize());
    if (m_seed[index] != nullptr) m_seed[index]->Fill(cluster.getSeedCharge());
    if (cluster.getCharge() < m_CutVTXCharge) continue;
    if (m_hitMapUCl[index] != nullptr) m_hitMapUCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()));
    if (m_hitMapVCl[index] != nullptr) m_hitMapVCl[index]->Fill(
        SensorInfo.getVCellID(cluster.getV()));
    if (m_hitMapCl[index] != nullptr) m_hitMapCl[index]->Fill(
        SensorInfo.getUCellID(cluster.getU()),
        SensorInfo.getVCellID(cluster.getV()));

  }
  for (int i = 0; i < nVTXSensors; i++) {
    if ((m_clusters[i] != nullptr) && (counts[i] > 0))
      m_clusters[i]->Fill(counts[i]);
  }

}
