#include "testbeam/vxd/modules/DQM/TelxVXDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

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
REG_MODULE(TelxVXD)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelxVXDModule::TelxVXDModule() : HistoModule()
{
  //Set module properties
  setDescription("Tel x VXD DQM module adds some correlation plots between telescopes and nearest VXD sensors.");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("Clusters", m_storeTelClustersName, "Name of the telescopes cluster collection",
           std::string("PXDClusters")); // always be explicit about this, can cause trouble
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed", std::string("tel-vxd"));
}


TelxVXDModule::~TelxVXDModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TelxVXDModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  //----------------------------------------------------------------
  // Charge of clusters : hClusterCharge[U/V][PlaneNo]
  //----------------------------------------------------------------

  int iPlane = indexToPlane(4);
  string name = str(format("hPXDClusterChargeU%1%") % iPlane);
  string title = str(format("PXD cluster charge, plane %1%") % iPlane);
  m_chargePXD2 = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
  m_chargePXD2->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargePXD2->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(5);
  name = str(format("hSVDClusterChargeU%1%") % iPlane);
  title = str(format("SVD cluster charge in U, plane %1%") % iPlane);
  m_chargeUSVD1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
  m_chargeUSVD1->GetXaxis()->SetTitle("charge of u clusters [ADU]");
  m_chargeUSVD1->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(5);
  name = str(format("hSVDClusterChargeV%1%") % iPlane);
  title = str(format("SVD cluster charge in V, plane %1%") % iPlane);
  m_chargeVSVD1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
  m_chargeVSVD1->GetXaxis()->SetTitle("charge of v clusters [ADU]");
  m_chargeVSVD1->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(2);
  name = str(format("hTelClusterCharge%1%") % iPlane);
  title = str(format("Tel cluster charge, plane %1%") % iPlane);
  m_chargeTel3 = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
  m_chargeTel3->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargeTel3->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(9);
  name = str(format("hTelClusterCharge%1%") % iPlane);
  title = str(format("Tel cluster charge, plane %1%") % iPlane);
  m_chargeTel4 = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
  m_chargeTel4->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargeTel4->GetYaxis()->SetTitle("count");

  //----------------------------------------------------------------
  // 2D Hitmaps for some TB detectors in local uv coordinates : m_hitMapxxx
  //----------------------------------------------------------------

  int i = 2;
  iPlane = indexToPlane(i);
  name = str(format("hTELHitmap%1%") % iPlane);
  title = str(format("TEL Hitmap, plane %1%") % iPlane);
  int nPixelsU = getInfoTel(i).getUCells();
  int nPixelsV = getInfoTel(i).getVCells();
  m_hitMapTel3 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapTel3->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapTel3->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapTel3->GetZaxis()->SetTitle("hits");

  i = 9;
  iPlane = indexToPlane(i);
  name = str(format("hTELHitmap%1%") % iPlane);
  title = str(format("TEL Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoTel(i).getUCells();
  nPixelsV = getInfoTel(i).getVCells();
  m_hitMapTel4 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapTel4->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapTel4->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapTel4->GetZaxis()->SetTitle("hits");

  i = 4;
  iPlane = indexToPlane(i);
  name = str(format("hPXDHitmap%1%") % iPlane);
  title = str(format("PXD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoPXD(i).getUCells();
  nPixelsV = getInfoPXD(i).getVCells();
  m_hitMapPXD2 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapPXD2->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapPXD2->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapPXD2->GetZaxis()->SetTitle("hits");

  i = 5;
  iPlane = indexToPlane(i);
  name = str(format("hSVDHitmap%1%") % iPlane);
  title = str(format("SVD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoSVD(i).getUCells();
  nPixelsV = getInfoSVD(i).getVCells();
  m_hitMapSVD1 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapSVD1->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapSVD1->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapSVD1->GetZaxis()->SetTitle("hits");

  i = 8;
  iPlane = indexToPlane(i);
  name = str(format("hSVDHitmap%1%") % iPlane);
  title = str(format("SVD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoSVD(i).getUCells();
  nPixelsV = getInfoSVD(i).getVCells();
  m_hitMapSVD4 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapSVD4->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapSVD4->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapSVD4->GetZaxis()->SetTitle("hits");

  //----------------------------------------------------------------
  // Correlations for some combination of TB detectors in local uv coordinates : m_correlationsxxxxxx[U/V]
  //----------------------------------------------------------------

  for (int i = 0; i < 2; i++) {
    string Axis;
    if (i)
      Axis = str(format("v"));
    else
      Axis = str(format("u"));
    string vAxis;
    int nPixelsU;
    int nPixelsV;

    string name = str(format("hCorrelationsTel3SVD1%1%") % Axis.data());
    string title = str(format("Corelation Tel3 SVD1, axis %1%") % Axis.data());
    string AxisU = str(format("%1% position Tel3 [pitch units]") % Axis.data());
    string AxisV = str(format("%1% position SVD1 [pitch units]") % Axis.data());
    if (i) {
      nPixelsU = getInfoTel(2).getVCells();
      nPixelsV = getInfoSVD(5).getVCells();
    } else {
      nPixelsU = getInfoTel(2).getUCells();
      nPixelsV = getInfoSVD(5).getUCells();
    }
    m_correlationsTel3SVD1[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_correlationsTel3SVD1[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel3SVD1[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel3SVD1[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsTel4SVD4%1%") % Axis.data());
    title = str(format("Corelation Tel4 SVD4, axis %1%") % Axis.data());
    AxisU = str(format("%1% position Tel4 [pitch units]") % Axis.data());
    AxisV = str(format("%1% position SVD4 [pitch units]") % Axis.data());
    if (i) {
      nPixelsU = getInfoTel(9).getVCells();
      nPixelsV = getInfoSVD(8).getVCells();
    } else {
      nPixelsU = getInfoTel(9).getUCells();
      nPixelsV = getInfoSVD(8).getUCells();
    }
    m_correlationsTel4SVD4[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_correlationsTel4SVD4[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel4SVD4[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel4SVD4[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsTel3PXD2%1%") % Axis.data());
    title = str(format("Corelation Tel3 PXD2, axis %1%") % Axis.data());
    AxisU = str(format("%1% position Tel3 [pitch units]") % Axis.data());
    AxisV = str(format("%1% position PXD2 [pitch units]") % Axis.data());
    if (i) {
      nPixelsU = getInfoTel(2).getVCells();
      nPixelsV = getInfoPXD(4).getVCells();
    } else {
      nPixelsU = getInfoTel(2).getUCells();
      nPixelsV = getInfoPXD(4).getUCells();
    }
    m_correlationsTel3PXD2[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_correlationsTel3PXD2[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel3PXD2[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel3PXD2[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsSVD1PXD2%1%") % Axis.data());
    title = str(format("Corelation SVD1 PXD2, axis %1%") % Axis.data());
    AxisU = str(format("%1% position SVD1 [pitch units]") % Axis.data());
    AxisV = str(format("%1% position PXD2 [pitch units]") % Axis.data());
    if (i) {
      nPixelsU = getInfoSVD(5).getVCells();
      nPixelsV = getInfoPXD(4).getVCells();
    } else {
      nPixelsU = getInfoSVD(5).getUCells();
      nPixelsV = getInfoPXD(4).getUCells();
    }
    m_correlationsSVD1PXD2[i] = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    m_correlationsSVD1PXD2[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsSVD1PXD2[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsSVD1PXD2[i]->GetZaxis()->SetTitle("hits");
//    m_correlationsTel3SVD1SP[i]->Reset();
//    m_correlationsTel4SVD4SP[i]->Reset();
//    m_correlationsTel3PXD2SP[i]->Reset();
//    m_correlationsSVD1PXD2SP[i]->Reset();
  }

  oldDir->cd();
}


void TelxVXDModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);

  //storeClusters.required();
  //storeDigits.required();

  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  //Store names to speed up creation later
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeTelClustersName = storeTelClusters.getName();
  m_storeSVDDigitsName = storeSVDDigits.getName();
  m_storeTelDigitsName = storeTelDigits.getName();
  m_relSVDClusterDigitName = relSVDClusterDigits.getName();
  m_relTelClusterDigitName = relTelClusterDigits.getName();
}

void TelxVXDModule::beginRun()
{
  //auto geo = VXD::GeoCache::getInstance();
  //VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_chargePXD2->Reset();
  m_chargeUSVD1->Reset();
  m_chargeVSVD1->Reset();
  m_chargeTel3->Reset();
  m_chargeTel4->Reset();

  m_hitMapTel3->Reset();
  m_hitMapTel4->Reset();
  m_hitMapPXD2->Reset();
  m_hitMapSVD1->Reset();
  m_hitMapSVD4->Reset();

  // Just to make sure, reset all the histograms.
  for (int i = 0; i < 2; i++) {
    m_correlationsTel3SVD1[i]->Reset();
    m_correlationsTel4SVD4[i]->Reset();
    m_correlationsTel3PXD2[i]->Reset();
    m_correlationsSVD1PXD2[i]->Reset();
    //m_correlationsTel3SVD1SP[i]->Reset();
    //m_correlationsTel4SVD4SP[i]->Reset();
    //m_correlationsTel3PXD2SP[i]->Reset();
    //m_correlationsSVD1PXD2SP[i]->Reset();
  }

}


void TelxVXDModule::event()
{

  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);

  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);
  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);

  // m_chargeUSVD1, m_chargeVSVD1
  for (const SVDCluster & cluster : storeSVDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane + 3 < c_firstSVDPlane) || (iPlane + 3 > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane + 3);
    if (index != 5) continue;
    if (cluster.isUCluster()) {
      m_chargeUSVD1->Fill(cluster.getCharge());
    } else {
      m_chargeVSVD1->Fill(cluster.getCharge());
    }
  }

//  m_chargePXD2, m_hitMapPXD2
  for (const PXDCluster & cluster : storePXDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane + 3 < c_firstPXDPlane) || (iPlane + 3 > c_lastPXDPlane)) continue;
    int index = planeToIndex(iPlane + 3);
    if (index != 4) continue;
    m_chargePXD2->Fill(cluster.getCharge());
    m_hitMapPXD2->Fill(getInfoPXD(index).getUCellID(cluster.getU()), getInfoPXD(index).getVCellID(cluster.getV()));
  }
//  m_chargeTel3, m_chargeTel4, m_hitMapTel3, m_hitMapTel4
  for (const PXDCluster & cluster : storeTelClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    int iPlane2 = cluster.getSensorID().getSensorNumber();
    if (iPlane != 7) continue;
    if ((iPlane2 < 1) || (iPlane2 > 6)) continue;
    if (iPlane2 == 3) {
      int index = 2;
      m_chargeTel3->Fill(cluster.getCharge());
      m_hitMapTel3->Fill(getInfoTel(index).getUCellID(cluster.getU()), getInfoTel(index).getVCellID(cluster.getV()));
    }
    if (iPlane2 == 4) {
      int index = 9;
      m_chargeTel4->Fill(cluster.getCharge());
      m_hitMapTel4->Fill(getInfoTel(index).getUCellID(cluster.getU()), getInfoTel(index).getVCellID(cluster.getV()));
    }
  }

// m_hitMapSVD1, m_hitMapSVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iPlane1 = 0;
    const SVDCluster& cluster1 = *storeSVDClusters[i1];
    iPlane1 = cluster1.getSensorID().getLayerNumber();
    if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
    index1 = planeToIndex(iPlane1 + 3);
    if (cluster1.isUCluster()) {
      iIsU1 = 1;
      fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
    }
    for (int i2 = 0; i2 < storeSVDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      const SVDCluster& cluster2 = *storeSVDClusters[i2];
      iPlane2 = cluster2.getSensorID().getLayerNumber();
      if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
      index2 = planeToIndex(iPlane2 + 3);
      if (!cluster2.isUCluster()) {
        iIsV2 = 1;
        fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
      }
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
        if (index1 == 5)
          m_hitMapSVD1->Fill(fPosU1, fPosV2);
        if (index1 == 8)
          m_hitMapSVD4->Fill(fPosU1, fPosV2);
      }
    }
  }

  // Correlations for local u v coordinates

// m_correlationsSVD1PXD2[u/v]
  int SelDet1 = 5;  // SVD1
  int SelDet2 = 4;  // PXD2
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstPXDPlane) || (iPlane1 + 3 > c_lastPXDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosV1 = getInfoPXD(index1).getVCellID(clusterPXD1.getV());
      fPosU1 = getInfoPXD(index1).getUCellID(clusterPXD1.getU());
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstPXDPlane) || (iPlane2 + 3 > c_lastPXDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosV2 = getInfoPXD(index2).getVCellID(clusterPXD2.getV());
        fPosU2 = getInfoPXD(index2).getUCellID(clusterPXD2.getU());
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsSVD1PXD2[0]->Fill(fPosU2, fPosU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsSVD1PXD2[1]->Fill(fPosV1, fPosV2);
      }
    }
  }

// m_correlationsTel3PXD2[u/v]
  SelDet1 = 2;  // Tel3
  SelDet2 = 4;  // PXD2
  for (int i1 = 0; i1 < storeTelClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstPXDPlane) || (iPlane1 + 3 > c_lastPXDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosV1 = getInfoPXD(index1).getVCellID(clusterPXD1.getV());
      fPosU1 = getInfoPXD(index1).getUCellID(clusterPXD1.getU());
    } else {                                  // Tel clusters:
      const PXDCluster& clusterPXD1 = *storeTelClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      iPlane1 = indexToPlane(index1);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeTelClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstPXDPlane) || (iPlane2 + 3 > c_lastPXDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosV2 = getInfoPXD(index2).getVCellID(clusterPXD2.getV());
        fPosU2 = getInfoPXD(index2).getUCellID(clusterPXD2.getU());
      } else {                                  // Tel clusters:
        const PXDCluster& clusterPXD2 = *storeTelClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        iPlane2 = indexToPlane(index2);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel3PXD2[0]->Fill(fPosU2, fPosU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel3PXD2[1]->Fill(fPosV1, fPosV2);
      }
    }
  }


// m_correlationsTel4SVD4[u/v]
  SelDet1 = 9;  // Tel4
  SelDet2 = 8;  // SVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storeTelClusters.getEntries()) {  // Tel clusters:
      const PXDCluster& clusterPXD1 = *storeTelClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      iPlane1 = indexToPlane(index1);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storeTelClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storeTelClusters.getEntries()) {  // Tel clusters:
        const PXDCluster& clusterPXD2 = *storeTelClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        iPlane2 = indexToPlane(index2);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storeTelClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel4SVD4[0]->Fill(fPosU2, fPosU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel4SVD4[1]->Fill(fPosV1, fPosV2);
      }
    }
  }

// m_correlationsTel3SVD1[u/v]
  SelDet1 = 2;  // Tel3
  SelDet2 = 5;  // SVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storeTelClusters.getEntries()) {  // Tel clusters:
      const PXDCluster& clusterPXD1 = *storeTelClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      iPlane1 = indexToPlane(index1);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storeTelClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storeTelClusters.getEntries()) {  // Tel clusters:
        const PXDCluster& clusterPXD2 = *storeTelClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        iPlane2 = indexToPlane(index2);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storeTelClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel3SVD1[0]->Fill(fPosU1, fPosU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel3SVD1[1]->Fill(fPosV2, fPosV1);
      }
    }
  }

}


void TelxVXDModule::endRun()
{
}


void TelxVXDModule::terminate()
{
}
