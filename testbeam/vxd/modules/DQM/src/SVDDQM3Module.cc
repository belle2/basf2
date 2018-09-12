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

#include <pxd/dataobjects/PXDDigit.h>
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
  setDescription("VXD on-line DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("vxd"));
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

  // 1D histograms:
  TDirectory* DirVXDGlobCorrels1DNeigh = NULL;
  TDirectory* DirVXDGlobHitmaps1D = NULL;
  TDirectory* DirVXDGlobCorrels1D = NULL;
  if (!m_Reduce1DCorrelHistos) {
    DirVXDGlobCorrels1DNeigh = oldDir->mkdir("VXD_Global_1DCorrelationsNeighboar");
    DirVXDGlobHitmaps1D = oldDir->mkdir("VXD_Global_1DHitmaps");
    if (m_SaveOtherHistos) {
      DirVXDGlobCorrels1D = oldDir->mkdir("VXD_Global_1DCorrelationsOthers");
    }
  }

  // 2D histograms in global coordinates:
  TDirectory* DirVXDGlobCorrelsNeigh = oldDir->mkdir("VXD_Global_CorrelationsNeighboar");
  TDirectory* DirVXDGlobHitmaps = oldDir->mkdir("VXD_Global_Hitmaps");
  TDirectory* DirVXDGlobCorrels = NULL;
  if (m_SaveOtherHistos) {
    DirVXDGlobCorrels = oldDir->mkdir("VXD_Global_CorrelationsOthers");
  }

  // PXD sensor status 1D+2D histograms
  TDirectory* DirPXDDetailBasic = oldDir->mkdir("PXD_Detail_Basic");

  // SVD sensor status 1D histograms
  TDirectory* DirSVDDetailBasic = oldDir->mkdir("SVD_Detail_Basic");

  DirPXDDetailBasic->cd();

  for (int i = 0; i < c_nPXDPlanes * c_MaxSensorsInPXDPlane; i++) {
    m_firedPxdSen[i] = NULL;
    m_clustersPxdSen[i] = NULL;
    m_hitMapPxdSen[i] = NULL;
    m_chargePxdSen[i] = NULL;
    m_seedPxdSen[i] = NULL;
    m_sizePxdSen[i] = NULL;
    m_sizeUPxdSen[i] = NULL;
    m_sizeVPxdSen[i] = NULL;
  }

  for (int i = 0; i < c_nPXDPlanes; i++) {
    int iPlane = indexToPlanePXD(i);
    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
      //----------------------------------------------------------------
      // Number of fired pixels per frame : hFired[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Fired pixel counts
      if (iS >= getSensorsInLayer(c_firstPXDPlane + i)) continue;
      string name = str(format("hPXD_L%1%_S%2%_Fired") % iPlane % (iS + 1));
      string title = str(format("TB2016 PXD Sensor %1% Fired pixels, plane %2%") % (iS + 1) % iPlane);
      m_firedPxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("# of fired pixels");
      m_firedPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Number of clusters per frame : hClusters[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Number of clusters
      name = str(format("hPXD_L%2%_S%1%_Clusters") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% Number of clusters, plane %2%") % (iS + 1) % iPlane);
      m_clustersPxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("# of clusters");
      m_clustersPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Hitmaps
      name = str(format("hPXD_L%2%_S%1%_Hitmap") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% Hitmap, plane %2%") % (iS + 1) % iPlane);
      int nStripsU = getInfoPXD(i, 1 + 1).getUCells();  // TODO - very special for TB2016
      int nStripsV = getInfoPXD(i, 1 + 1).getVCells();  // TODO - very special for TB2016
      m_hitMapPxdSen[iS * c_nPXDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
      m_hitMapPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("v position [pitch units]");
      m_hitMapPxdSen[iS * c_nPXDPlanes + i]->GetZaxis()->SetTitle("hits");
      //----------------------------------------------------------------
      // Charge of clusters : hClusterCharge[U/V][PlaneNo]
      //----------------------------------------------------------------
      // charge by plane
      name = str(format("hPXD_L%2%_S%1%_ClusterCharge") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% cluster charge, plane %2%") % (iS + 1) % iPlane);
      m_chargePxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_chargePxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
      m_chargePxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster seed charge distribution : hSeedCharge[U/V][PlaneNo]
      //----------------------------------------------------------------
      // seed by plane
      name = str(format("hPXD_L%2%_S%1%_Seed") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% seed charge, plane %2%") % (iS + 1) % iPlane);
      m_seedPxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_seedPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("seed charge of clusters [ADU]");
      m_seedPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster size distribution : hClusterSize[U/V][PlaneNo]
      //----------------------------------------------------------------
      // cluster size by plane
      name = str(format("hPXD_L%2%_S%1%_ClusterSize") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% cluster size, plane %2%") % (iS + 1) % iPlane);
      m_sizePxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizePxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizePxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      // u size by plane
      name = str(format("hPXD_L%2%_S%1%_ClusterSizeU") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% cluster size in U, plane %2%") % (iS + 1) % iPlane);
      m_sizeUPxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeUPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizeUPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
      // v size by plane
      name = str(format("hPXD_L%2%_S%1%_ClusterSizeV") % (iS + 1) % iPlane);
      title = str(format("TB2016 PXD Sensor %1% cluster size in V, plane %2%") % (iS + 1) % iPlane);
      m_sizeVPxdSen[iS * c_nPXDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeVPxdSen[iS * c_nPXDPlanes + i]->GetXaxis()->SetTitle("size of v clusters");
      m_sizeVPxdSen[iS * c_nPXDPlanes + i]->GetYaxis()->SetTitle("count");
    }
  }

  DirSVDDetailBasic->cd();

  for (int i = 0; i < c_nSVDPlanes * c_MaxSensorsInSVDPlane; i++) {
    m_firedUSen[i] = NULL;
    m_firedVSen[i] = NULL;
    m_clustersUSen[i] = NULL;
    m_clustersVSen[i] = NULL;
    m_hitMapUSen[i] = NULL;
    m_hitMapVSen[i] = NULL;
    m_chargeUSen[i] = NULL;
    m_chargeVSen[i] = NULL;
    m_seedUSen[i] = NULL;
    m_seedVSen[i] = NULL;
    m_sizeUSen[i] = NULL;
    m_sizeVSen[i] = NULL;
    m_timeUSen[i] = NULL;
    m_timeVSen[i] = NULL;
  }

  for (int i = 0; i < c_nSVDPlanes; i++) {
    int iPlane = indexToPlane(i);
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      //----------------------------------------------------------------
      // Number of fired strips per frame : hFired[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Fired strip counts U
      if (iS >= getSensorsInLayer(c_firstSVDPlane + i)) continue;
      string name = str(format("hSVD_L%1%_S%2%_FiredU") % iPlane % (iS + 1));
      string title = str(format("TB2016 SVD Sensor %1% Fired pixels in U, plane %2%") % (iS + 1) % iPlane);
      m_firedUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of fired u strips");
      m_firedUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // Fired strip counts V
      name = str(format("hSVD_L%2%_S%1%_FiredV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Fired pixels in V, plane %2%") % (iS + 1) % iPlane);
      m_firedVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      m_firedVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of fired v strips");
      m_firedVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Number of clusters per frame : hClusters[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Number of clusters U
      name = str(format("hSVD_L%2%_S%1%_ClustersU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Number of clusters in U, plane %2%") % (iS + 1) % iPlane);
      m_clustersUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of u clusters");
      m_clustersUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // Number of clusters V
      name = str(format("hSVD_L%2%_S%1%_ClustersV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Number of clusters in V, plane %2%") % (iS + 1) % iPlane);
      m_clustersVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("# of v clusters");
      m_clustersVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
      //----------------------------------------------------------------
      // Hitmaps in U
      name = str(format("hSVD_L%2%_S%1%_HitmapU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Hitmap in U, plane %2%") % (iS + 1) % iPlane);
      int nStrips = getInfo(i, iS + 1).getUCells();
      m_hitMapUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("hSVD_L%2%_S%1%_HitmapV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Hitmap in V, plane %2%") % (iS + 1) % iPlane);
      nStrips = getInfo(i, iS + 1).getVCells();
      m_hitMapVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("hits");
      //----------------------------------------------------------------
      // Charge of clusters : hClusterCharge[U/V][PlaneNo]
      //----------------------------------------------------------------
      // u charge by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterChargeU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster charge in U, plane %2%") % (iS + 1) % iPlane);
      m_chargeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_chargeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
      m_chargeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // v charge by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterChargeV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster charge in V, plane %2%") % (iS + 1) % iPlane);
      m_chargeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_chargeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
      m_chargeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster seed charge distribution : hSeedCharge[U/V][PlaneNo]
      //----------------------------------------------------------------
      // u seed by plane
      name = str(format("hSVD_L%2%_S%1%_SeedU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% seed charge in U, plane %2%") % (iS + 1) % iPlane);
      m_seedUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_seedUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("seed charge of u clusters [ADU]");
      m_seedUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // v seed by plane
      name = str(format("hSVD_L%2%_S%1%_SeedV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% seed charge in V, plane %2%") % (iS + 1) % iPlane);
      m_seedVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
      m_seedVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("seed charge of v clusters [ADU]");
      m_seedVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster size distribution : hClusterSize[U/V][PlaneNo]
      //----------------------------------------------------------------
      // u size by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterSizeU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster size in U, plane %2%") % (iS + 1) % iPlane);
      m_sizeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // v size by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterSizeV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster size in V, plane %2%") % (iS + 1) % iPlane);
      m_sizeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("size of v clusters");
      m_sizeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster time distribution : hClusterTime[U/V][PlaneNo]
      //----------------------------------------------------------------
      // u time by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterTimeU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster time in U, plane %2%") % (iS + 1) % iPlane);
      m_timeUSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("time of u clusters [ns]");
      m_timeUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
      // v time by plane
      name = str(format("hSVD_L%2%_S%1%_ClusterTimeV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% cluster time in V, plane %2%") % (iS + 1) % iPlane);
      m_timeVSen[iS * c_nSVDPlanes + i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("time of v clusters [ns]");
      m_timeVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("count");
    }
  }

  for (int i = 0; i < c_nVXDPlanes; i++) {
    for (int j = 0; j < c_nVXDPlanes; j++) {
      m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = NULL;
      m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i] = NULL;
      m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i] = NULL;
    }
  }

  for (int i = 0; i < c_nVXDPlanes; i++) {
    int iPlane1 = indexToPlaneVXD(i);
    float vSize1, uSize1;
    int nStripsU1, nStripsV1;
    if (i >= c_nPXDPlanes) {  // SVD
      vSize1 = 32.0;  // cm
      uSize1 = 12.0;   // cm
    } else { // PXD
      vSize1 = 16.0;  // cm
      uSize1 = 8.0;   // cm
    }
    nStripsV1 = vSize1 * 10;  // step 1 mm
    nStripsU1 = uSize1 * 10;  // step 1 mm
    for (int j = 0; j < c_nVXDPlanes; j++) {
      int iPlane2 = indexToPlaneVXD(j);
      float vSize2, uSize2;
      int nStripsU2, nStripsV2;
      if (j >= c_nPXDPlanes) {  // SVD
        vSize2 = 32.0;  // cm
        uSize2 = 12.0;   // cm
      } else { // PXD
        vSize2 = 16.0;  // cm
        uSize2 = 8.0;   // cm
      }
      nStripsU2 = uSize2 * 10;  // step 1 mm
      nStripsV2 = vSize2 * 10;  // step 1 mm
      if (i == j) {  // hit maps
        DirVXDGlobHitmaps->cd();
        string nameSP = str(format("h2GlobVXD_L%1%_HitmapSP") % iPlane2);
        string titleSP = str(format("TB2016 Glob Hitmap VXD in space points, plane %1%") % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU2, -0.5 * uSize2,
            0.5 * uSize2,
            nStripsV2, -0.5 * vSize2, 0.5 * vSize2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("vertical y position [cm]");
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("horizontal z position [cm]");
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");

        if (!m_Reduce1DCorrelHistos) {
          DirVXDGlobHitmaps1D->cd();
          nameSP = str(format("h1GlobVXD_L%1%_y_HitmapSP") % iPlane2);
          titleSP = str(format("TB2016 Glob Hitmap in y VXD in space points, plane %1%") % iPlane2);
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(), nStripsU2 * 2, -1.0 * uSize2,
              1.0 * uSize2);
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("vertical y position [cm]");
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("hits");
          nameSP = str(format("h1GlobVXD_L%1%_z_HitmapSP") % iPlane2);
          titleSP = str(format("TB2016 Glob Hitmap in z VXD in space points, plane %1%") % iPlane2);
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(), nStripsV2 * 2, -1.0 * vSize2,
              1.0 * vSize2);
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle("horizontal z position [cm]");
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("hits");
        }
      } else if (i < j) { // correlations for u
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) continue;
        if (abs(i - j) > 1) {
          DirVXDGlobCorrels->cd();
        } else {
          DirVXDGlobCorrelsNeigh->cd();
        }
        string nameSP = str(format("h2GlobVXD_L%1%_L%2%_CorrelationmapSPU") % iPlane1 % iPlane2);
        string titleSP = str(format("TB2016 Glob Correlation map VXD space points in U, plane %1%, plane %2%") % iPlane1 % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU1, -0.5 * uSize1,
            0.5 * uSize1,
            nStripsU2, -0.5 * uSize2, 0.5 * uSize2);
        string axisxtitle = str(format("vertical y position, plane %1% [cm]") % iPlane1);
        string axisytitle = str(format("vertical y position, plane %1% [cm]") % iPlane2);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->GetZaxis()->SetTitle("hits");

        if (!m_Reduce1DCorrelHistos) {
          if (abs(i - j) > 1) {
            DirVXDGlobCorrels1D->cd();
          } else {
            DirVXDGlobCorrels1DNeigh->cd();
          }
          nameSP = str(format("h1GlobVXD_L%1%_L%2%_y_CorrelationmapSP") % iPlane1 % iPlane2);
          titleSP = str(format("TB2016 Glob Correlation map VXD space points, difference in Y, plane %1%, plane %2%") % iPlane1 %
                        iPlane2);
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(), nStripsU1 * 2, -1.0 * uSize1,
              1.0 * uSize1);
          axisxtitle = str(format("vertical y position, planes %1% - %2% [cm]") % iPlane1 % iPlane2);
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("hits");
          nameSP = str(format("h1GlobVXD_L%1%_L%2%_z_CorrelationmapSP") % iPlane1 % iPlane2);
          titleSP = str(format("TB2016 Glob Correlation map VXD space points, difference in Z, plane %1%, plane %2%") % iPlane1 % iPlane2);
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i] = new TH1F(nameSP.c_str(), titleSP.c_str(), nStripsV1 * 2, -1.0 * vSize1,
              1.0 * vSize1);
          axisxtitle = str(format("vertical z difference, planes %1% - %2% [cm]") % iPlane1 % iPlane2);
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
          m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i]->GetYaxis()->SetTitle("hits");
        }
      } else {       // correlations for v
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) continue;
        if (abs(i - j) > 1) {
          DirVXDGlobCorrels->cd();
        } else {
          DirVXDGlobCorrelsNeigh->cd();
        }
        DirVXDGlobCorrelsNeigh->cd();
        string nameSP = str(format("h2GlobVXD_L%1%_L%2%_CorrelationmapSPV") % iPlane2 % iPlane1);
        string titleSP = str(format("TB2016 Glob Correlation map VXD space points in V, plane %1%, plane %2%") % iPlane2 % iPlane1);
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsV2, -0.5 * vSize2,
            0.5 * vSize2,
            nStripsV1, -0.5 * vSize1, 0.5 * vSize1);
        string axisxtitle = str(format("horizontal z position, plane %1% [cm]") % iPlane1);
        string axisytitle = str(format("horizontal z position, plane %1% [cm]") % iPlane2);
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

  B2INFO("***********************************************************");
  B2INFO("*                                                         *");
  B2INFO("* THIS MODULE WILL BE NO MORE SUPPORTED BY AUTHOR!        *");
  B2INFO("* PLEASE CHANGE MODULE TO: 'VXDDQMOnLine'                 *");
  B2INFO("*                          SaveOtherHistos=1              *");
  B2INFO("*                                                         *");
  B2INFO("***********************************************************");

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  RelationArray relClusterDigits(storeSVDClusters, storeDigits);

  //Store names to speed up creation later
  m_storePXDClustersName = storePXDClusters.getName();
  m_storePXDDigitsName = storePXDDigits.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
}

void SVDDQM3Module::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
    for (int i = 0; i < c_nSVDPlanes; i++) {
      if (iS >= getSensorsInLayer(c_firstSVDPlane + i)) continue;
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
  }
  for (int i = 0; i < c_nVXDPlanes; i++) {
    for (int j = 0; j < c_nVXDPlanes; j++) {
      if (abs(i - j) > 1) continue;
      m_correlationsHitMapsSPGlob[c_nVXDPlanes * j + i]->Reset();
      if (m_Reduce1DCorrelHistos) continue;
      if (i <= j) m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * j + i]->Reset();
      if (i <= j) m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * j + i]->Reset();
    }
  }
}


