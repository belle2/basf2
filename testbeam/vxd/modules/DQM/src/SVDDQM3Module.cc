/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Combined TB DESY 2016                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "testbeam/vxd/modules/DQM/SVDDQM3Module.h"

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
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQM3)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQM3Module::SVDDQM3Module() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("svd"));
}


SVDDQM3Module::~SVDDQM3Module()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQM3Module::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* BasicDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
  // oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();
  //----------------------------------------------------------------
  // Number of fired strips per frame : hFired[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Fired strip counts U
  BasicDir->mkdir("JoinHistos_Basic")->cd();
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDFiredU%1%") % iPlane);
    string title = str(format("TB2016 SVD Fired pixels in U, plane %1%") % iPlane);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
  }
  // Fired strip counts V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDFiredV%1%") % iPlane);
    string title = str(format("TB2016 SVD Fired pixels in V, plane %1%") % iPlane);
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
    string title = str(format("TB2016 SVD Number of clusters in U, plane %1%") % iPlane);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
  }
  // Number of clusters V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClustersV%1%") % iPlane);
    string title = str(format("TB2016 SVD Number of clusters in V, plane %1%") % iPlane);
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
    string title = str(format("TB2016 SVD Hitmap in U, plane %1%") % iPlane);
    int nStrips = getInfo(i).getUCells();
    m_hitMapU[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapU[i]->GetYaxis()->SetTitle("hits");
  }
  // Hitmaps in V
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDHitmapV%1%") % iPlane);
    string title = str(format("TB2016 SVD Hitmap in V, plane %1%") % iPlane);
    int nStrips = getInfo(i).getVCells();
    m_hitMapV[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
    m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
    m_hitMapV[i]->GetYaxis()->SetTitle("hits");
  }
  //----------------------------------------------------------------
  // Charge of clusters : hClusterCharge[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterChargeU%1%") % iPlane);
    string title = str(format("TB2016 SVD cluster charge in U, plane %1%") % iPlane);
    m_chargeU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_chargeU[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
    m_chargeU[i]->GetYaxis()->SetTitle("count");
  }
  // v charge by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterChargeV%1%") % iPlane);
    string title = str(format("TB2016 SVD cluster charge in V, plane %1%") % iPlane);
    m_chargeV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_chargeV[i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
    m_chargeV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster seed charge distribution : hSeedCharge[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSeedU%1%") % iPlane);
    string title = str(format("TB2016 SVD seed charge in U, plane %1%") % iPlane);
    m_seedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_seedU[i]->GetXaxis()->SetTitle("seed charge of u clusters [ADU]");
    m_seedU[i]->GetYaxis()->SetTitle("count");
  }
  // v seed by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDSeedV%1%") % iPlane);
    string title = str(format("TB2016 SVD seed charge in V, plane %1%") % iPlane);
    m_seedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
    m_seedV[i]->GetXaxis()->SetTitle("seed charge of v clusters [ADU]");
    m_seedV[i]->GetYaxis()->SetTitle("count");
  }
  //----------------------------------------------------------------
  // Cluster size distribution : hClusterSize[U/V][PlaneNo]
  //----------------------------------------------------------------
  // u size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterSizeU%1%") % iPlane);
    string title = str(format("TB2016 SVD cluster size in U, plane %1%") % iPlane);
    m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_sizeU[i]->GetYaxis()->SetTitle("count");
  }
  // v size by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterSizeV%1%") % iPlane);
    string title = str(format("TB2016 SVD cluster size in V, plane %1%") % iPlane);
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
    string title = str(format("TB2016 SVD cluster time in U, plane %1%") % iPlane);
    m_timeU[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_timeU[i]->GetXaxis()->SetTitle("time of u clusters [ns]");
    m_timeU[i]->GetYaxis()->SetTitle("count");
  }
  // v time by plane
  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    string name = str(format("hSVDClusterTimeV%1%") % iPlane);
    string title = str(format("TB2016 SVD cluster time in V, plane %1%") % iPlane);
    m_timeV[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_timeV[i]->GetXaxis()->SetTitle("time of v clusters [ns]");
    m_timeV[i]->GetYaxis()->SetTitle("count");
  }

  BasicDir->mkdir("JoinHistos_Correlations")->cd();
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps for VXD in local uv coordinates : m_correlationsHitMaps[U/V][PlaneNo]
  //----------------------------------------------------------------
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps for VXD in for space points : m_correlationsHitMapsSP[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps, also for PXD - SVD combination
  for (int i = 0; i < c_nVXDPlanes; i++) {
    int iPlane1 = indexToPlaneVXD(i);
    float vSize1, uSize1;
    int nStripsU1, nStripsV1;
    if (i >= c_nPXDPlanes) {  // SVD
      vSize1 = getInfo(i - c_nPXDPlanes).getVSize();
      nStripsU1 = getInfo(i - c_nPXDPlanes).getUCells();
      uSize1 = getInfo(i - c_nPXDPlanes).getUSize();
      nStripsV1 = getInfo(i - c_nPXDPlanes).getVCells();
    } else { // PXD
      vSize1 = getInfoPXD(i).getVSize();
      nStripsU1 = getInfoPXD(i).getUCells();
      uSize1 = getInfoPXD(i).getUSize();
      nStripsV1 = getInfoPXD(i).getVCells();
    }
    for (int j = 0; j < c_nVXDPlanes; j++) {
      int iPlane2 = indexToPlaneVXD(j);
      float vSize2, uSize2;
      int nStripsU2, nStripsV2;
      if (j >= c_nPXDPlanes) {  // SVD
        vSize2 = getInfo(j - c_nPXDPlanes).getVSize();
        nStripsU2 = getInfo(j - c_nPXDPlanes).getUCells();
        uSize2 = getInfo(j - c_nPXDPlanes).getUSize();
        nStripsV2 = getInfo(j - c_nPXDPlanes).getVCells();
      } else { // PXD
        vSize2 = getInfoPXD(j).getVSize();
        nStripsU2 = getInfoPXD(j).getUCells();
        uSize2 = getInfoPXD(j).getUSize();
        nStripsV2 = getInfoPXD(j).getVCells();
      }
      if (i == j) {  // hit maps
        string name = str(format("h2VXDHitmapUV%1%") % iPlane2);
        string title = str(format("TB2016 Hitmap VXD in U x V, plane %1%") % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU2, 0, nStripsU2, nStripsV2, 0,
                                                               nStripsV2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("u position [pitch units]");
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("v position [pitch units]");
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2VXDHitmapSP%1%") % iPlane2);
        string titleSP = str(format("TB2016 Hitmap VXD in space points, plane %1%") % iPlane2);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU2, -0.5 * uSize2, 0.5 * uSize2,
                                                                 nStripsV2, -0.5 * vSize2, 0.5 * vSize2);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("vertical y position [cm]");
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("horizontal z position [cm]");
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else if (i < j) { // correlations for u
        string name = str(format("h2VXDCorrelationmapU%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("TB2016 Correlation map VXD in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU1, 0, nStripsU1, nStripsU2, 0,
                                                               nStripsU2);
        string axisxtitle = str(format("u position, plane %1% [pitch units]") % iPlane1);
        string axisytitle = str(format("u position, plane %1% [pitch units]") % iPlane2);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2VXDCorrelationmapSPU%1%%2%") % iPlane1 % iPlane2);
        string titleSP = str(format("TB2016 Correlation map VXD space points in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU1, -0.5 * uSize1, 0.5 * uSize1,
                                                                 nStripsU2, -0.5 * uSize2, 0.5 * uSize2);
        axisxtitle = str(format("vertical y position, plane %1% [cm]") % iPlane1);
        axisytitle = str(format("vertical y position, plane %1% [cm]") % iPlane2);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else {       // correlations for v
        string name = str(format("h2VXDCorrelationmapV%1%%2%") % iPlane2 % iPlane1);
        string title = str(format("TB2016 Correlation map VXD in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMaps[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsV2, 0, nStripsV2, nStripsV1, 0,
                                                               nStripsV1);
        string axisxtitle = str(format("v position, plane %1% [pitch units]") % iPlane2);
        string axisytitle = str(format("v position, plane %1% [pitch units]") % iPlane1);
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMaps[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2VXDCorrelationmapSPV%1%%2%") % iPlane2 % iPlane1);
        string titleSP = str(format("TB2016 Correlation map VXD space points in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsV2, -0.5 * vSize2, 0.5 * vSize2,
                                                                 nStripsV1, -0.5 * vSize1, 0.5 * vSize1);
        axisxtitle = str(format("horizontal z position, plane %1% [cm]") % iPlane1);
        axisytitle = str(format("horizontal z position, plane %1% [cm]") % iPlane2);
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsSP[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }



  BasicDir->mkdir("DetailHistos_Basic")->cd();

  for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
    //----------------------------------------------------------------
    // Number of fired strips per frame : hFired[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Fired strip counts U
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_FiredU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Fired pixels in U, plane %2%") % (iS + 1) % iPlane);
      m_firedUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of fired u strips");
      m_firedUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // Fired strip counts V
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_FiredV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Fired pixels in V, plane %2%") % (iS + 1) % iPlane);
      m_firedVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of fired v strips");
      m_firedVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    //----------------------------------------------------------------
    // Number of clusters per frame : hClusters[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Number of clusters U
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClustersU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Number of clusters in U, plane %2%") % (iS + 1) % iPlane);
      m_clustersUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of u clusters");
      m_clustersUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // Number of clusters V
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClustersV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Number of clusters in V, plane %2%") % (iS + 1) % iPlane);
      m_clustersVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of v clusters");
      m_clustersVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    //----------------------------------------------------------------
    // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Hitmaps in U
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_HitmapU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Hitmap in U, plane %2%") % (iS + 1) % iPlane);
      int nStrips = getInfo(i).getUCells();
      m_hitMapUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("hits");
    }
    // Hitmaps in V
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_HitmapV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% Hitmap in V, plane %2%") % (iS + 1) % iPlane);
      int nStrips = getInfo(i).getVCells();
      m_hitMapVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("hits");
    }
    //----------------------------------------------------------------
    // Charge of clusters : hClusterCharge[U/V][PlaneNo]
    //----------------------------------------------------------------
    // u charge by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterChargeU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster charge in U, plane %2%") % (iS + 1) % iPlane);
      m_chargeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_chargeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
      m_chargeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // v charge by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterChargeV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster charge in V, plane %2%") % (iS + 1) % iPlane);
      m_chargeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_chargeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
      m_chargeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    //----------------------------------------------------------------
    // Cluster seed charge distribution : hSeedCharge[U/V][PlaneNo]
    //----------------------------------------------------------------
    // u seed by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_SeedU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% seed charge in U, plane %2%") % (iS + 1) % iPlane);
      m_seedUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_seedUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("seed charge of u clusters [ADU]");
      m_seedUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // v seed by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_SeedV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% seed charge in V, plane %2%") % (iS + 1) % iPlane);
      m_seedVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_seedVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("seed charge of v clusters [ADU]");
      m_seedVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    //----------------------------------------------------------------
    // Cluster size distribution : hClusterSize[U/V][PlaneNo]
    //----------------------------------------------------------------
    // u size by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterSizeU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster size in U, plane %2%") % (iS + 1) % iPlane);
      m_sizeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // v size by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterSizeV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster size in V, plane %2%") % (iS + 1) % iPlane);
      m_sizeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("size of v clusters");
      m_sizeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    //----------------------------------------------------------------
    // Cluster time distribution : hClusterTime[U/V][PlaneNo]
    //----------------------------------------------------------------
    // u time by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterTimeU%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster time in U, plane %2%") % (iS + 1) % iPlane);
      m_timeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("time of u clusters [ns]");
      m_timeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
    // v time by plane
    for (int i = 0; i < c_nSVDPlanes; i++) {
      int iPlane = indexToPlane(i);
      string name = str(format("hSVD_Sen%1%_ClusterTimeV%2%") % (iS + 1) % iPlane);
      string title = str(format("TB2016 SVD Sensor %1% cluster time in V, plane %2%") % (iS + 1) % iPlane);
      m_timeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("time of v clusters [ns]");
      m_timeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
  }

  BasicDir->mkdir("GlobalCorrelations")->cd();
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps for VXD in local uv coordinates : m_correlationsHitMaps[U/V][PlaneNo]
  //----------------------------------------------------------------
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps for VXD in for space points : m_correlationsHitMapsSP[U/V][PlaneNo]
  //----------------------------------------------------------------
  // Correlations in U + V, 2D Hitmaps, also for PXD - SVD combination

//  getObject<TFile>(name_SourceTree.Data())
//  getObject<TFile>("TB2016-DQM-VXD-histo.root").mkdir("NoSorting");

//  TB2016-DQM-VXD-histo.root

  for (int i = 0; i < c_nVXDPlanes; i++) {
    int iPlane1 = indexToPlaneVXD(i);
    float vSize1, uSize1;
    int nStripsU1, nStripsV1;
    if (i >= c_nPXDPlanes) {  // SVD
      //vSize1 = getInfo(i - c_nPXDPlanes).getVSize();
      //nStripsU1 = getInfo(i - c_nPXDPlanes).getUCells();
      //uSize1 = getInfo(i - c_nPXDPlanes).getUSize();
      //nStripsV1 = getInfo(i - c_nPXDPlanes).getVCells();
      vSize1 = 32.0;  // cm
      uSize1 = 6.0;   // cm
    } else { // PXD
      //vSize1 = getInfoPXD(i).getVSize();
      //nStripsU1 = getInfoPXD(i).getUCells();
      //uSize1 = getInfoPXD(i).getUSize();
      //nStripsV1 = getInfoPXD(i).getVCells();
      vSize1 = 8.0;  // cm
      uSize1 = 2.0;   // cm
    }
    nStripsV1 = vSize1 * 20;  // step 0.5 mm
    nStripsU1 = uSize1 * 20;  // step 0.5 mm
    for (int j = 0; j < c_nVXDPlanes; j++) {
      int iPlane2 = indexToPlaneVXD(j);
      float vSize2, uSize2;
      int nStripsU2, nStripsV2;
      if (j >= c_nPXDPlanes) {  // SVD
        //vSize2 = getInfo(j - c_nPXDPlanes).getVSize();
        //nStripsV2 = getInfo(j - c_nPXDPlanes).getVCells();
        //uSize2 = getInfo(j - c_nPXDPlanes).getUSize();
        //nStripsU2 = getInfo(j - c_nPXDPlanes).getUCells();
        vSize2 = 32.0;  // cm
        uSize2 = 6.0;   // cm
      } else { // PXD
        //uSize2 = getInfoPXD(j).getUSize();
        //nStripsU2 = getInfoPXD(j).getUCells();
        //vSize2 = getInfoPXD(j).getVSize();
        //nStripsV2 = getInfoPXD(j).getVCells();
        vSize2 = 8.0;  // cm
        uSize2 = 2.0;   // cm
      }
      nStripsU2 = uSize2 * 20;  // step 0.5 mm
      nStripsV2 = vSize2 * 20;  // step 0.5 mm
      if (i == j) {  // hit maps
        string name = str(format("h2GlobVXDHitmapUV%1%") % iPlane2);
        string title = str(format("TB2016 Glob Hitmap VXD in U x V, plane %1%") % iPlane2);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU2, 0, nStripsU2, nStripsV2, 0,
                                                                   nStripsV2);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("u position [pitch units]");
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("v position [pitch units]");
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2GlobVXDHitmapSP%1%") % iPlane2);
        string titleSP = str(format("TB2016 Glob Hitmap VXD in space points, plane %1%") % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU2, -0.5 * uSize2,
            0.5 * uSize2,
            nStripsV2, -0.5 * vSize2, 0.5 * vSize2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("vertical y position [cm]");
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("horizontal z position [cm]");
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else if (i < j) { // correlations for u
        string name = str(format("h2GlobVXDCorrelationmapU%1%%2%") % iPlane1 % iPlane2);
        string title = str(format("TB2016 Glob Correlation map VXD in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsU1, 0, nStripsU1, nStripsU2, 0,
                                                                   nStripsU2);
        string axisxtitle = str(format("u position, plane %1% [pitch units]") % iPlane1);
        string axisytitle = str(format("u position, plane %1% [pitch units]") % iPlane2);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2GlobVXDCorrelationmapSPU%1%%2%") % iPlane1 % iPlane2);
        string titleSP = str(format("TB2016 Glob Correlation map VXD space points in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU1, -0.5 * uSize1,
            0.5 * uSize1,
            nStripsU2, -0.5 * uSize2, 0.5 * uSize2);
        axisxtitle = str(format("vertical y position, plane %1% [cm]") % iPlane1);
        axisytitle = str(format("vertical y position, plane %1% [cm]") % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      } else {       // correlations for v
        string name = str(format("h2GlobVXDCorrelationmapV%1%%2%") % iPlane2 % iPlane1);
        string title = str(format("TB2016 Glob Correlation map VXD in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i] = new TH2F(name.c_str(), title.c_str(), nStripsV2, 0, nStripsV2, nStripsV1, 0,
                                                                   nStripsV1);
        string axisxtitle = str(format("v position, plane %1% [pitch units]") % iPlane2);
        string axisytitle = str(format("v position, plane %1% [pitch units]") % iPlane1);
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
        string nameSP = str(format("h2GlobVXDCorrelationmapSPV%1%%2%") % iPlane2 % iPlane1);
        string titleSP = str(format("TB2016 Glob Correlation map VXD space points in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsV2, -0.5 * vSize2,
            0.5 * vSize2,
            nStripsV1, -0.5 * vSize1, 0.5 * vSize1);
        axisxtitle = str(format("horizontal z position, plane %1% [cm]") % iPlane1);
        axisytitle = str(format("horizontal z position, plane %1% [cm]") % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }


  oldDir->cd();
}


void SVDDQM3Module::initialize()
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

void SVDDQM3Module::beginRun()
{
  //auto geo = VXD::GeoCache::getInstance();
  //VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Reset();
    m_firedV[i]->Reset();
    m_clustersU[i]->Reset();
    m_clustersV[i]->Reset();
    m_hitMapU[i]->Reset();
    m_hitMapV[i]->Reset();
    m_chargeU[i]->Reset();
    m_chargeV[i]->Reset();
    m_seedU[i]->Reset();
    m_seedV[i]->Reset();
    m_sizeU[i]->Reset();
    m_sizeV[i]->Reset();
    m_timeU[i]->Reset();
    m_timeV[i]->Reset();
  }
  for (int i = 0; i < c_nSVDPlanes * c_MaxSensorsInSVDPlane; i++) {
    m_firedUSen[i]->Reset();
    m_firedVSen[i]->Reset();
    m_clustersUSen[i]->Reset();
    m_clustersVSen[i]->Reset();
    m_hitMapUSen[i]->Reset();
    m_hitMapVSen[i]->Reset();
    m_chargeUSen[i]->Reset();
    m_chargeVSen[i]->Reset();
    m_seedUSen[i]->Reset();
    m_seedVSen[i]->Reset();
    m_sizeUSen[i]->Reset();
    m_sizeVSen[i]->Reset();
    m_timeUSen[i]->Reset();
    m_timeVSen[i]->Reset();
  }
  for (int i = 0; i < c_nVXDPlanes * c_nVXDPlanes; i++) {
    m_correlationsHitMaps[i]->Reset();
    m_correlationsHitMapsSP[i]->Reset();
    m_correlationsHitMapsGlob[i]->Reset();
    m_correlationsHitMapsSPGlob[i]->Reset();
  }
}


void SVDDQM3Module::event()
{

  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);

  const RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);

  // Fired strips
  vector< set<int> > uStrips(c_nSVDPlanes); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDPlanes);
  vector< set<int> > uStripsSen(c_nSVDPlanes * c_MaxSensorsInSVDPlane); // sets to eliminate multiple samples per strip
  vector< set<int> > vStripsSen(c_nSVDPlanes * c_MaxSensorsInSVDPlane);
  for (const SVDDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
//    printf("---> iPlane %i - L %i Sen %i Seg %i\n",iPlane, digit.getSensorID().getLayerNumber(), digit.getSensorID().getSensorNumber(), digit.getSensorID().getSegmentNumber());
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    int indexSen = (digit.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (digit.isUStrip()) {
      uStrips.at(index).insert(digit.getCellID());
      uStripsSen.at(indexSen).insert(digit.getCellID());
    } else {
      vStrips.at(index).insert(digit.getCellID());
      vStripsSen.at(indexSen).insert(digit.getCellID());
    }
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_firedU[i]->Fill(0.1667 * uStrips[i].size());
    m_firedV[i]->Fill(0.1667 * vStrips[i].size());
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      m_firedUSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * uStripsSen[i].size());
      m_firedVSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * vStripsSen[i].size());
    }
  }

  // Cluster counts - same as strips
  int countsU[c_nSVDPlanes];
  int countsV[c_nSVDPlanes];
  int countsUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];
  int countsVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];
  for (int i = 0; i < c_nSVDPlanes; i++) {
    countsU[i] = 0; countsV[i] = 0;
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      countsUSen[iS * c_nSVDPlanes + i] = 0; countsVSen[iS * c_nSVDPlanes + i] = 0;
    }
  }
  for (const SVDCluster& cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (cluster.isUCluster()) {
      countsU[index]++;
      countsUSen[indexSen]++;
    } else {
      countsV[index]++;
      countsVSen[indexSen]++;
    }
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    m_clustersU[i]->Fill(countsU[i]);
    m_clustersV[i]->Fill(countsV[i]);
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      m_clustersUSen[iS * c_nSVDPlanes + i]->Fill(countsUSen[iS * c_nSVDPlanes + i]);
      m_clustersVSen[iS * c_nSVDPlanes + i]->Fill(countsVSen[iS * c_nSVDPlanes + i]);
    }
  }

  // Hitmaps, Charge, Seed, Size, Time
  for (const SVDCluster& cluster : storeClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane);
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (cluster.isUCluster()) {
      m_hitMapU[index]->Fill(getInfo(index).getUCellID(cluster.getPosition()));
      m_chargeU[index]->Fill(cluster.getCharge());
      m_seedU[index]->Fill(cluster.getSeedCharge());
      m_sizeU[index]->Fill(cluster.getSize());
      m_timeU[index]->Fill(cluster.getClsTime());
    } else {
      m_hitMapV[index]->Fill(getInfo(index).getVCellID(cluster.getPosition()));
      m_chargeV[index]->Fill(cluster.getCharge());
      m_seedV[index]->Fill(cluster.getSeedCharge());
      m_sizeV[index]->Fill(cluster.getSize());
      m_timeV[index]->Fill(cluster.getClsTime());
    }
    if (cluster.isUCluster()) {
      m_hitMapUSen[indexSen]->Fill(getInfo(index).getUCellID(cluster.getPosition()));
      m_chargeUSen[indexSen]->Fill(cluster.getCharge());
      m_seedUSen[indexSen]->Fill(cluster.getSeedCharge());
      m_sizeUSen[indexSen]->Fill(cluster.getSize());
      m_timeUSen[indexSen]->Fill(cluster.getClsTime());
    } else {
      m_hitMapVSen[indexSen]->Fill(getInfo(index).getVCellID(cluster.getPosition()));
      m_chargeVSen[indexSen]->Fill(cluster.getCharge());
      m_seedVSen[indexSen]->Fill(cluster.getSeedCharge());
      m_sizeVSen[indexSen]->Fill(cluster.getSize());
      m_timeVSen[indexSen]->Fill(cluster.getClsTime());
    }
  }


  // Correlations for local u v coordinates
  // Threshold for acceptance hit signal to correlation, for every plane, every direction
  // Threshold for acceptance hit time distance to correlation, common for all SVD, not PXD
  // Finaly should be read from external xml cfg file for test beam
  float CutDQMCorrelSigU[c_nVXDPlanes];
  float CutDQMCorrelSigV[c_nVXDPlanes];
//  float CutDQMCorrelSigUSen[c_nVXDPlanes * c_MaxSensorsInSVDPlane];
//  float CutDQMCorrelSigVSen[c_nVXDPlanes * c_MaxSensorsInSVDPlane];
  float CutDQMCorrelTime;
  for (int i = 0; i < c_nPXDPlanes; i++) {  // PXD
    CutDQMCorrelSigU[i] = 0;
    CutDQMCorrelSigV[i] = 0;
//    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
//      CutDQMCorrelSigUSen[iS * c_nPXDPlanes + i] = 0;
//      CutDQMCorrelSigVSen[iS * c_nPXDPlanes + i] = 0;
//    }
  }
  for (int i = c_nPXDPlanes; i < c_nVXDPlanes; i++) {  // SVD
    CutDQMCorrelSigU[i] = 0;
    CutDQMCorrelSigV[i] = 0;
//    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
//      CutDQMCorrelSigUSen[iS * c_nSVDPlanes + i] = 0;
//      CutDQMCorrelSigVSen[iS * c_nSVDPlanes + i] = 0;
//    }
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
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      float fCharge1 = clusterPXD1.getCharge();
      if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
      iIsPXD1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoPXD(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoPXD(index1).getVCellID(clusterPXD1.getV());
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
        fPosU1 = getInfo(index1 - c_nPXDPlanes).getUCellID(cluster1.getPosition());
      } else {
        if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
        iIsV1 = 1;
        fPosV1 = getInfo(index1 - c_nPXDPlanes).getVCellID(cluster1.getPosition());
      }
    }
    // hit maps for PXD:
    if ((iIsU1 == 1) && (iIsV1 == 1))
      m_correlationsHitMaps[c_nVXDPlanes * index1 + index1]->Fill(fPosU1, fPosV1);
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
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        float fCharge2 = clusterPXD2.getCharge();
        if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
        iIsPXD2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoPXD(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoPXD(index2).getVCellID(clusterPXD2.getV());
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
          fPosU2 = getInfo(index2 - c_nPXDPlanes).getUCellID(cluster2.getPosition());
        } else {
          if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
          iIsV2 = 1;
          fPosV2 = getInfo(index2 - c_nPXDPlanes).getVCellID(cluster2.getPosition());
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0))
        if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      // ready to fill correlation histograms:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0)) {
        // hit maps for SVD:
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

  // Correlations for space point coordinates
  for (int i1 = 0; i1 < storeClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosSPU1 = 0.0;
    float fPosSPV1 = 0.0;
    //float fPosSPU1Glo = 0.0;
    //float fPosSPV1Glo = 0.0;
    int iIsPXD1 = 0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      const PXDCluster& clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstPXDPlane) || (iPlane1 > c_lastPXDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      float fCharge1 = clusterPXD1.getCharge();
      if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
      VxdID sensorID1 = clusterPXD1.getSensorID();
      auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      TVector3 rLocal1(clusterPXD1.getU(), clusterPXD1.getV(), 0);
      TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
      iIsPXD1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosSPU1 = rGlobal1.Y();
      fPosSPV1 = rGlobal1.Z();
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 < c_firstSVDPlane) || (iPlane1 > c_lastSVDPlane)) continue;
      index1 = planeToIndexVXD(iPlane1);
      float fCharge1 = cluster1.getCharge();
      fTime1 = cluster1.getClsTime();
      VxdID sensorID1 = cluster1.getSensorID();
      auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      if (cluster1.isUCluster()) {
        if (fCharge1 < CutDQMCorrelSigU[index1]) continue;
        TVector3 rLocal1(cluster1.getPosition(), 0 , 0);
        TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
        iIsU1 = 1;
        fPosSPU1 = rGlobal1.Y();
      } else {
        if (fCharge1 < CutDQMCorrelSigV[index1]) continue;
        TVector3 rLocal1(0, cluster1.getPosition(), 0);
        TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
        iIsV1 = 1;
        fPosSPV1 = rGlobal1.Z();
      }
    }
    // hit maps for PXD:
    if ((iIsU1 == 1) && (iIsV1 == 1)) {
      m_correlationsHitMapsSP[c_nVXDPlanes * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
      m_correlationsHitMapsSPGlob[c_nVXDPlanes * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
    }
    for (int i2 = 0; i2 < storeClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosSPU2 = 0.0;
      float fPosSPV2 = 0.0;
      //float fPosSPU2Glo = 0.0;
      //float fPosSPV2Glo = 0.0;
      int iIsPXD2 = 0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        const PXDCluster& clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstPXDPlane) || (iPlane2 > c_lastPXDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        float fCharge2 = clusterPXD2.getCharge();
        if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
        VxdID sensorID2 = clusterPXD2.getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        TVector3 rLocal2(clusterPXD2.getU(), clusterPXD2.getV(), 0);
        TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
        iIsPXD2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosSPU2 = rGlobal2.Y();
        fPosSPV2 = rGlobal2.Z();
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 < c_firstSVDPlane) || (iPlane2 > c_lastSVDPlane)) continue;
        index2 = planeToIndexVXD(iPlane2);
        float fCharge2 = cluster2.getCharge();
        fTime2 = cluster2.getClsTime();
        VxdID sensorID2 = cluster2.getSensorID();
        auto info = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        if (cluster2.isUCluster()) {
          if (fCharge2 < CutDQMCorrelSigU[index2]) continue;
          TVector3 rLocal2(cluster2.getPosition(), 0 , 0);
          TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
          iIsU2 = 1;
          fPosSPU2 = rGlobal2.Y();
        } else {
          if (fCharge2 < CutDQMCorrelSigV[index2]) continue;
          TVector3 rLocal2(0, cluster2.getPosition(), 0);
          TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
          iIsV2 = 1;
          fPosSPV2 = rGlobal2.Z();
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0))
        if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0)) {
        // hit maps for SVD:
        m_correlationsHitMapsSP[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsHitMapsSP[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsHitMapsSP[c_nVXDPlanes * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
      }
    }
  }

}


void SVDDQM3Module::endRun()
{
}


void SVDDQM3Module::terminate()
{
}
