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

#include "testbeam/vxd/modules/DQM/VXDTelDQMOffLineModule.h"


using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDTelDQMOffLine)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDTelDQMOffLineModule::VXDTelDQMOffLineModule() : HistoModule()
{
  //Set module properties
  setDescription("VXD on-line DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("vxd"));
  addParam("SaveOtherHistos", m_SaveOtherHistos,
           "flag <0,1> for creation of correlation plots for non-neighboar layers, default = 0 ", m_SaveOtherHistos);
  addParam("SwapPXD", m_SwapPXD, "flag <0,1> very special case for swap of u-v coordinates, default = 0 ", m_SwapPXD);
  addParam("SwapTel", m_SwapTel, "flag <0,1> very special case for swap of u-v coordinates, default = 0 ", m_SwapTel);
  addParam("CorrelationGranulation", m_CorrelationGranulation,
           "set granulation of histogram plots, default is 1 mm, min = 0.002 mm, max = 1 mm ", m_CorrelationGranulation);

}


VXDTelDQMOffLineModule::~VXDTelDQMOffLineModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VXDTelDQMOffLineModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;

  if (m_CorrelationGranulation > 1.0) m_CorrelationGranulation = 1.0;  //  set maximum of gramularity to 1 mm
  if (m_CorrelationGranulation < 0.02) m_CorrelationGranulation = 0.02;  //  set minimum of gramularity to 0.02 mm

  // if binning go over h_MaxBins it decrease preset of range
  int h_MaxBins = 2000;       //maximal size of histogram binning:

  // 2D histograms in global coordinates:
  TDirectory* DirVXDGlobCorrelsNeigh = oldDir->mkdir("TB_CorrelationsNeighboar");
  TDirectory* DirVXDGlobHitmaps = oldDir->mkdir("TB_Hitmaps");
  TDirectory* DirVXDGlobCorrels = NULL;
  if (m_SaveOtherHistos) {
    DirVXDGlobCorrels = oldDir->mkdir("TB_CorrelationsOthers");
  }

  // PXD sensor status 1D+2D histograms
  TDirectory* DirPXDDetailBasic = oldDir->mkdir("PXD_Basic");

  // SVD sensor status 1D histograms
  TDirectory* DirSVDDetailBasic = oldDir->mkdir("SVD_Basic");

  // PXD sensor status 1D+2D histograms
  TDirectory* DirTelDetailBasic = oldDir->mkdir("Tel_Basic");

  DirTelDetailBasic->cd();


  for (int i = 0; i < c_nTelPlanes; i++) {
    m_firedTelSen[i] = NULL;
    m_clustersTelSen[i] = NULL;
    m_hitMapTelSen[i] = NULL;
    m_sizeTelSen[i] = NULL;
  }

  for (int i = 0; i < c_nTelPlanes; i++) {
    int iPlane = indexToPlaneTel(i);
    //----------------------------------------------------------------
    // Number of fired pixels per frame : hFired[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Fired pixel counts
    string name = str(format("hTel_L%1%_Fired") % iPlane);
    string title = str(format("TB2016 Tel Fired pixels, plane %1%") % iPlane);
    m_firedTelSen[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedTelSen[i]->GetXaxis()->SetTitle("# of fired pixels");
    m_firedTelSen[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Number of clusters per frame : hClusters[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Number of clusters
    name = str(format("hTel_L%1%_Clusters") % iPlane);
    title = str(format("TB2016 Tel Number of clusters, plane %1%") % iPlane);
    m_clustersTelSen[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersTelSen[i]->GetXaxis()->SetTitle("# of clusters");
    m_clustersTelSen[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Hitmaps: Number of clusters by coordinate : hHitmap[U/V][PlaneNo]
    //----------------------------------------------------------------
    // Hitmaps
    name = str(format("hTel_L%1%_Hitmap") % iPlane);
    title = str(format("TB2016 Tel Hitmap, plane %1%") % iPlane);
    int nStripsU = getInfoTel(i).getUCells();
    int nStripsV = getInfoTel(i).getVCells();
    m_hitMapTelSen[i] = new TH2F(name.c_str(), title.c_str(), nStripsU, 0, nStripsU, nStripsV, 0, nStripsV);
    m_hitMapTelSen[i]->GetXaxis()->SetTitle("u position [pitch units]");
    m_hitMapTelSen[i]->GetYaxis()->SetTitle("v position [pitch units]");
    m_hitMapTelSen[i]->GetZaxis()->SetTitle("hits");
    //----------------------------------------------------------------
    // Cluster size distribution : hClusterSize[U/V][PlaneNo]
    //----------------------------------------------------------------
    // cluster size by plane
    name = str(format("hTel_L%1%_ClusterSize") % iPlane);
    title = str(format("TB2016 Tel cluster size, plane %1%") % iPlane);
    m_sizeTelSen[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_sizeTelSen[i]->GetXaxis()->SetTitle("size of u clusters");
    m_sizeTelSen[i]->GetYaxis()->SetTitle("count");
  }

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
    m_hitMapDigitUSen[i] = NULL;
    m_hitMapDigitVSen[i] = NULL;
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
      // Hitmaps in U
      name = str(format("hSVD_L%2%_S%1%_HitmapDigitsU") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Hitmap of digits in U, plane %2%") % (iS + 1) % iPlane);
      nStrips = getInfo(i, iS + 1).getUCells();
      m_hitMapDigitUSen[iS * c_nSVDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, 6, 0, 6);
      m_hitMapDigitUSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapDigitUSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("timestamp");
      m_hitMapDigitUSen[iS * c_nSVDPlanes + i]->GetZaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("hSVD_L%2%_S%1%_HitmapDigitsV") % (iS + 1) % iPlane);
      title = str(format("TB2016 SVD Sensor %1% Hitmap of digits in V, plane %2%") % (iS + 1) % iPlane);
      nStrips = getInfo(i, iS + 1).getVCells();
      m_hitMapDigitVSen[iS * c_nSVDPlanes + i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, 6, 0, 6);
      m_hitMapDigitVSen[iS * c_nSVDPlanes + i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapDigitVSen[iS * c_nSVDPlanes + i]->GetYaxis()->SetTitle("timestamp");
      m_hitMapDigitVSen[iS * c_nSVDPlanes + i]->GetZaxis()->SetTitle("hits");
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

  for (int i = 0; i < c_nTBPlanes; i++) {
    for (int j = 0; j < c_nTBPlanes; j++) {
      m_correlations[c_nTBPlanes * j + i] = NULL;
    }
  }

  for (int iL = 0; iL < c_nTBPlanes; iL++) {
    int i = iL;
//    if (iL >= c_nVXDPlanes) i = c_nTBPlanes + c_nVXDPlanes - iL - c_firstTBPlane;  // reorder of telescopes
    int iPlane1 = indexToPlaneTB(i);
    float vSize1, uSize1;
    int nStripsU1, nStripsV1;
    if (i >= c_nVXDPlanes) {  // Tel
      vSize1 = 32.0;  // cm
      uSize1 = 12.0;   // cm
    } else if (i >= c_nPXDPlanes) { // SVD
      vSize1 = 32.0;  // cm
      uSize1 = 12.0;   // cm
    } else { // PXD
      vSize1 = 16.0;  // cm
      uSize1 = 8.0;   // cm
    }
    nStripsV1 = vSize1 * 10 / m_CorrelationGranulation;  // step 1/m_CorrelationGranulation mm
    nStripsU1 = uSize1 * 10 / m_CorrelationGranulation;  // step 1/m_CorrelationGranulation mm

    if (nStripsV1 > h_MaxBins) {
      vSize1 = h_MaxBins * m_CorrelationGranulation / 10.0;
      nStripsV1 = vSize1 * 10 / m_CorrelationGranulation;
    }
    if (nStripsU1 > h_MaxBins) {
      uSize1 = h_MaxBins * m_CorrelationGranulation / 10.0;
      nStripsU1 = uSize1 * 10 / m_CorrelationGranulation;
    }

    for (int jL = 0; jL < c_nTBPlanes; jL++) {
      int j = jL;
//      if (jL >= c_nVXDPlanes) j = c_nTBPlanes + c_nVXDPlanes - jL - c_firstTBPlane;  // reorder of telescopes
      int iPlane2 = indexToPlaneTB(j);
      float vSize2, uSize2;
      int nStripsU2, nStripsV2;
      int indexOrd1 = iPlane1;
//      if (indexOrd1 > c_nVXDPlanes) indexOrd1 = c_nTelPlanes - (indexOrd1 - c_nVXDPlanes) +
//                                                  c_firstTelPlane;  // reorder telescopes to more native order
      if (indexOrd1 > c_nVXDPlanes) indexOrd1 = indexOrd1 - c_nVXDPlanes;  // reorder telescopes to more native order
      int indexOrd2 = iPlane2;
//      if (indexOrd2 > c_nVXDPlanes) indexOrd2 = c_nTelPlanes - (indexOrd2 - c_nVXDPlanes) +
//                                                  c_firstTelPlane;  // reorder telescopes to more native order
      if (indexOrd2 > c_nVXDPlanes) indexOrd2 = indexOrd2 - c_nVXDPlanes;  // reorder telescopes to more native order
      if (j >= c_nVXDPlanes) {  // Tel
        vSize2 = 32.0;  // cm
        uSize2 = 12.0;   // cm
      } else if (j >= c_nPXDPlanes) { // SVD
        vSize2 = 32.0;  // cm
        uSize2 = 12.0;   // cm
      } else { // PXD
        vSize2 = 16.0;  // cm
        uSize2 = 8.0;   // cm
      }
      //nStripsU2 = uSize2 * 10;  // step 1 mm
      //nStripsV2 = vSize2 * 10;  // step 1 mm
      nStripsV2 = vSize2 * 10 / m_CorrelationGranulation;  // step 1/m_CorrelationGranulation mm
      nStripsU2 = uSize2 * 10 / m_CorrelationGranulation;  // step 1/m_CorrelationGranulation mm

      if (nStripsV2 > h_MaxBins) {
        vSize2 = h_MaxBins * m_CorrelationGranulation / 10.0;
        nStripsV2 = vSize2 * 10 / m_CorrelationGranulation;
      }
      if (nStripsU2 > h_MaxBins) {
        uSize2 = h_MaxBins * m_CorrelationGranulation / 10.0;
        nStripsU2 = uSize2 * 10 / m_CorrelationGranulation;
      }
      if (i == j) {  // hit maps
        DirVXDGlobHitmaps->cd();
        string nameSP = str(format("VXD_L%1%_Hitmap") % indexOrd2);
        string titleSP = str(format("TB2016 HitMap VXD plane %1% in space points") % indexOrd2);
        if (i >= c_nVXDPlanes) {
          nameSP = str(format("Tel_L%1%_HitMap") % indexOrd2);
          titleSP = str(format("TB2016 Hitmap Tel %1% in space points") % indexOrd2);
        }
        m_correlations[c_nTBPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU2, -0.5 * uSize2,
                                                       0.5 * uSize2,
                                                       nStripsV2, -0.5 * vSize2, 0.5 * vSize2);
        m_correlations[c_nTBPlanes * j + i]->GetXaxis()->SetTitle("y position [cm]");
        m_correlations[c_nTBPlanes * j + i]->GetYaxis()->SetTitle("z position [cm]");
        m_correlations[c_nTBPlanes * j + i]->GetZaxis()->SetTitle("hits");

      } else if (i < j) { // correlations for u
        int skypOtherHistos = 0;
        if (!m_SaveOtherHistos) if (((i == 5) && (j == 6)) || ((i == 6) && (j == 5)) || ((i == 8) && (j == 9)) || ((i == 9)
                                      && (j == 8))) skypOtherHistos = 1;
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) skypOtherHistos = 1;
        if (!m_SaveOtherHistos) if (((i == 5) && (j == 9)) || ((i == 9) && (j == 5)) || ((i == 0) && (j == 8)) || ((i == 8)
                                      && (j == 0))) skypOtherHistos = 0;
        if (skypOtherHistos) continue;
        if (abs(i - j) > 1) {
          if (m_SaveOtherHistos) DirVXDGlobCorrels->cd();
        } else {
          DirVXDGlobCorrelsNeigh->cd();
        }
        if (m_SaveOtherHistos) if (((i == 5) && (j == 6)) || ((i == 6) && (j == 5))) DirVXDGlobCorrels->cd(); //they are not neighboar
        if (m_SaveOtherHistos) if (((i == 8) && (j == 9)) || ((i == 9) && (j == 8))) DirVXDGlobCorrels->cd(); //they are not neighboar tels
        if (((i == 5) && (j == 9)) || ((i == 9) && (j == 5))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
        if (((i == 0) && (j == 8)) || ((i == 8) && (j == 0))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
//        if (((i == 5) && (j == 8)) || ((i == 8) && (j == 5))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
//        if (((i == 0) && (j == 9)) || ((i == 9) && (j == 0))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar

        string nameSP = str(format("VXD_L%1%_VXD_L%2%_CorrelationMapU") % indexOrd1 % indexOrd2);
        string titleSP = str(format("TB2016 Correlation map VXD %1% VXD %2% space points in U") % indexOrd1 % indexOrd2);
        if ((iPlane1 > c_nVXDPlanes) && (iPlane2 > c_nVXDPlanes)) {
          nameSP = str(format("Tel_L%1%_Tel_L%2%_CorrelationMapU") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map Tel %1% Tel %2% space points in U") % indexOrd1 % indexOrd2);
        } else if ((iPlane1 > c_nVXDPlanes) && (iPlane2 <= c_nVXDPlanes)) {
          nameSP = str(format("Tel_L%1%_VXD_L%2%_CorrelationMapU") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map Tel %1% VXD %2% space points in U") % indexOrd1 % indexOrd2);
        } else if ((iPlane1 <= c_nVXDPlanes) && (iPlane2 > c_nVXDPlanes)) {
          nameSP = str(format("VXD_L%1%_Tel_L%2%_CorrelationMapU") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map VXD %1% Tel %2% space points in U") % indexOrd1 % indexOrd2);
        }
        m_correlations[c_nTBPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsU1, -0.5 * uSize1,
                                                       0.5 * uSize1,
                                                       nStripsU2, -0.5 * uSize2, 0.5 * uSize2);
        string axisxtitle = str(format("y position [cm]"));
        string axisytitle = str(format("y position [cm]"));
        m_correlations[c_nTBPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlations[c_nTBPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlations[c_nTBPlanes * j + i]->GetZaxis()->SetTitle("hits");

      } else {       // correlations for v
        int skypOtherHistos = 0;
        if (!m_SaveOtherHistos) if (((i == 5) && (j == 6)) || ((i == 6) && (j == 5)) || ((i == 8) && (j == 9)) || ((i == 9)
                                      && (j == 8))) skypOtherHistos = 1;
        if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) skypOtherHistos = 1;
        if (!m_SaveOtherHistos) if (((i == 5) && (j == 9)) || ((i == 9) && (j == 5)) || ((i == 0) && (j == 8)) || ((i == 8)
                                      && (j == 0))) skypOtherHistos = 0;
        if (skypOtherHistos) continue;
        //if ((!m_SaveOtherHistos) && (abs(i - j) > 1)) continue;
        if (abs(i - j) > 1) {
          if (m_SaveOtherHistos) DirVXDGlobCorrels->cd();
        } else {
          DirVXDGlobCorrelsNeigh->cd();
        }
        if (m_SaveOtherHistos) if (((i == 5) && (j == 6)) || ((i == 6) && (j == 5))) DirVXDGlobCorrels->cd(); //they are not neighboar
        if (m_SaveOtherHistos) if (((i == 8) && (j == 9)) || ((i == 9) && (j == 8))) DirVXDGlobCorrels->cd(); //they are not neighboar tels
        if (((i == 5) && (j == 9)) || ((i == 9) && (j == 5))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
        if (((i == 0) && (j == 8)) || ((i == 8) && (j == 0))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
//        if (((i == 5) && (j == 8)) || ((i == 8) && (j == 5))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar
//        if (((i == 0) && (j == 9)) || ((i == 9) && (j == 0))) DirVXDGlobCorrelsNeigh->cd();  // VXD-Tel neighboar

        string nameSP = str(format("VXD_L%1%_VXD_L%2%_CorrelationMapV") % indexOrd1 % indexOrd2);
        string titleSP = str(format("TB2016 Correlation map VXD %1% VXD %2% space points in V") % indexOrd1 % indexOrd2);
        if ((iPlane1 > c_nVXDPlanes) && (iPlane2 > c_nVXDPlanes)) {
          nameSP = str(format("Tel_L%1%_Tel_L%2%_CorrelationMapV") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map Tel %1% Tel %2% space points in V") % indexOrd1 % indexOrd2);
        } else if ((iPlane1 > c_nVXDPlanes) && (iPlane2 <= c_nVXDPlanes)) {
          nameSP = str(format("Tel_L%1%_VXD_L%2%_CorrelationMapV") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map Tel %1% VXD %2% space points in V") % indexOrd1 % indexOrd2);
        } else if ((iPlane1 <= c_nVXDPlanes) && (iPlane2 > c_nVXDPlanes)) {
          nameSP = str(format("VXD_L%1%_Tel_L%2%_CorrelationMapV") % indexOrd1 % indexOrd2);
          titleSP = str(format("TB2016 Correlation map VXD %1% Tel %2% space points in V") % indexOrd1 % indexOrd2);
        }
        m_correlations[c_nTBPlanes * j + i] = new TH2F(nameSP.c_str(), titleSP.c_str(), nStripsV2, -0.5 * vSize2,
                                                       0.5 * vSize2,
                                                       nStripsV1, -0.5 * vSize1, 0.5 * vSize1);
        string axisxtitle = str(format("z position [cm]"));
        string axisytitle = str(format("z position [cm]"));
        m_correlations[c_nTBPlanes * j + i]->GetXaxis()->SetTitle(axisxtitle.c_str());
        m_correlations[c_nTBPlanes * j + i]->GetYaxis()->SetTitle(axisytitle.c_str());
        m_correlations[c_nTBPlanes * j + i]->GetZaxis()->SetTitle("hits");
      }
    }
  }

  oldDir->cd();
}


void VXDTelDQMOffLineModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);
  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);

  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  RelationArray relClusterDigits(storeSVDClusters, storeDigits);
  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  //Store names to speed up creation later
  m_storePXDClustersName = storePXDClusters.getName();
  m_storePXDDigitsName = storePXDDigits.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_relClusterDigitName = relClusterDigits.getName();
  m_storeTelClustersName = storeTelClusters.getName();
  m_storeTelDigitsName = storeTelDigits.getName();
  m_relTelClusterDigitName = relTelClusterDigits.getName();
}

void VXDTelDQMOffLineModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  /*
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
    for (int i = 0; i < c_nTBPlanes; i++) {
      for (int j = 0; j < c_nTBPlanes; j++) {
  //      if (abs(i - j) > 1) continue;
        m_correlations[c_nTBPlanes * j + i]->Reset();
      }
    }
  */
}


void VXDTelDQMOffLineModule::event()
{

  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);

  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const RelationArray relClusterDigits(storeSVDClusters, storeDigits, m_relClusterDigitName);
  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);

//  StoreObjPtr<EventMetaData> emd;
//  unsigned int m_evt = emd->getEvent();
//  insignet int m_evt = emd->g
//  emd.getEvent().

  // Tel basic histograms:
  // Fired strips
  vector< set<int> > uStripsSenpt(c_nTelPlanes); // sets to eliminate multiple samples per strip
  //vector< set<int> > vStripsSenp(c_nPXDPlanes * c_MaxSensorsInPXDPlane);
  for (const TelDigit& digit : storeTelDigits) {
    int iPlane = digit.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
    uStripsSenpt.at(iPlane - 1).insert(digit.getUCellID());
    //vStripsSen.at(indexSen).insert(digit.getVCellID());
  }
  for (int i = 0; i < c_nTelPlanes; i++) {
    m_firedTelSen[i]->Fill(0.1667 * uStripsSenpt[i].size());
    //m_firedVSen[iS * c_nPXDPlanes + i]->Fill(0.1667 * vStripsSen[i].size());
  }

  int countsSent[c_nTelPlanes];
  for (int i = 0; i < c_nTelPlanes; i++) {
    countsSent[i] = 0;
  }
  for (const TelCluster& cluster : storeTelClusters) {
    int iPlane = cluster.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
    countsSent[iPlane - 1]++;
  }
  for (int i = 0; i < c_nTelPlanes; i++) {
    m_clustersTelSen[i]->Fill(countsSent[i]);
  }

  // Hitmaps, Size
  for (const TelCluster& cluster : storeTelClusters) {
    int iPlane = cluster.getSensorID().getSensorNumber();
    if ((iPlane < c_firstTelPlane) || (iPlane > c_lastTelPlane)) continue;
    m_hitMapTelSen[iPlane - 1]->Fill(getInfoTel(planeToIndexTel(iPlane)).getUCellID(cluster.getU()),
                                     getInfoTel(planeToIndexTel(iPlane)).getVCellID(cluster.getV())
                                    );
    m_sizeTelSen[iPlane - 1]->Fill(cluster.getSize());
  }

  // PXD basic histograms:
  // Fired strips
  vector< set<int> > uStripsSenp(c_nPXDPlanes * c_MaxSensorsInPXDPlane); // sets to eliminate multiple samples per strip
  //vector< set<int> > vStripsSenp(c_nPXDPlanes * c_MaxSensorsInPXDPlane);
  for (const PXDDigit& digit : storePXDDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstPXDPlane) || (iPlane > c_lastPXDPlane)) continue;
    int indexSen = digit.getSensorID().getSensorNumber() - 1;
    uStripsSenp.at(indexSen * c_nPXDPlanes + iPlane - 1).insert(digit.getUCellID());
    //vStripsSen.at(indexSen).insert(digit.getVCellID());
  }
  for (int i = 0; i < c_nPXDPlanes; i++) {
    for (int iS = 0; iS < c_MaxSensorsInPXDPlane; iS++) {
      if (iS >= getSensorsInLayer(c_firstPXDPlane + i)) continue;
      m_firedPxdSen[iS * c_nPXDPlanes + i]->Fill(0.1667 * uStripsSenp[iS * c_nPXDPlanes + i].size());
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
      m_firedUSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * uStripsSen[iS * c_nSVDPlanes + i].size());
      m_firedVSen[iS * c_nSVDPlanes + i]->Fill(0.1667 * vStripsSen[iS * c_nSVDPlanes + i].size());
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
  float m_CutSVDCharge = 22;
  for (const SVDDigit& digit : storeDigits) {
    int iPlane = digit.getSensorID().getLayerNumber();
    if ((iPlane < c_firstSVDPlane) || (iPlane > c_lastSVDPlane)) continue;
    int indexSen = (digit.getSensorID().getSensorNumber() - 1) * c_nSVDPlanes + planeToIndex(iPlane);
    if (digit.getCharge() > m_CutSVDCharge) {
      if (digit.isUStrip()) {
        m_hitMapDigitUSen[indexSen]->Fill(digit.getCellID(), digit.getIndex());
      } else {
        m_hitMapDigitVSen[indexSen]->Fill(digit.getCellID(), digit.getIndex());
      }
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
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storePXDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fTime1 = 0.0;
    float fPosSPU1 = 0.0;
    float fPosSPV1 = 0.0;
    int iIsPXD1 = 0;
    int iIsTel1 = 0;
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
      if (m_SwapPXD) {
        fPosSPV1 = rGlobal1.Y();
        fPosSPU1 = rGlobal1.Z();
      }
    } else if ((i1 >= storePXDClusters.getEntries())
               && (i1 < (storePXDClusters.getEntries() + storeSVDClusters.getEntries()))) {                                  // SVD clusters:
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
    } else {                                  // Tel clusters:
      const TelCluster& clusterTel1 = *storeTelClusters[ i1 - (storePXDClusters.getEntries() + storeSVDClusters.getEntries())];
      iPlane1 = clusterTel1.getSensorID().getSensorNumber();
      if ((iPlane1 < c_firstTelPlane) || (iPlane1 > c_lastTelPlane)) continue;
      index1 = c_nVXDPlanes + iPlane1 - c_firstTelPlane;
      VxdID sensorID1 = clusterTel1.getSensorID();
      auto info = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID1));
      TVector3 rLocal1(clusterTel1.getU(), clusterTel1.getV(), 0);
      TVector3 rGlobal1 = info.pointToGlobal(rLocal1);
      iIsTel1 = 1;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosSPU1 = rGlobal1.Y();
      fPosSPV1 = rGlobal1.Z();
      if (m_SwapTel) {
        fPosSPV1 = rGlobal1.Y();
        fPosSPU1 = rGlobal1.Z();
      }
    }
    // hit maps for PXD:
    if ((iIsPXD1 == 1) && (iIsU1 == 1) && (iIsV1 == 1)) {
      m_correlations[c_nTBPlanes * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
    }
    // hit maps for Tel:
    if ((iIsTel1 == 1) && (iIsU1 == 1) && (iIsV1 == 1)) {
      m_correlations[c_nTBPlanes * index1 + index1]->Fill(fPosSPU1, fPosSPV1);
    }
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storePXDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fTime2 = 0.0;
      float fPosSPU2 = 0.0;
      float fPosSPV2 = 0.0;
      int iIsPXD2 = 0;
      int iIsTel2 = 0;
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
        if (m_SwapPXD) {
          fPosSPV2 = rGlobal2.Y();
          fPosSPU2 = rGlobal2.Z();
        }
      } else if ((i2 >= storePXDClusters.getEntries())
                 && (i2 < (storePXDClusters.getEntries() + storeSVDClusters.getEntries()))) {                                  // SVD clusters:
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
      } else {                                  // Tel clusters:
        const TelCluster& clusterTel2 = *storeTelClusters[ i2 - (storePXDClusters.getEntries() + storeSVDClusters.getEntries())];
        iPlane2 = clusterTel2.getSensorID().getSensorNumber();
        if ((iPlane2 < c_firstTelPlane) || (iPlane2 > c_lastTelPlane)) continue;
        index2 = c_nVXDPlanes + iPlane2 - c_firstTelPlane;
        VxdID sensorID2 = clusterTel2.getSensorID();
        auto info = dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID2));
        TVector3 rLocal2(clusterTel2.getU(), clusterTel2.getV(), 0);
        TVector3 rGlobal2 = info.pointToGlobal(rLocal2);
        iIsTel2 = 1;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosSPU2 = rGlobal2.Y();
        fPosSPV2 = rGlobal2.Z();
        if (m_SwapTel) {
          fPosSPV2 = rGlobal2.Y();
          fPosSPU2 = rGlobal2.Z();
        }
      }
      if ((iIsPXD1 == 0) && (iIsPXD2 == 0) && (iIsTel1 == 0) && (iIsTel2 == 0)) // only for SVD - filter:
        if ((fabs(fTime1 - fTime2)) > CutDQMCorrelTime) continue;
      if (!m_SaveOtherHistos)
        if (abs(index1 - index2) > 1) continue;  // only neighboars
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1) && (iIsPXD1 == 0) && (iIsPXD2 == 0) && (iIsTel1 == 0) && (iIsTel2 == 0)) {
        // hit maps for SVD:
        m_correlations[c_nTBPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPV2);
      } else if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlations[c_nTBPlanes * index2 + index1]->Fill(fPosSPU1, fPosSPU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlations[c_nTBPlanes * index2 + index1]->Fill(fPosSPV2, fPosSPV1);
      }
    }
  }

}


void VXDTelDQMOffLineModule::endRun()
{
}


void VXDTelDQMOffLineModule::terminate()
{
}