void SVDDQM3Module::event()
{

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);

  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const RelationArray relClusterDigits(storeSVDClusters, storeDigits, m_relClusterDigitName);

  // PXD basic histograms:
  // Fired strips
  vector< set<int> > uStripsSenp(c_nPXDPlanes * c_MaxSensorsInPXDPlane); // sets to eliminate multiple samples per strip
  //vector< set<int> > vStripsSenp(c_nPXDPlanes * c_MaxSensorsInPXDPlane);
  for (const PXDDigit& digit : storePXDDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int indexSen = (digit.getSensorID().getSensorNumber() - 1) * c_nPXDPlanes + planeToIndexPXD(iPlane);
    uStripsSenp.at(indexSen).insert(digit.getUCellID());
    //vStripsSen.at(indexSen).insert(digit.getVCellID());
  }
  for (int i = 0; i < c_nPXDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
      if (iS >= getSensorsInLayer(c_firstPXDPlane + i)) continue;
      m_firedPxdSen[iS * c_nPXDPlanes + i]->Fill(0.1667 * uStripsSenp[i].size());
      //m_firedVSen[iS * c_nPXDPlanes + i]->Fill(0.1667 * vStripsSen[i].size());
    }
  }

  int countsSen[c_nPXDPlanes * c_MaxSensorsInPXDPlane];
  for (int i = 0; i < c_nPXDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
      countsSen[iS * c_nPXDPlanes + i] = 0;
    }
  }
  for (const PXDCluster& cluster : storePXDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nPXDPlanes + planeToIndex(iPlane);
    countsSen[indexSen]++;
  }
  for (int i = 0; i < c_nPXDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
      if (iS >= getSensorsInLayer(c_firstPXDPlane + i)) continue;
      m_clustersPxdSen[iS * c_nPXDPlanes + i]->Fill(countsSen[iS * c_nPXDPlanes + i]);
    }
  }

  // Hitmaps, Charge, Seed, Size
  for (const PXDCluster& cluster : storePXDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nPXDPlanes + planeToIndexPXD(iPlane);
    m_hitMapPxdSen[indexSen]->Fill(getInfoPXD(planeToIndexPXD(iPlane),
                                              cluster.getSensorID().getSensorNumber()).getUCellID(cluster.getU()),
                                   getInfoPXD(planeToIndexPXD(iPlane), cluster.getSensorID().getSensorNumber()).getVCellID(cluster.getV())
                                  );
    m_chargePxdSen[indexSen]->Fill(cluster.getCharge());
    m_seedPxdSen[indexSen]->Fill(cluster.getSeedCharge());
    m_sizePxdSen[indexSen]->Fill(cluster.getSize());
    m_sizeUPxdSen[indexSen]->Fill(cluster.getUSize());
    m_sizeVPxdSen[indexSen]->Fill(cluster.getVSize());
  }

  // SVD basic histograms:
  // Fired strips
  vector< set<int> > uStripsSen(c_nSVDPlanes * c_MaxSensorsInSVDPlane); // sets to eliminate multiple samples per strip
  vector< set<int> > vStripsSen(c_nSVDPlanes * c_MaxSensorsInSVDPlane);
  for (const SVDDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int indexSen = (digit.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (digit.isUStrip()) {
      uStripsSen.at(indexSen).insert(digit.getCellID());
    } else {
      vStripsSen.at(indexSen).insert(digit.getCellID());
    }
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      if (iS >= getSensorsInLayer(c_firstSVDPlane + i)) continue;
      m_firedUSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * uStripsSen[i].size());
      m_firedVSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * vStripsSen[i].size());
    }
  }

  int countsUSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];
  int countsVSen[c_nSVDPlanes * c_MaxSensorsInSVDPlane];
  for (int i = 0; i < c_nSVDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      countsUSen[iS * c_nSVDPlanes + i] = 0; countsVSen[iS * c_nSVDPlanes + i] = 0;
    }
  }
  for (const SVDCluster& cluster : storeSVDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (cluster.isUCluster()) {
      countsUSen[indexSen]++;
    } else {
      countsVSen[indexSen]++;
    }
  }
  for (int i = 0; i < c_nSVDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInSVDPlane; iS++) {
      if (iS >= getSensorsInLayer(c_firstSVDPlane + i)) continue;
      m_clustersUSen[iS * c_nSVDPlanes + i]->Fill(countsUSen[iS * c_nSVDPlanes + i]);
      m_clustersVSen[iS * c_nSVDPlanes + i]->Fill(countsVSen[iS * c_nSVDPlanes + i]);
    }
  }

  // Hitmaps, Charge, Seed, Size, Time
  for (const SVDCluster& cluster : storeSVDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int indexSen = (cluster.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (cluster.isUCluster()) {
      m_hitMapUSen[indexSen]->Fill(getInfo(planeToIndex(iPlane),
                                           cluster.getSensorID().getSensorNumber()).getUCellID(cluster.getPosition()));
      m_chargeUSen[indexSen]->Fill(cluster.getCharge());
      m_seedUSen[indexSen]->Fill(cluster.getSeedCharge());
      m_sizeUSen[indexSen]->Fill(cluster.getSize());
      m_timeUSen[indexSen]->Fill(cluster.getClsTime());
    } else {
      m_hitMapVSen[indexSen]->Fill(getInfo(planeToIndex(iPlane),
                                           cluster.getSensorID().getSensorNumber()).getVCellID(cluster.getPosition()));
      m_chargeVSen[indexSen]->Fill(cluster.getCharge());
      m_seedVSen[indexSen]->Fill(cluster.getSeedCharge());
      m_sizeVSen[indexSen]->Fill(cluster.getSize());
      m_timeVSen[indexSen]->Fill(cluster.getClsTime());
    }
  }

  // Correlations for space point coordinates

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
  int SwapPXD = 0;    // TODO Very special case, in simulations we see swap u-v! check with data. April 18,2016
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosSPU1 = 0.0;
    float fPosSPV1 = 0.0;
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
      if (SwapPXD) {
        fPosSPV1 = rGlobal1.Y();
        fPosSPU1 = rGlobal1.Z();
      }
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
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
      m_correlationsHitMapsSPGlob[c_nVXDPlanes * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
      if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * index1 + index1]->Fill(fPosSPU1);
      if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * index1 + index1]->Fill(fPosSPV1);
    }
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosSPU2 = 0.0;
      float fPosSPV2 = 0.0;
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
        if (SwapPXD) {
          fPosSPV2 = rGlobal2.Y();
          fPosSPU2 = rGlobal2.Z();
        }
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storePXDClusters.getEntries()];
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
      if (abs(index1 - index2) > 1) continue;  // only neighboars
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0)) {
        // hit maps for SVD:
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
        if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1);
        if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * index2 + index1]->Fill(fPosSPV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
        if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Du[c_nVXDPlanes * index2 + index1]->Fill(fPosSPU2 - fPosSPU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsHitMapsSPGlob[c_nVXDPlanes * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
        if (!m_Reduce1DCorrelHistos) m_correlationsHitMapsSPGlob1Dv[c_nVXDPlanes * index1 + index2]->Fill(fPosSPV2 - fPosSPV1);
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
