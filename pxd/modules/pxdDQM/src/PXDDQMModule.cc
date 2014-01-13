#include "pxd/modules/pxdDQM/PXDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vector>
#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

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
}


PXDDQMModule::~PXDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMModule::defineHisto()
{
  // Fired pixel counts U
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDFired%1%") % iPlane);
    string title = str(format("Number of PXD fired pixels, plane %1%") % iPlane);
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDClusters%1%") % iPlane);
    string title = str(format("Number of PXD clusters, plane %1%") % iPlane);
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("count");
  }
  // Hitmaps in U
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDHitmapU%1%") % iPlane);
    string title = str(format("PXD Hitmap in U, plane %1%") % iPlane);
    float length = getInfo(i).getLength();
    int nPixels = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nPixels, -0.5 * length, 0.5 * length);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position [cm]");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDHitmapV%1%") % iPlane);
    string title = str(format("PXD Hitmap in V, plane %1%") % iPlane);
    float width = getInfo(i).getWidth();
    int nPixels = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nPixels, -0.5 * width, 0.5 * width);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position [cm]");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in UV
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("h2PXDHitmapUV%1%") % iPlane);
    string title = str(format("PXD Hitmap in U x V, plane %1%") % iPlane);
    float lengthU = getInfo(i).getLength();
    int nPixelsU = getInfo(i).getUCells();
    float widthV = getInfo(i).getWidth();
    int nPixelsV = getInfo(i).getVCells();
    m_hitMapUV[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, -0.5 * lengthU, 0.5 * lengthU, nPixelsV, -0.5 * widthV, 0.5 * widthV);
    m_hitMapUV[i]->GetXaxis()->SetTitle("u position [cm]");
    m_hitMapUV[i]->GetYaxis()->SetTitle("v position [cm]");
    m_hitMapUV[i]->GetZaxis()->SetTitle("hits");
  }
  // charge by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDCharge%1%") % iPlane);
    string title = str(format("PXD cluster charge, plane %1%") % iPlane);
    m_charge[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150);
    m_charge[i]->GetXaxis()->SetTitle("cluster charge [ADU]");
    m_charge[i]->GetYaxis()->SetTitle("count");
  }
  // seed by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDSeed%1%") % iPlane);
    string title = str(format("PXD seed charge, plane %1%") % iPlane);
    m_seed[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150);
    m_seed[i]->GetXaxis()->SetTitle("seed charge [ADU]");
    m_seed[i]->GetYaxis()->SetTitle("count");
  }
  // u size by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDSizeU%1%") % iPlane);
    string title = str(format("PXD cluster size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("cluster size in u [pitch units]");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDSizeV%1%") % iPlane);
    string title = str(format("PXD cluster size in V, plane %1%") % iPlane);
    m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeV[i]->GetXaxis()->SetTitle("cluster size in v [pitch units]");
    m_sizeV[i]->GetYaxis()->SetTitle("count");
  }
  // size by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDSize%1%") % iPlane);
    string title = str(format("PXD cluster size, plane %1%") % iPlane);
    m_size[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_size[i]->GetXaxis()->SetTitle("cluster size [pitch units]");
    m_size[i]->GetYaxis()->SetTitle("count");
  }
  // start row by plane
  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hPXDstartRow%1%") % iPlane);
    string title = str(format("PXD readout start row, plane %1%") % iPlane);
    m_startRow[i] = new TH1F(name.c_str(), title.c_str(), getInfo(i).getVCells(), 0, getInfo(i).getVCells());
    m_startRow[i]->GetXaxis()->SetTitle("start row");
    m_startRow[i]->GetYaxis()->SetTitle("count");
  }
}


void PXDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  RelationArray relClusterDigits(storeClusters, storeDigits);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
  m_storeFramesName = storeFrames.getName();
}

void PXDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nPXDPlanes; i++) {
    m_fired[i]->Reset();
    m_clusters[i]->Reset();
    m_hitMapU[i]->Reset();
    m_hitMapV[i]->Reset();
    m_hitMapUV[i]->Reset();
    m_charge[i]->Reset();
    m_seed[i]->Reset();
    m_sizeU[i]->Reset();
    m_sizeV[i]->Reset();
    m_size[i]->Reset();
    m_startRow[i]->Reset();
  }
}


void PXDDQMModule::event()
{

  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // If there are no pixels, leave
  if (!storeDigits || !storeDigits.getEntries()) return;
  // Fired pixels
  vector<int> pixel_count(c_nPXDPlanes);
  for (int i = 0; i < c_nPXDPlanes; i++) pixel_count[i] = 0;
  for (const PXDDigit & digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int index = planeToIndex(iPlane);
    pixel_count[index]++;
  }
  for (int i = 0; i < c_nPXDPlanes; i++) m_fired[i]->Fill(pixel_count[i]);

  // Cluster counts - same as Pixels
  vector<int> cluster_count(c_nPXDPlanes);
  for (int i = 0; i < c_nPXDPlanes; i++) cluster_count[i] = 0;
  for (const PXDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int index = planeToIndex(iPlane);
    cluster_count[index]++;
  }
  for (int i = 0; i < c_nPXDPlanes; i++) {
    m_clusters[i]->Fill(cluster_count[i]);
  }

  // Hitmaps, Charge, Seed, Size, Time
  for (const PXDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int index = planeToIndex(iPlane);
    m_hitMapU[index]->Fill(cluster.getU());
    m_hitMapV[index]->Fill(cluster.getV());
    m_hitMapUV[index]->Fill(cluster.getU(), cluster.getV());
    m_charge[index]->Fill(cluster.getCharge());
    m_seed[index]->Fill(cluster.getSeedCharge());
    m_sizeU[index]->Fill(cluster.getUSize());
    m_sizeV[index]->Fill(cluster.getVSize());
    m_size[index]->Fill(cluster.getSize());
  }
  // Start rows
  if (storeFrames && storeFrames.getEntries()) {
    for (const PXDFrame & frame : storeFrames) {
      int iPlane = frame.getSensorID().getLayerNumber();
      if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
      int index = planeToIndex(iPlane);
      m_startRow[index]->Fill(frame.getStartRow());
    }
  }

}


void PXDDQMModule::endRun()
{
}


void PXDDQMModule::terminate()
{
}
