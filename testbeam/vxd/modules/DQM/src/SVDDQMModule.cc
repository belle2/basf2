#include "testbeam/vxd/modules/DQM/SVDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

//#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

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
  //----------------------------------------------------------------
  // Number of fired strips per frame : hFired[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Fired strip counts U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDFiredU%1%") % iPlane);
    string title = str(format("SVD Fired pixels in U, plane %1%") % iPlane);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
  }
  // Fired strip counts V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDFiredV%1%") % iPlane);
    string title = str(format("SVD Fired pixels in V, plane %1%") % iPlane);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Number of clusters per frame : hClusters[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Number of clusters U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClustersU%1%") % iPlane);
    string title = str(format("Number of SVD clusters in U, plane %1%") % iPlane);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClustersV%1%") % iPlane);
    string title = str(format("Number of SVD clusters in V, plane %1%") % iPlane);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Hitmaps in U
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapU%1%") % iPlane);
    string title = str(format("SVD Hitmap in U, plane %1%") % iPlane);
    float length = getInfo(i).getLength();
    int nStrips = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * length, 0.5 * length);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapV%1%") % iPlane);
    string title = str(format("SVD Hitmap in V, plane %1%") % iPlane);
    float width = getInfo(i).getWidth();
    int nStrips = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nStrips, -0.5 * width, 0.5 * width);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
  //----------------------------------------------------------------
  // Charge of clusters : hClusterCharge[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterChargeU%1%") % iPlane);
    string title = str(format("SVD cluster charge in U, plane %1%") % iPlane);
    m_chargeU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_chargeU[i]->GetXaxis()->SetTitle("charge of u clusters");
    m_chargeU[i]->GetYaxis()->SetTitle("count");
  }
  // v charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterChargeV%1%") % iPlane);
    string title = str(format("SVD cluster charge in V, plane %1%") % iPlane);
    m_chargeV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 150000);
    m_chargeV[i]->GetXaxis()->SetTitle("charge of v clusters");
    m_chargeV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster seed charge distribution : hSeedCharge[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSeedU%1%") % iPlane);
    string title = str(format("SVD seed charge in U, plane %1%") % iPlane);
    m_seedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_seedU[i]->GetXaxis()->SetTitle("seed charge of u clusters");
    m_seedU[i]->GetYaxis()->SetTitle("count");
  }
  // v seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSeedV%1%") % iPlane);
    string title = str(format("SVD seed charge in V, plane %1%") % iPlane);
    m_seedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_seedV[i]->GetXaxis()->SetTitle("seed charge of v clusters");
    m_seedV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster size distribution : hClusterSize[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterSizeU%1%") % iPlane);
    string title = str(format("SVD cluster size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterSizeV%1%") % iPlane);
    string title = str(format("SVD cluster size in V, plane %1%") % iPlane);
    m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_sizeV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster time distribution : hClusterTime[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterTimeU%1%") % iPlane);
    string title = str(format("SVD cluster time in U, plane %1%") % iPlane);
    m_timeU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_timeU[i]->GetXaxis()->SetTitle("time of u clusters");
    m_timeU[i]->GetYaxis()->SetTitle("count");
  }
  // v time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterTimeV%1%") % iPlane);
    string title = str(format("SVD cluster time in V, plane %1%") % iPlane);
    m_timeV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_timeV[i]->GetXaxis()->SetTitle("time of v clusters");
    m_timeV[i]->GetYaxis()->SetTitle("count");
  }

  // Correlations in U + V, 2D Hitmaps, also for PXD - SVD combination
  for (int i = 0; i < c_nVXDPlanes; i++) {
    int iPlane1 = indexToPlaneVXD(i);
    float length1, width1;
    int nStripsU1, nStripsV1;
    if (i >= c_nPXDPlanes) {  // SVD
      length1 = getInfo(i - c_nPXDPlanes).getLength();
      nStripsU1 = getInfo(i - c_nPXDPlanes).getUCells();
      width1 = getInfo(i - c_nPXDPlanes).getWidth();
      nStripsV1 = getInfo(i - c_nPXDPlanes).getVCells();
    } else { // PXD
      length1 = getInfoPXD(i).getLength();
      nStripsU1 = getInfoPXD(i).getUCells();
      width1 = getInfoPXD(i).getWidth();
      nStripsV1 = getInfoPXD(i).getVCells();
    }
    for (int j = 0; j < c_nVXDPlanes; j++) {
      int iPlane2 = indexToPlaneVXD(j);
      float length2, width2;
      int nStripsU2, nStripsV2;
      if (j >= c_nPXDPlanes) {  // SVD
        length2 = getInfo(j - c_nPXDPlanes).getLength();
        nStripsU2 = getInfo(j - c_nPXDPlanes).getUCells();
        width2 = getInfo(j - c_nPXDPlanes).getWidth();
        nStripsV2 = getInfo(j - c_nPXDPlanes).getVCells();
      } else { // PXD
        length2 = getInfoPXD(j).getLength();
        nStripsU2 = getInfoPXD(j).getUCells();
        width2 = getInfoPXD(j).getWidth();
        nStripsV2 = getInfoPXD(j).getVCells();
      }
      if (i == j) {  // hit maps
        string name = str(format("h2VXDHitmapUV%1%") % iPlane2);
        string title = str(format("Hitmap VXD in U x V, plane %1%") % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU2, -0.5 * length2, 0.5 * length2, nStripsV2, -0.5 * width2, 0.5 * width2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("u position");
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("v position");
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else if (i < j) { // correlations for u
        string name = str(format("h2VXDCorrelationmapU%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("Correlationmap VXD in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU1, -0.5 * length1, 0.5 * length1, nStripsU2, -0.5 * length2, 0.5 * length2);
        string axisxtitle = str(format("u position, plane %1%") % iPlane1);
        string axisytitle = str(format("u position, plane %1%") % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else {       // correlations for v
        string name = str(format("h2VXDCorrelationmapV%1%%2%") % iPlane2 % iPlane1);
        string title = str(format("Correlationmap VXD in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsV2, -0.5 * width2, 0.5 * width2, nStripsV1, -0.5 * width1, 0.5 * width1);
        string axisxtitle = str(format("v position, plane %1%") % iPlane2);
        string axisytitle = str(format("v position, plane %1%") % iPlane1);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }
}


void SVDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
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
  for (int i = 0; i < c_nVXDPlanes * c_nVXDPlanes; i++) {
    m_correlationsHitMaps[i]->Reset();
  }
}


void SVDDQMModule::event()
{

  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);

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
    m_firedU[i]->Fill(0.1667 * uStrips[i].size());
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
  float CutDQMCorrelSigU[c_nVXDPlanes];
  float CutDQMCorrelSigV[c_nVXDPlanes];
  float CutDQMCorrelTime;
  for (int i = 0; i < c_nPXDPlanes; i++) {  // PXD
    CutDQMCorrelSigU[i] = 0;
    CutDQMCorrelSigV[i] = 0;
  }
  for (int i = c_nPXDPlanes; i < c_nVXDPlanes; i++) {  // SVD
    CutDQMCorrelSigU[i] = 0;
    CutDQMCorrelSigV[i] = 0;
  }
  CutDQMCorrelTime = 70;  // ns

  for (int i1 = 0; i1 < storeClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int iIsPXD1 = 0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PCD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      float fCharge1 = clusterPXD1.getCharge();
      if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
      iIsPXD1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = clusterPXD1.getU();
      fPosV1 = clusterPXD1.getV();
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstSVDPlane) || (iPlane1 > c_lastSVDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      float fCharge1 = cluster1.getCharge();
      fTime1 = cluster1.getClsTime();
      if (cluster1.isUCluster()) {
        if (fCharge1 < CutDQMCorrelSigU[index1]) continue;
        iIsU1 = 1;
        fPosU1 = cluster1.getPosition();
      } else {
        if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
        iIsV1 = 1;
        fPosV1 = cluster1.getPosition();
      }
    }
    for (int i2 = 0; i2 < storeClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int iIsPXD2 = 0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PCD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        float fCharge2 = clusterPXD2.getCharge();
        if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
        iIsPXD2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = clusterPXD2.getU();
        fPosV2 = clusterPXD2.getV();
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstSVDPlane) || (iPlane2 > c_lastSVDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        float fCharge2 = cluster2.getCharge();
        fTime2 = cluster2.getClsTime();
        if (cluster2.isUCluster()) {
          if (fCharge2 < CutDQMCorrelSigU[index2]) continue;
          iIsU2 = 1;
          fPosU2 = cluster2.getPosition();
        } else {
          if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
          iIsV2 = 1;
          fPosV2 = cluster2.getPosition();
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0))
        if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
        m_correlationsHitMaps[c_nVXDPlanes * index2 + index1]->Fill(fPosU1, fPosV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsHitMaps[c_nVXDPlanes * index2 + index1]->Fill(fPosU1, fPosU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsHitMaps[c_nVXDPlanes * index2 + index1]->Fill(fPosV2, fPosV1);
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
