/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "vtx/modules/vtxDQM/VTXDQMExpressRecoModule.h"

#include <vtx/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

#include <boost/format.hpp>

#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::VTX;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VTXDQMExpressReco)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VTXDQMExpressRecoModule::VTXDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("VTX DQM module for Express Reco "
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CutVTXCharge", m_CutVTXCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 0.0 ", m_CutVTXCharge);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("VTXER"));
}


VTXDQMExpressRecoModule::~VTXDQMExpressRecoModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VTXDQMExpressRecoModule::defineHisto()
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
  m_hitMapCounts = new TH1I("DQMER_VTX_PixelHitmapCounts", "DQM ER VTX Integrated number of fired pixels per sensor",
                            nVTXSensors, 0, nVTXSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");
  m_hitMapClCounts = new TH1I("DQMER_VTX_ClusterHitmapCounts", "DQM ER VTX Integrated number of clusters per sensor",
                              nVTXSensors, 0, nVTXSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");
  // basic counters per chip:
  m_hitMapCountsChip = new TH1I("DQMER_VTX_PixelHitmapCountsChip", "DQM ER VTX Integrated number of fired pixels per chip",
                                nVTXChips, 0, nVTXChips);
  m_hitMapCountsChip->GetXaxis()->SetTitle("Chip ID");
  m_hitMapCountsChip->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsChip = new TH1I("DQMER_VTX_ClusterHitmapCountsChip", "DQM ER VTX Integrated number of clusters per chip",
                                  nVTXChips, 0, nVTXChips);
  m_hitMapClCountsChip->GetXaxis()->SetTitle("Chip ID");
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
  // FIXME: startrow histos are for experts not shifters
  //m_startRow = new TH1F*[nVTXSensors];
  //m_chargStartRow = new TH1F*[nVTXSensors];
  //m_startRowCount = new TH1F*[nVTXSensors];
  m_clusterCharge = new TH1F*[nVTXSensors];
  m_pixelSignal = new TH1F*[nVTXSensors];
  m_clusterSizeU = new TH1F*[nVTXSensors];
  m_clusterSizeV = new TH1F*[nVTXSensors];
  m_clusterSizeUV = new TH1F*[nVTXSensors];
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
    string name = str(format("DQMER_VTX_%1%_Fired") % sensorDescr);
    string title = str(format("DQM ER VTX Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = nullptr;
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 200);
    m_fired[i]->SetCanExtend(TH1::kAllAxes);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_Clusters") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = nullptr;
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 200);
    m_clusters[i]->SetCanExtend(TH1::kAllAxes);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    // FIXME: expert level, remove here at some point
    //----------------------------------------------------------------
    //name = str(format("DQMER_VTX_%1%_StartRow") % sensorDescr);
    //title = str(format("DQM ER VTX Sensor %1% Start row distribution") % sensorDescr);

    //int nPixels;/** Number of pixels on VTX v direction */
    //nPixels = SensorInfo.getVCells();
    //m_startRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    //m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    //name = str(format("DQMER_VTX_%1%_AverageSeedByStartRow") % sensorDescr);
    //title = str(format("DQM ER VTX Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    //m_chargStartRow[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    //m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    //name = str(format("DQMER_VTX_%1%_SeedCountsByStartRow") % sensorDescr);
    //title = str(format("DQM ER VTX Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    //m_startRowCount[i] = new TH1F(name.c_str(), title.c_str(), nPixels / 4, 0.0, nPixels);
    //m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    //m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_ClusterCharge") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_PixelSignal") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("DQMER_VTX_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("DQM ER VTX Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");
  }

  oldDir->cd();
}


void VTXDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() != 0) {
    //Register collections
    m_storeVTXDigits.isOptional(m_storeVTXDigitsName);
    m_storeVTXClusters.isOptional(m_storeVTXClustersName);
  }
}

void VTXDQMExpressRecoModule::beginRun()
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


void VTXDQMExpressRecoModule::event()
{
  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  if (gTools->getNumberOfLayers() == 0) return;

  // If there are no digits, leave
  if (!m_storeVTXDigits || !m_storeVTXDigits.getEntries()) return;

  int nVTXSensors = gTools->getNumberOfSensors();

  // VTX basic histograms:
  // Fired pixels
  vector< int > Pixels(nVTXSensors);
  for (const VTXDigit& digit : m_storeVTXDigits) {
    int iLayer = digit.getSensorID().getLayerNumber();
    int iLadder = digit.getSensorID().getLadderNumber();
    int iSensor = digit.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getVTXSensorIndex(sensorID);
    VTX::SensorInfo SensorInfo = dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels[index]++;
    if (m_pixelSignal[index] != nullptr) m_pixelSignal[index]->Fill(digit.getCharge());
    if ((m_hitMapCounts != nullptr) && (digit.getCharge() > m_CutVTXCharge))
      m_hitMapCounts->Fill(index);
  }
  for (int i = 0; i < nVTXSensors; i++) {
    if (m_fired[i] != nullptr) m_fired[i]->Fill(Pixels[i]);
  }

  vector< int > counts(nVTXSensors);
  // Hitmaps, Charge, Size, ...
  for (const VTXCluster& cluster : m_storeVTXClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    VxdID sensorID(iLayer, iLadder, iSensor);
    int index = gTools->getVTXSensorIndex(sensorID);
    VTX::SensorInfo SensorInfo = dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    counts[index]++;
    if (m_hitMapClCounts != nullptr) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != nullptr) m_clusterCharge[index]->Fill(cluster.getCharge());
    if (m_clusterSizeU[index] != nullptr) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != nullptr) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != nullptr) m_clusterSizeUV[index]->Fill(cluster.getSize());
  }
  for (int i = 0; i < nVTXSensors; i++) {
    if (m_clusters[i] != nullptr)
      m_clusters[i]->Fill(counts[i]);
  }
}
