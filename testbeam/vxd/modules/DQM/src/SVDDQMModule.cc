#include "testbeam/vxd/modules/DQM/SVDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::SVD;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMModule::SVDDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}


SVDDQMModule::~SVDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMModule::defineHisto()
{
  // Fired strip counts U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hFiredU%1%") % iPlane);
    string title = str(format("Fired pixels in U, plane %1%") % iPlane);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
  }
  // Fired strip counts V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hFiredV%1%") % iPlane);
    string title = str(format("Fired pixels in V, plane %1%") % iPlane);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hClustersU%1%") % iPlane);
    string title = str(format("Number of clusters in U, plane %1%") % iPlane);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hClustersV%1%") % iPlane);
    string title = str(format("Number of clusters in V, plane %1%") % iPlane);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
  }
  // Hitmaps in U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hHitmapU%1%") % iPlane);
    string title = str(format("Hitmap in U, plane %1%") % iPlane);
    float length = getInfo(i).getLength();
    int nStrips = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * length, 0.5 * length);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hHitmapV%1%") % iPlane);
    string title = str(format("Hitmap in V, plane %1%") % iPlane);
    float width = getInfo(i).getWidth();
    int nStrips = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * width, 0.5 * width);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
  // u charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hChargeU%1%") % iPlane);
    string title = str(format("Charge in U, plane %1%") % iPlane);
    m_chargeU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150000);
    m_chargeU[i]->GetXaxis()->SetTitle("charge at u strips");
    m_chargeU[i]->GetYaxis()->SetTitle("count");
  }
  // v charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hChargeV%1%") % iPlane);
    string title = str(format("Charge in V, plane %1%") % iPlane);
    m_chargeV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150000);
    m_chargeV[i]->GetXaxis()->SetTitle("charge at v strips");
    m_chargeV[i]->GetYaxis()->SetTitle("count");
  }
  // u seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSeedU%1%") % iPlane);
    string title = str(format("Seed in U, plane %1%") % iPlane);
    m_seedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150000);
    m_seedU[i]->GetXaxis()->SetTitle("seed at u strips");
    m_seedU[i]->GetYaxis()->SetTitle("count");
  }
  // v seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSeedV%1%") % iPlane);
    string title = str(format("Seed in V, plane %1%") % iPlane);
    m_seedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150000);
    m_seedV[i]->GetXaxis()->SetTitle("seed at v strips");
    m_seedV[i]->GetYaxis()->SetTitle("count");
  }
  // u size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSizeU%1%") % iPlane);
    string title = str(format("Size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("size at u strips");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSizeV%1%") % iPlane);
    string title = str(format("Size in V, plane %1%") % iPlane);
    m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeV[i]->GetXaxis()->SetTitle("size at v strips");
    m_sizeV[i]->GetYaxis()->SetTitle("count");
  }
  // u time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTimeU%1%") % iPlane);
    string title = str(format("Time in U, plane %1%") % iPlane);
    m_timeU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_timeU[i]->GetXaxis()->SetTitle("time at u strips");
    m_timeU[i]->GetYaxis()->SetTitle("count");
  }
  // v time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hTimeV%1%") % iPlane);
    string title = str(format("Time in V, plane %1%") % iPlane);
    m_timeV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_timeV[i]->GetXaxis()->SetTitle("time at v strips");
    m_timeV[i]->GetYaxis()->SetTitle("count");
  }

  // Correlations in U + V, 2D Hitmaps
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane1 = indexToPlane(i);
    float length1 = getInfo(i).getLength();
    int nStripsU1 = getInfo(i).getUCells();
    float width1 = getInfo(i).getWidth();
    int nStripsV1 = getInfo(i).getVCells();
    for (int j = 0; j < c_nSVDPlanes; j++) {
      int iPlane2 = indexToPlane(j);
      float length2 = getInfo(j).getLength();
      int nStripsU2 = getInfo(j).getUCells();
      float width2 = getInfo(j).getWidth();
      int nStripsV2 = getInfo(j).getVCells();
      if (i == j) {  // hit maps
        string name = str(format("h2HitmapUV%1%") % iPlane2);
        string title = str(format("Hitmap in U x V, plane %1%") % iPlane2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU2, -0.5 * length2, 0.5 * length2, nStripsV2, -0.5 * width2, 0.5 * width2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetXaxis()->SetTitle("u position");
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetYaxis()->SetTitle("v position");
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else if (i < j) { // correlations for u
        string name = str(format("h2CorellationmapU%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("Corellationmap in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU1, -0.5 * length1, 0.5 * length1, nStripsU2, -0.5 * length2, 0.5 * length2);
        string axisxtitle = str(format("u position, plane %1%") % iPlane1);
        string axisytitle = str(format("u position, plane %1%") % iPlane2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else {       // correlations for v
        string name = str(format("h2CorellationmapV%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("Corellationmap in V, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsV1, -0.5 * width1, 0.5 * width1, nStripsV2, -0.5 * width2, 0.5 * width2);
        string axisxtitle = str(format("v position, plane %1%") % iPlane1);
        string axisytitle = str(format("v position, plane %1%") % iPlane2);
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_corellationsHitMaps[c_nSVDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }
}


void SVDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  //storeClusters.required();
  //storeDigits.required();

  RelationArray relClusterDigits(storeClusters, storeDigits);

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
}

void SVDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Reset();
    m_firedV[i]->Reset();
    m_clustersU[i]->Reset();
    m_clustersV[i]->Reset();
    m_hitMapU[i]->Reset();
    m_hitMapV[i]->Reset();
  }
  for (int i = 0; i < c_nSVDPlanes * c_nSVDPlanes; i++) {
    m_corellationsHitMaps[i]->Reset();
  }
}


void SVDDQMModule::event()
{
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // Fired strips
  vector< set<int> > uStrips(c_nSVDPlanes); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDPlanes);
  for (const SVDDigit & digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (digit.isUStrip())
      uStrips.at(index).insert(digit.getCellID());
    else
      vStrips.at(index).insert(digit.getCellID());
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Fill(0.1667 * uStrips[i].size()); // FIXME:Account for 6 samples
    m_firedV[i]->Fill(0.1667 * vStrips[i].size());
  }

  // Cluster counts - same as strips
  int countsU[c_nSVDPlanes];
  int countsV[c_nSVDPlanes];
  for (int i = 0; i < c_nSVDPlanes; i++) {
    countsU[i] = 0; countsV[i] = 0;
  }
  for (const SVDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (cluster.isUCluster())
      countsU[index]++;
    else
      countsV[index]++;
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_clustersU[i]->Fill(countsU[i]);
    m_clustersV[i]->Fill(countsV[i]);
  }

  // Hitmaps, Charge, Seed, Size, Time
  for (const SVDCluster & cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    if (cluster.isUCluster()) {
      m_hitMapU[index]->Fill(cluster.getPosition());
      m_chargeU[index]->Fill(cluster.getCharge());
      m_seedU[index]->Fill(cluster.getSeedCharge());
      m_sizeU[index]->Fill(cluster.getSize());
      m_timeU[index]->Fill(cluster.getClsTime());
    } else {
      m_hitMapV[index]->Fill(cluster.getPosition());
      m_chargeV[index]->Fill(cluster.getCharge());
      m_seedV[index]->Fill(cluster.getSeedCharge());
      m_sizeV[index]->Fill(cluster.getSize());
      m_timeV[index]->Fill(cluster.getClsTime());
    }
  }


  // Correlations
  // Threshold for acceptance hit signal to correlation, for every plane, every direction
  // Threshold for acceptance hit time distance to correlation, common for all SVD, not PXD
  // Finaly should be read from external xml cfg file for test beam
  float CutDQMCorrelSigU[c_nSVDPlanes];
  float CutDQMCorrelSigV[c_nSVDPlanes];
  float CutDQMCorrelTime;
  for (int i = 0; i < c_nSVDPlanes; i++) {
    CutDQMCorrelSigU[i] = 300;
    CutDQMCorrelSigV[i] = 300;
  }
  CutDQMCorrelTime = 70;  // ns

  for (const SVDCluster & cluster1 : storeClusters) {
    int iPlane1 = cluster1.getSensorID().getLayerNumber();
    if ((iPlane1 < c_firstSVDPlane) || (iPlane1 > c_lastSVDPlane)) continue;
    int index1 = planeToIndex(iPlane1);
    float fCharge1 = cluster1.getCharge();
    if (cluster1.isUCluster()) {
      if (fCharge1 < CutDQMCorrelSigU[index1]) continue;
    } else {
      if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
    }
    float fTime1 = cluster1.getClsTime();
    for (const SVDCluster & cluster2 : storeClusters) {
      int iPlane2 = cluster2.getSensorID().getLayerNumber();
      if ((iPlane2 < c_firstSVDPlane) || (iPlane2 > c_lastSVDPlane)) continue;
      int index2 = planeToIndex(iPlane2);
      float fCharge2 = cluster2.getCharge();
      if (cluster2.isUCluster()) {
        if (fCharge2 < CutDQMCorrelSigU[index2]) continue;
      } else {
        if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
      }
      float fTime2 = cluster2.getClsTime();
      if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      if ((index1 == index2) && (cluster1.isUCluster() != cluster2.isUCluster()) && (cluster1.isUCluster())) {  // hit maps
        m_corellationsHitMaps[c_nSVDPlanes * index2 + index1]->Fill(cluster1.getPosition(), cluster2.getPosition());
      } else if ((index1 < index2) && (cluster1.isUCluster() == cluster2.isUCluster()) && (cluster1.isUCluster())) { // correlations for u
        if (cluster1.isUCluster())
          m_corellationsHitMaps[c_nSVDPlanes * index2 + index1]->Fill(cluster1.getPosition(), cluster2.getPosition());
        else
          m_corellationsHitMaps[c_nSVDPlanes * index2 + index1]->Fill(cluster1.getPosition(), cluster2.getPosition());
      } else if ((index1 > index2) && (cluster1.isUCluster() == cluster2.isUCluster()) && (cluster1.isUCluster() == 0)) { // correlations for v
        if (cluster1.isUCluster())
          m_corellationsHitMaps[c_nSVDPlanes * index2 + index1]->Fill(cluster1.getPosition(), cluster2.getPosition());
        else
          m_corellationsHitMaps[c_nSVDPlanes * index2 + index1]->Fill(cluster1.getPosition(), cluster2.getPosition());
      }
    }
  }

}


void SVDDQMModule::endRun()
{
}


void SVDDQMModule::terminate()
{
}
