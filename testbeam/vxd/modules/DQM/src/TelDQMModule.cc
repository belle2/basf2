#include <testbeam/vxd/modules/DQM/TelDQMModule.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <testbeam/vxd/dataobjects/TelDigit.h>
#ifdef MAKE_TELCLUSTERS
#include <testbeam/vxd/dataobjects/TelCluster.h>
#else
#include <pxd/dataobjects/PXDCluster.h>
#endif

#include <vector>
#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelDQMModule::TelDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("TEL DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("Clusters", m_storeClustersName, "Name of the telescopes cluster collection",
           std::string("")); // always be explicit about this in case PXDClusters are used.
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("eutel"));
}


TelDQMModule::~TelDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TelDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  //----------------------------------------------------------------
  // Number of fired strips per frame : hFired[PlaneNo]
  //----------------------------------------------------------------
  // Fired pixel counts U
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELFired%1%") % iPlane);
    string title = str(format("Number of TEL fired pixels, plane %1%") % iPlane);
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_fired[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_fired[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Number of clusters per frame : hClusters[PlaneNo]
  //----------------------------------------------------------------
  // Number of clusters
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELClusters%1%") % iPlane);
    string title = str(format("Number of TEL clusters, plane %1%") % iPlane);
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Hitmaps in U
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELHitmapU%1%") % iPlane);
    string title = str(format("TEL Hitmap in U, plane %1%") % iPlane);
    int nPixels = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELHitmapV%1%") % iPlane);
    string title = str(format("TEL Hitmap in V, plane %1%") % iPlane);
    int nPixels = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nPixels, 0, nPixels);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
  //----------------------------------------------------------------
  // Cluster size distribution : hClusterSize[U/V/UV][PlaneNo]
  //----------------------------------------------------------------
  // u size by plane
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELSizeU%1%") % iPlane);
    string title = str(format("TEL cluster size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("cluster size in u");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELSizeV%1%") % iPlane);
    string title = str(format("TEL cluster size in V, plane %1%") % iPlane);
    m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeV[i]->GetXaxis()->SetTitle("cluster size in v");
    m_sizeV[i]->GetYaxis()->SetTitle("count");
  }
  // size by plane
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTELSize%1%") % iPlane);
    string title = str(format("TEL cluster size, plane %1%") % iPlane);
    m_size[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_size[i]->GetXaxis()->SetTitle("cluster size");
    m_size[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps : m_correlationsHitMaps[U][V]
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps
  for (int i = 0; i < c_nTELPlanes; i++) {
    int iPlane1 = indexToPlane(i);
    int nStripsU1 = getInfo(i).getUCells();
    int nStripsV1 = getInfo(i).getVCells();
    for (int j = 0; j < c_nTELPlanes; j++) {
      int iPlane2 = indexToPlane(j);
      int nStripsU2 = getInfo(j).getUCells();
      int nStripsV2 = getInfo(j).getVCells();
      if (i == j) {  // hit maps
        string name = str(format("h2TELHitmapUV%1%") % iPlane2);
        string title = str(format("Hitmap TEL in U x V, plane %1%") % iPlane2);
        m_correlationsHitMaps[c_nTELPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU2, 0, nStripsU2, nStripsV2, 0,
                                                               nStripsV2);
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetXaxis()->SetTitle("u position [pitch units]");
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetYaxis()->SetTitle("v position [pitch units]");
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else if (i < j) { // correlations for u
        string name = str(format("h2TELCorrelationmapU%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("Correlationmap TEL in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMaps[c_nTELPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU1, 0, nStripsU1, nStripsU2, 0,
                                                               nStripsU2);
        string axisxtitle = str(format("u position, plane %1% [pitch units]") % iPlane1);
        string axisytitle = str(format("u position, plane %1% [pitch units]") % iPlane2);
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else {            // correlations for v
        string name = str(format("h2TELCorrelationmapV%1%%2%") % iPlane2 % iPlane1);
        string title = str(format("Correlationmap TEL in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMaps[c_nTELPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsV2, 0, nStripsV2, nStripsV1, 0,
                                                               nStripsV1);
        string axisxtitle = str(format("v position, plane %1% [pitch units]") % iPlane2);
        string axisytitle = str(format("v position, plane %1% [pitch units]") % iPlane1);
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nTELPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }
  oldDir->cd();
}


void TelDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
#ifdef MAKE_TELCLUSTERS
  StoreArray<TelCluster> storeClusters(m_storeClustersName);
#else
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
#endif
  StoreArray<TelDigit> storeDigits(m_storeDigitsName);

  //storeClusters.isRequired();
  //storeDigits.isRequired();

  RelationArray relClusterDigits(storeClusters, storeDigits);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
}

void TelDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nTELPlanes; i++) {
    m_fired[i]->Reset();
    m_clusters[i]->Reset();
    m_hitMapU[i]->Reset();
    m_hitMapV[i]->Reset();
    m_sizeU[i]->Reset();
    m_sizeV[i]->Reset();
    m_size[i]->Reset();
  }
  for (int i = 0; i < c_nTELPlanes * c_nTELPlanes; i++) {
    m_correlationsHitMaps[i]->Reset();
  }
}


void TelDQMModule::event()
{

  const StoreArray<TelDigit> storeDigits(m_storeDigitsName);
#ifdef MAKE_TELCLUSTERS
  const StoreArray<TelCluster> storeClusters(m_storeClustersName);
#else
  const StoreArray<PXDCluster> storeClusters(m_storeClustersName);
#endif
  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // Fired pixels
  vector<int> pixel_count(c_nTELPlanes);
  for (int i = 0; i < c_nTELPlanes; i++) pixel_count[i] = 0;
  for (const TelDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTELPlane) || (iPlane > c_lastTELPlane)) continue;
    int index = planeToIndex(iPlane);
    pixel_count[index]++;
  }
  for (int i = 0; i < c_nTELPlanes; i++) m_fired[i]->Fill(pixel_count[i]);

  // Cluster counts - same as Pixels
  vector<int> cluster_count(c_nTELPlanes);
  for (int i = 0; i < c_nTELPlanes; i++) cluster_count[i] = 0;
  for (auto cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTELPlane) || (iPlane > c_lastTELPlane)) continue;
    int index = planeToIndex(iPlane);
    cluster_count[index]++;
  }
  for (int i = 0; i < c_nTELPlanes; i++) {
    m_clusters[i]->Fill(cluster_count[i]);
  }

  // Hitmaps, Size
  for (auto cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTELPlane) || (iPlane > c_lastTELPlane)) continue;
    int index = planeToIndex(iPlane);
    if ((getInfo(index).getUCellID(cluster.getU()) < -1) || (getInfo(index).getVCellID(cluster.getV()) < -1)) continue;
    m_hitMapU[index]->Fill(getInfo(index).getUCellID(cluster.getU()));
    m_hitMapV[index]->Fill(getInfo(index).getVCellID(cluster.getV()));
    m_sizeU[index]->Fill(cluster.getUSize());
    m_sizeV[index]->Fill(cluster.getVSize());
    m_size[index]->Fill(cluster.getSize());
  }

  // Correlations in U + V, 2D Hitmaps
  for (int i1 = 0; i1 < storeClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    auto cluster1 = *storeClusters[i1];
    int iPlaneL1 = cluster1.getSensorID().getLayerNumber();
    if (iPlaneL1 != 7) continue;
    int iPlane1 = cluster1.getSensorID().getSensorNumber();
    if ((iPlane1 < c_firstTELPlane) || (iPlane1 > c_lastTELPlane)) continue;
    int index1 = planeToIndex(iPlane1);
    if ((getInfo(index1).getUCellID(cluster1.getU()) < -1) || (getInfo(index1).getVCellID(cluster1.getV()) < -1)) continue;
    // hit maps:
    m_correlationsHitMaps[c_nTELPlanes * index1 + index1]->Fill(getInfo(index1).getUCellID(cluster1.getU()),
                                                                getInfo(index1).getVCellID(cluster1.getV()));
    for (int i2 = 0; i2 < storeClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      auto cluster2 = *storeClusters[i2];
      int iPlaneL2 = cluster2.getSensorID().getLayerNumber();
      if (iPlaneL2 != 7) continue;
      int iPlane2 = cluster2.getSensorID().getSensorNumber();
      if ((iPlane2 < c_firstTELPlane) || (iPlane2 > c_lastTELPlane)) continue;
      int index2 = planeToIndex(iPlane2);
      if ((getInfo(index2).getUCellID(cluster2.getU()) < -1) || (getInfo(index2).getVCellID(cluster2.getV()) < -1)) continue;
      // ready to fill correlation histograms:
      if (index1 < index2) {
        // correlations for u
        m_correlationsHitMaps[c_nTELPlanes * index2 + index1]->Fill(getInfo(index1).getUCellID(cluster1.getU()),
                                                                    getInfo(index2).getUCellID(cluster2.getU()));
      } else if (index1 > index2) {
        // correlations for v
        m_correlationsHitMaps[c_nTELPlanes * index2 + index1]->Fill(getInfo(index2).getVCellID(cluster2.getV()),
                                                                    getInfo(index1).getVCellID(cluster1.getV()));
      }
    }
  }
}


void TelDQMModule::endRun()
{
}


void TelDQMModule::terminate()
{
}
