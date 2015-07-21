#include "svd/modules/svdDQM/SVDDQM2Module.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

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

const char* const SVDDQM2Module::type[] = {"Slanted", "Barrel"}; // array of strings used to name sensor type in histo

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQM2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQM2Module::SVDDQM2Module() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM 2 module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("svd"));
}

SVDDQM2Module::~SVDDQM2Module()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQM2Module::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  //----------------------------------------------------------------
  // Number of fired strips per event : hSVDFired[PlaneNo]
  //----------------------------------------------------------------
  // Fired strip counts
  //----------------------------------------------------------------
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDFired%1%") % iPlane);
    string title = str(format("Number of SVD fired strips, plane %1%") % iPlane);
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 200, 3000);
    m_fired[i]->GetXaxis()->SetTitle("# of fired strips");
    m_fired[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Number of clusters per event : hSVDClusters[PlaneNo]
  //----------------------------------------------------------------
  // Number of clusters
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusters%1%") % iPlane);
    string title = str(format("Number of SVD clusters, plane %1%") % iPlane);
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of clusters");
    m_clusters[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Hitmaps: Number of clusters by coordinate : hSVDHitmap[U/V/UV][PlaneNo]
  //----------------------------------------------------------------
// Hitmaps in U Slanted
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapUSlanted%1%") % iPlane);
    string title = str(format("SVD Hitmap in U, Slanted, plane %1%") % iPlane);
    int nStrips = getInfo(i, 0).getUCells();
    m_hitMapUSlanted[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapUSlanted[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapUSlanted[i]->GetYaxis()->SetTitle("hits");
  }

// Hitmaps in U Barrel
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapUBarrel%1%") % iPlane);
    string title = str(format("SVD Hitmap in U, Barrel, plane %1%") % iPlane);
    int nStrips = getInfo(i, 1).getUCells();
    m_hitMapUBarrel[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapUBarrel[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapUBarrel[i]->GetYaxis()->SetTitle("hits");
  }


// Hitmaps in V Slanted
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapVSlanted%1%") % iPlane);
    string title = str(format("SVD Hitmap in V, Slanted, plane %1%") % iPlane);
    int nStrips = getInfo(i, 0).getVCells();
    m_hitMapVSlanted[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapVSlanted[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapVSlanted[i]->GetYaxis()->SetTitle("hits");
  }

// Hitmaps in V Barrel
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapVBarrel%1%") % iPlane);
    string title = str(format("SVD Hitmap in V, Barrel, plane %1%") % iPlane);
    int nStrips = getInfo(i, 1).getVCells();
    m_hitMapVBarrel[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapVBarrel[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapVBarrel[i]->GetYaxis()->SetTitle("hits");
  }

  //----------------------------------------------------------------
  // Charge of digits : hSVDDigitCharge[PlaneNo]
  //----------------------------------------------------------------
  // charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDDigitCharge%1%") % iPlane);
    string title = str(format("SVD strip charge, plane %1%") % iPlane);
    m_digitCharge[i] = new TH1F(name.c_str(), title.c_str(), 500, 0, 500);
    m_digitCharge[i]->GetXaxis()->SetTitle("strip charge [ADU]");
    m_digitCharge[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Charge of clusters : hSVDClusterCharge[PlaneNo]
  //----------------------------------------------------------------
  // charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterCharge%1%") % iPlane);
    string title = str(format("SVD cluster charge, plane %1%") % iPlane);
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), 500, 0, 500);
    m_clusterCharge[i]->GetXaxis()->SetTitle("cluster charge [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster seed charge distribution : hSVDSeed[PlaneNo]
  //----------------------------------------------------------------
  // seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSeed%1%") % iPlane);
    string title = str(format("SVD seed charge, plane %1%") % iPlane);
    m_seedCharge[i] = new TH1F(name.c_str(), title.c_str(), 500, 0, 500);
    m_seedCharge[i]->GetXaxis()->SetTitle("seed charge [ADU]");
    m_seedCharge[i]->GetYaxis()->SetTitle("count");
  }

  //----------------------------------------------------------------
  // Cluster size distribution : hSVDSize[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSizeU%1%") % iPlane);
    string title = str(format("SVD cluster size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("cluster size in u");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSizeV%1%") % iPlane);
    string title = str(format("SVD cluster size in V, plane %1%") % iPlane);
    m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeV[i]->GetXaxis()->SetTitle("cluster size in v");
    m_sizeV[i]->GetYaxis()->SetTitle("count");
  }
  // cd back to root directory
  oldDir->cd();
}

void SVDDQM2Module::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  RelationArray relClusterDigits(storeClusters, storeDigits);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
}

void SVDDQM2Module::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_fired[i]->Reset();
    m_clusters[i]->Reset();
    m_digitCharge[i]->Reset();
    m_clusterCharge[i]->Reset();
    m_seedCharge[i]->Reset();
    m_sizeU[i]->Reset();
    m_sizeV[i]->Reset();
    m_hitMapUSlanted[i]->Reset();
    m_hitMapUBarrel[i]->Reset();
    m_hitMapVSlanted[i]->Reset();
    m_hitMapVBarrel[i]->Reset();
  }
}

void SVDDQM2Module::event()
{

  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // If there are no strips, leave
  if (!storeDigits || !storeDigits.getEntries()) return;
  // Fired strips
  vector<int> strip_count(c_nSVDPlanes);
  for (int i = 0; i < c_nSVDPlanes; i++) strip_count[i] = 0;
  for (const SVDDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    strip_count[index]++;
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_fired[i]->Fill(strip_count[i]);
  }
  // Cluster counts - same as Strips
  vector<int> cluster_count(c_nSVDPlanes);
  for (int i = 0; i < c_nSVDPlanes; i++) cluster_count[i] = 0;
  for (const SVDCluster& cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    cluster_count[index]++;
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_clusters[i]->Fill(cluster_count[i]);
  }

  // Digit charge
  for (const SVDDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    m_digitCharge[index]->Fill(digit.getCharge());
  }

  // Hitmaps, Charge, Seed, Size, Time
  for (const SVDCluster& cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int layerIndex = planeToIndex(iPlane);
    int sensorIndex = sensorToIndex(iSensor, iPlane);

    if (cluster.isUCluster()) {

      if (sensorIndex == 0) { // fill slanted
        m_hitMapUSlanted[layerIndex]->Fill(
          getInfo(layerIndex, sensorIndex).getUCellID(cluster.getPosition()));
      } else { // fill barrel
        m_hitMapUBarrel[layerIndex]->Fill(
          getInfo(layerIndex, sensorIndex).getUCellID(cluster.getPosition()));
      }

      m_sizeU[layerIndex]->Fill(cluster.getSize());
    } else {
      if (sensorIndex == 0) { // fill slanted
        m_hitMapVSlanted[layerIndex]->Fill(
          getInfo(layerIndex, sensorIndex).getVCellID(cluster.getPosition()));
      } else { // fill barrel
        m_hitMapVBarrel[layerIndex]->Fill(
          getInfo(layerIndex, sensorIndex).getVCellID(cluster.getPosition()));
      }

      m_sizeV[layerIndex]->Fill(cluster.getSize());
    }

    m_clusterCharge[layerIndex]->Fill(cluster.getCharge());
    m_seedCharge[layerIndex]->Fill(cluster.getSeedCharge());

  }
}

void SVDDQM2Module::endRun()
{
}

void SVDDQM2Module::terminate()
{
}
