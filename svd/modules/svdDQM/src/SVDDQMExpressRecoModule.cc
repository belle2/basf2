/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDDQMExpressRecoModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TVectorD.h"
#include "TF1.h"

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMExpressReco)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMExpressRecoModule::SVDDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("SVD DQM module for Express Reco"
                 "Recommended Number of events for monitor is 40 kEvents or more to fill all histograms "
                );

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("CutSVDCharge", m_CutSVDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 22 ", m_CutSVDCharge);
  addParam("ReferenceHistosFileName", m_RefHistFileName,
           "Name of file contain reference histograms, default=vxd/data/VXD-DQMReferenceHistos.root", m_RefHistFileName);
  addParam("NotUseDB", m_NotUseDB,
           "Using local files instead of DataBase for reference histogram, default=0 ", m_NotUseDB);
  addParam("CreateDB", m_CreateDB,
           "Create and fill reference histograms in DataBase, default=0 ", m_CreateDB);
}


SVDDQMExpressRecoModule::~SVDDQMExpressRecoModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMExpressRecoModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  m_oldDir = gDirectory;

  // basic constants presets:
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  c_nVXDLayers = geo.getLayers().size();
  c_firstVXDLayer = 1;  // counting start from 1...
  c_lastVXDLayer = c_nVXDLayers;
  c_nPXDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size();
  c_firstPXDLayer = c_firstVXDLayer;
  c_lastPXDLayer = c_nPXDLayers;
  c_nSVDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::SVD).size();
  c_firstSVDLayer = c_nPXDLayers + c_firstPXDLayer;
  c_lastSVDLayer = c_firstSVDLayer + c_nSVDLayers;

  c_nSVDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() > c_lastPXDLayer) {  // SVD
        c_nSVDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  TDirectory* DirSVDBasic = NULL;
  TDirectory* DirSVDFlags = NULL;
  DirSVDBasic = m_oldDir->mkdir("SVDExpReco");
  DirSVDFlags = m_oldDir->mkdir("SVDExpReco_Flags");

  // Create basic histograms:
  DirSVDBasic->cd();
  m_hitMapCountsU = new TH1I("DQMER_SVD_StripHitmapCountsU", "DQM ER SVD U Strip Hitmaps Counts",
                             c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapCountsV = new TH1I("DQMER_SVD_StripHitmapCountsV", "DQM ER SVD V Strip Hitmaps Counts",
                             c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCountsV->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsU = new TH1I("DQMER_SVD_ClusterHitmapCountsU", "DQM ER SVD U Cluster Hitmaps Counts",
                               c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapClCountsU->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsU->GetYaxis()->SetTitle("counts");
  m_hitMapClCountsV = new TH1I("DQMER_SVD_ClusterHitmapCountsV", "DQM ER SVD V Cluster Hitmaps Counts",
                               c_nSVDSensors, 0, c_nSVDSensors);
  m_hitMapClCountsV->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCountsV->GetYaxis()->SetTitle("counts");

  m_firedU = new TH1F*[c_nSVDSensors];
  m_firedV = new TH1F*[c_nSVDSensors];
  m_clustersU = new TH1F*[c_nSVDSensors];
  m_clustersV = new TH1F*[c_nSVDSensors];

  m_clusterChargeU = new TH1F*[c_nSVDSensors];
  m_clusterChargeV = new TH1F*[c_nSVDSensors];
  m_stripSignalU = new TH1F*[c_nSVDSensors];
  m_stripSignalV = new TH1F*[c_nSVDSensors];
  m_clusterSizeU = new TH1F*[c_nSVDSensors];
  m_clusterSizeV = new TH1F*[c_nSVDSensors];
  m_clusterTimeU = new TH1F*[c_nSVDSensors];
  m_clusterTimeV = new TH1F*[c_nSVDSensors];

  for (int i = 0; i < c_nSVDSensors; i++) {
    DirSVDBasic->cd();
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired strips per frame
    //----------------------------------------------------------------
    string name = str(format("DQMER_SVD_%1%_FiredU") % sensorDescr);
    string title = str(format("DQM ER SVD Sensor %1% Fired strips in U") % sensorDescr);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_FiredV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Fired strips in V") % sensorDescr);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClustersU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Number of clusters in U") % sensorDescr);
    m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clustersU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClustersV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Number of clusters in V") % sensorDescr);
    m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
    m_clustersV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of clusters
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterChargeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster charge in U") % sensorDescr);
    m_clusterChargeU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 300000);
    m_clusterChargeU[i]->GetXaxis()->SetTitle("charge of u clusters [el]");
    m_clusterChargeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster charge in V") % sensorDescr);
    m_clusterChargeV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 300000);
    m_clusterChargeV[i]->GetXaxis()->SetTitle("charge of v clusters [el]");
    m_clusterChargeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Charge of strips
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_StripChargeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in U") % sensorDescr);
    m_stripSignalU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
    m_stripSignalU[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
    m_stripSignalU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_StripChargeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Strip charge in V") % sensorDescr);
    m_stripSignalV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
    m_stripSignalV[i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
    m_stripSignalV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster size distribution
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster size in U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster size in V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster time distribution
    //----------------------------------------------------------------
    name = str(format("DQMER_SVD_%1%_ClusterTimeU") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster time in U") % sensorDescr);
    m_clusterTimeU[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_clusterTimeU[i]->GetXaxis()->SetTitle("time of u clusters [ns]");
    m_clusterTimeU[i]->GetYaxis()->SetTitle("count");
    name = str(format("DQMER_SVD_%1%_ClusterTimeV") % sensorDescr);
    title = str(format("DQM ER SVD Sensor %1% Cluster time in V") % sensorDescr);
    m_clusterTimeV[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
    m_clusterTimeV[i]->GetXaxis()->SetTitle("time of v clusters [ns]");
    m_clusterTimeV[i]->GetYaxis()->SetTitle("count");
  }

  // Create flag histograms:
  DirSVDFlags->cd();
  m_fHitMapCountsUFlag = new TH1I("DQMER_SVD_StripHitmapCountsUFlag", "PXD Strip Hitmaps Counts U Flag",
                                  c_nSVDSensors, 0, c_nSVDSensors);
  m_fHitMapCountsUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapCountsUFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapCountsVFlag = new TH1I("DQMER_SVD_StripHitmapCountsVFlag", "PXD Strip Hitmaps Counts V Flag",
                                  c_nSVDSensors, 0, c_nSVDSensors);
  m_fHitMapCountsVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapCountsVFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapClCountsUFlag = new TH1I("DQMER_SVD_ClusterHitmapCountsUFlag", "PXD Cluster Hitmaps Counts U Flag",
                                    c_nSVDSensors, 0, c_nSVDSensors);
  m_fHitMapClCountsUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapClCountsUFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapClCountsVFlag = new TH1I("DQMER_SVD_ClusterHitmapCountsVFlag", "PXD Cluster Hitmaps Counts V Flag",
                                    c_nSVDSensors, 0, c_nSVDSensors);
  m_fHitMapClCountsVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapClCountsVFlag->GetYaxis()->SetTitle("flag");
  m_fFiredUFlag = new TH1I("DQMER_SVD_FiredUFlag", "DQM ER SVD Fired U Flag",
                           c_nSVDSensors, 0, c_nSVDSensors);
  m_fFiredUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fFiredUFlag->GetYaxis()->SetTitle("flag");
  m_fFiredVFlag = new TH1I("DQMER_SVD_FiredVFlag", "DQM ER SVD Fired V Flag",
                           c_nSVDSensors, 0, c_nSVDSensors);
  m_fFiredVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fFiredVFlag->GetYaxis()->SetTitle("flag");
  m_fClustersUFlag = new TH1I("DQMER_SVD_ClustersUFlag", "DQM ER SVD Clusters U Flag",
                              c_nSVDSensors, 0, c_nSVDSensors);
  m_fClustersUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClustersUFlag->GetYaxis()->SetTitle("flag");
  m_fClustersVFlag = new TH1I("DQMER_SVD_ClustersVFlag", "DQM ER SVD Clusters V Flag",
                              c_nSVDSensors, 0, c_nSVDSensors);
  m_fClustersVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClustersVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterChargeUFlag = new TH1I("DQMER_SVD_ClusterChargeUFlag", "DQM ER SVD Cluster Charge U Flag",
                                   c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterChargeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterChargeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterChargeVFlag = new TH1I("DQMER_SVD_ClusterChargeVFlag", "DQM ER SVD Cluster Charge V Flag",
                                   c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterChargeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterChargeVFlag->GetYaxis()->SetTitle("flag");
  m_fStripSignalUFlag = new TH1I("DQMER_SVD_StripSignalUFlag", "DQM ER SVD Strip Signal U Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fStripSignalUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStripSignalUFlag->GetYaxis()->SetTitle("flag");
  m_fStripSignalVFlag = new TH1I("DQMER_SVD_StripSignalVFlag", "DQM ER SVD Strip Signal V Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fStripSignalVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStripSignalVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUFlag = new TH1I("DQMER_SVD_ClasterSizeUFlag", "DQM ER SVD Cluster Size U Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterSizeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeVFlag = new TH1I("DQMER_SVD_ClasterSizeVFlag", "DQM ER SVD Cluster Size V Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterSizeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterTimeUFlag = new TH1I("DQMER_SVD_ClasterTimeUFlag", "DQM ER SVD Cluster Time U Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterTimeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterTimeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterTimeVFlag = new TH1I("DQMER_SVD_ClasterTimeVFlag", "DQM ER SVD Cluster Time V Flag",
                                 c_nSVDSensors, 0, c_nSVDSensors);
  m_fClusterTimeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterTimeVFlag->GetYaxis()->SetTitle("flag");

  for (int i = 0; i < c_nSVDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsU->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCountsV->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapCountsUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapCountsVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapClCountsUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapClCountsVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fFiredUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fFiredVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClustersUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClustersVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterChargeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterChargeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStripSignalUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStripSignalVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterTimeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterTimeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }
  m_oldDir->cd();
}


void SVDDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  m_storeSVDClustersName = storeSVDClusters.getName();

  storeSVDClusters.registerInDataStore();
  storeSVDShaperDigits.isOptional();

  //Store names to speed up creation later
  m_storeSVDShaperDigitsName = storeSVDShaperDigits.getName();

}

void SVDDQMExpressRecoModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (m_fHitMapCountsUFlag != NULL) m_fHitMapCountsUFlag->Reset();
  if (m_fHitMapCountsVFlag != NULL) m_fHitMapCountsVFlag->Reset();
  if (m_fHitMapClCountsUFlag != NULL) m_fHitMapClCountsUFlag->Reset();
  if (m_fHitMapClCountsVFlag != NULL) m_fHitMapClCountsVFlag->Reset();
  if (m_fFiredUFlag != NULL) m_fFiredUFlag->Reset();
  if (m_fFiredVFlag != NULL) m_fFiredVFlag->Reset();
  if (m_fClustersUFlag != NULL) m_fClustersUFlag->Reset();
  if (m_fClustersVFlag != NULL) m_fClustersVFlag->Reset();
  if (m_fClusterChargeUFlag != NULL) m_fClusterChargeUFlag->Reset();
  if (m_fClusterChargeVFlag != NULL) m_fClusterChargeVFlag->Reset();
  if (m_fStripSignalUFlag != NULL) m_fStripSignalUFlag->Reset();
  if (m_fStripSignalVFlag != NULL) m_fStripSignalVFlag->Reset();
  if (m_fClusterSizeUFlag != NULL) m_fClusterSizeUFlag->Reset();
  if (m_fClusterSizeVFlag != NULL) m_fClusterSizeVFlag->Reset();
  if (m_fClusterTimeUFlag != NULL) m_fClusterTimeUFlag->Reset();
  if (m_fClusterTimeVFlag != NULL) m_fClusterTimeVFlag->Reset();

  if (m_hitMapCountsU != NULL) m_hitMapCountsU->Reset();
  if (m_hitMapCountsV != NULL) m_hitMapCountsV->Reset();
  if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Reset();
  if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Reset();
  for (int i = 0; i < c_nSVDSensors; i++) {
    if (m_firedU[i] != NULL) m_firedU[i]->Reset();
    if (m_firedV[i] != NULL) m_firedV[i]->Reset();
    if (m_clustersU[i] != NULL) m_clustersU[i]->Reset();
    if (m_clustersV[i] != NULL) m_clustersV[i]->Reset();
    if (m_clusterChargeU[i] != NULL) m_clusterChargeU[i]->Reset();
    if (m_clusterChargeV[i] != NULL) m_clusterChargeV[i]->Reset();
    if (m_stripSignalU[i] != NULL) m_stripSignalU[i]->Reset();
    if (m_stripSignalV[i] != NULL) m_stripSignalV[i]->Reset();
    if (m_clusterSizeU[i] != NULL) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != NULL) m_clusterSizeV[i]->Reset();
    if (m_clusterTimeU[i] != NULL) m_clusterTimeU[i]->Reset();
    if (m_clusterTimeV[i] != NULL) m_clusterTimeV[i]->Reset();
  }
  m_NoOfEvents = 0;
}


void SVDDQMExpressRecoModule::event()
{

  const StoreArray<SVDShaperDigit> storeSVDShaperDigits(m_storeSVDShaperDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);

  m_NoOfEvents++;
  if (!storeSVDShaperDigits || !storeSVDShaperDigits.getEntries()) {
    return;
  }

  // SVD basic histograms:
  // Fired strips
  vector< set<int> > uStrips(c_nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDSensors);
  for (const SVDShaperDigit& digitIn : storeSVDShaperDigits) {
    int iLayer = digitIn.getSensorID().getLayerNumber();
    if ((iLayer < c_firstSVDLayer) || (iLayer > c_lastSVDLayer)) continue;
    int iLadder = digitIn.getSensorID().getLadderNumber();
    int iSensor = digitIn.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (digitIn.isUStrip()) {
      uStrips.at(index).insert(digitIn.getCellID());
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalU[index] != NULL) m_stripSignalU[index]->Fill(samples[i]);
        if ((m_hitMapCountsU != NULL) && (samples[i] > m_CutSVDCharge))
          m_hitMapCountsU->Fill(index);
      }
    } else {
      vStrips.at(index).insert(digitIn.getCellID());
      // 6-to-1 relation weights are equal to digit signals, modulo rounding error
      SVDShaperDigit::APVFloatSamples samples = digitIn.getSamples();
      for (size_t i = 0; i < SVDShaperDigit::c_nAPVSamples; ++i) {
        if (m_stripSignalV[index] != NULL) m_stripSignalV[index]->Fill(samples[i]);
        if ((m_hitMapCountsV != NULL) && (samples[i] > m_CutSVDCharge))
          m_hitMapCountsV->Fill(index);
      }
    }
  }
  for (int i = 0; i < c_nSVDSensors; i++) {
    if ((m_firedU[i] != NULL) && (uStrips[i].size() > 0))
      m_firedU[i]->Fill(uStrips[i].size());
    if ((m_firedV[i] != NULL) && (vStrips[i].size() > 0))
      m_firedV[i]->Fill(vStrips[i].size());
  }

  vector< set<int> > countsU(c_nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > countsV(c_nSVDSensors);
  // Hitmaps, Charge, Seed, Size, Time, ...
  for (const SVDCluster& cluster : storeSVDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < c_firstSVDLayer) || (iLayer > c_lastSVDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (cluster.isUCluster()) {
      countsU.at(index).insert(SensorInfo.getUCellID(cluster.getPosition()));
      if (m_hitMapClCountsU != NULL) m_hitMapClCountsU->Fill(index);
      if (m_clusterChargeU[index] != NULL) m_clusterChargeU[index]->Fill(cluster.getCharge());
      if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getSize());
      if (m_clusterTimeU[index] != NULL) m_clusterTimeU[index]->Fill(cluster.getClsTime());
    } else {
      countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
      if (m_hitMapClCountsV != NULL) m_hitMapClCountsV->Fill(index);
      if (m_clusterChargeV[index] != NULL) m_clusterChargeV[index]->Fill(cluster.getCharge());
      if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getSize());
      if (m_clusterTimeV[index] != NULL) m_clusterTimeV[index]->Fill(cluster.getClsTime());
    }
  }
  for (int i = 0; i < c_nSVDSensors; i++) {
    if ((m_clustersU[i] != NULL) && (countsU[i].size() > 0))
      m_clustersU[i]->Fill(countsU[i].size());
    if ((m_clustersV[i] != NULL) && (countsV[i].size() > 0))
      m_clustersV[i]->Fill(countsV[i].size());
  }
}


void SVDDQMExpressRecoModule::endRun()
{
  TVectorD* NoOfEvents;
  NoOfEvents = new TVectorD(1);
  double fNoOfEvents[1];
  fNoOfEvents[0] = m_NoOfEvents;
  NoOfEvents->SetElements(fNoOfEvents);
  TString nameBS = Form("DQMER_SVD_NoOfEvents");
  m_oldDir->cd();
  NoOfEvents->Write(nameBS.Data());

  TDirectory* DirSVDRefs = NULL;
  DirSVDRefs = m_oldDir->mkdir("SVDExpReco_Refs");
  DirSVDRefs->cd();
  // Load reference file of histograms:
  TVectorD* NoOfEventsRef;
  NoOfEventsRef = new TVectorD(1);
  double fNoOfEventsRef[1];

  TH1I* r_hitMapCountsU;
  TH1I* r_hitMapCountsV;
  TH1I* r_hitMapClCountsU;
  TH1I* r_hitMapClCountsV;
  TH1F** r_firedU = new TH1F*[c_nSVDSensors];
  TH1F** r_firedV = new TH1F*[c_nSVDSensors];
  TH1F** r_clustersU = new TH1F*[c_nSVDSensors];
  TH1F** r_clustersV = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterChargeU = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterChargeV = new TH1F*[c_nSVDSensors];
  TH1F** r_stripSignalU = new TH1F*[c_nSVDSensors];
  TH1F** r_stripSignalV = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterSizeU = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterSizeV = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterTimeU = new TH1F*[c_nSVDSensors];
  TH1F** r_clusterTimeV = new TH1F*[c_nSVDSensors];

  r_hitMapCountsU = NULL;
  r_hitMapCountsU = new TH1I(*m_hitMapCountsU);
  r_hitMapCountsU->Reset();
  r_hitMapCountsV = NULL;
  r_hitMapCountsV = new TH1I(*m_hitMapCountsV);
  r_hitMapCountsV->Reset();
  r_hitMapClCountsU = NULL;
  r_hitMapClCountsU = new TH1I(*m_hitMapClCountsU);
  r_hitMapClCountsU->Reset();
  r_hitMapClCountsV = NULL;
  r_hitMapClCountsV = new TH1I(*m_hitMapClCountsV);
  r_hitMapClCountsV->Reset();
  for (int i = 0; i < c_nSVDSensors; i++) {
    r_firedU[i] = NULL;
    r_firedU[i] = new TH1F(*m_firedU[i]);
    r_firedU[i]->Reset();
    r_firedV[i] = NULL;
    r_firedV[i] = new TH1F(*m_firedV[i]);
    r_firedV[i]->Reset();
    r_clustersU[i] = NULL;
    r_clustersU[i] = new TH1F(*m_clustersU[i]);
    r_clustersU[i]->Reset();
    r_clustersV[i] = NULL;
    r_clustersV[i] = new TH1F(*m_clustersV[i]);
    r_clustersV[i]->Reset();
    r_clusterChargeU[i] = NULL;
    r_clusterChargeU[i] = new TH1F(*m_clusterChargeU[i]);
    r_clusterChargeU[i]->Reset();
    r_clusterChargeV[i] = NULL;
    r_clusterChargeV[i] = new TH1F(*m_clusterChargeV[i]);
    r_clusterChargeV[i]->Reset();
    r_stripSignalU[i] = NULL;
    r_stripSignalU[i] = new TH1F(*m_stripSignalU[i]);
    r_stripSignalU[i]->Reset();
    r_stripSignalV[i] = NULL;
    r_stripSignalV[i] = new TH1F(*m_stripSignalV[i]);
    r_stripSignalV[i]->Reset();
    r_clusterSizeU[i] = NULL;
    r_clusterSizeU[i] = new TH1F(*m_clusterSizeU[i]);
    r_clusterSizeU[i]->Reset();
    r_clusterSizeV[i] = NULL;
    r_clusterSizeV[i] = new TH1F(*m_clusterSizeV[i]);
    r_clusterSizeV[i]->Reset();
    r_clusterTimeU[i] = NULL;
    r_clusterTimeU[i] = new TH1F(*m_clusterTimeU[i]);
    r_clusterTimeU[i]->Reset();
    r_clusterTimeV[i] = NULL;
    r_clusterTimeV[i] = new TH1F(*m_clusterTimeV[i]);
    r_clusterTimeV[i]->Reset();
  }

  if (m_NotUseDB == 1) {
    TFile* f_RefHistFile = new TFile(m_RefHistFileName.c_str(), "read");
    if (f_RefHistFile->IsOpen()) {
      B2INFO("Reference file name: " << m_RefHistFileName.c_str());
      TVectorD* NoOfEventsRef2 = NULL;
      f_RefHistFile->GetObject("DQMER_SVD_NoOfEvents_Ref", NoOfEventsRef2);
      m_NoOfEventsRef = (int)NoOfEventsRef2->GetMatrixArray()[0];
      //    m_NoOfEventsRef = 2;
      string name = str(format("SVDExpReco/DQMER_SVD_StripHitmapCountsU;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapCountsU);
      if (r_hitMapCountsU == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("SVDExpReco/DQMER_SVD_StripHitmapCountsV;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapCountsV);
      if (r_hitMapCountsV == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("SVDExpReco/DQMER_SVD_ClusterHitmapCountsU;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapClCountsU);
      if (r_hitMapClCountsU == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("SVDExpReco/DQMER_SVD_ClusterHitmapCountsV;1"));
      f_RefHistFile->GetObject(name.c_str(), r_hitMapClCountsV);
      if (r_hitMapClCountsV == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      for (int i = 0; i < c_nSVDSensors; i++) {
        int iLayer = 0;
        int iLadder = 0;
        int iSensor = 0;
        getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
        string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
        name = str(format("SVDExpReco/DQMER_SVD_%1%_FiredU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_firedU[i]);
        if (r_firedU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_FiredV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_firedV[i]);
        if (r_firedV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClustersU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clustersU[i]);
        if (r_clustersU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClustersV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clustersV[i]);
        if (r_clustersV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterChargeU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterChargeU[i]);
        if (r_clusterChargeU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterChargeV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterChargeV[i]);
        if (r_clusterChargeV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_StripChargeU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_stripSignalU[i]);
        if (r_stripSignalU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_StripChargeV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_stripSignalV[i]);
        if (r_stripSignalV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterSizeU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterSizeU[i]);
        if (r_clusterSizeU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterSizeV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterSizeV[i]);
        if (r_clusterSizeV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterTimeU") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterTimeU[i]);
        if (r_clusterTimeU[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
        name = str(format("SVDExpReco/DQMER_SVD_%1%_ClusterTimeV") % sensorDescr);
        f_RefHistFile->GetObject(name.c_str(), r_clusterTimeV[i]);
        if (r_clusterTimeV[i] == NULL) {
          B2INFO("There is missing histogram in reference file: " << name.c_str());
          return;
        }
      }
    } else {
      B2INFO("File of reference histograms: " << m_RefHistFileName.c_str() << " is not available, please check it!");
      return;
    }
  } else {
    if (m_CreateDB == 1) {
      IntervalOfValidity iov(0, 0, -1, -1);
      TString Name = Form("DQMER_SVD_NoOfEvents_Ref");
      DBImportObjPtr<TVectorD> DQMER_SVD_NoOfEvents(Name.Data());
      DQMER_SVD_NoOfEvents.construct(1);
      DQMER_SVD_NoOfEvents->SetElements(fNoOfEvents);
      DQMER_SVD_NoOfEvents.import(iov);

      CreateDBHisto(m_hitMapCountsU);
      CreateDBHisto(m_hitMapCountsV);
      CreateDBHisto(m_hitMapClCountsU);
      CreateDBHisto(m_hitMapClCountsV);
      CreateDBHistoGroup(m_firedU, c_nSVDSensors);
      CreateDBHistoGroup(m_firedV, c_nSVDSensors);
      CreateDBHistoGroup(m_clustersU, c_nSVDSensors);
      CreateDBHistoGroup(m_clustersV, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterChargeU, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterChargeV, c_nSVDSensors);
      CreateDBHistoGroup(m_stripSignalU, c_nSVDSensors);
      CreateDBHistoGroup(m_stripSignalV, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterSizeU, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterSizeV, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterTimeU, c_nSVDSensors);
      CreateDBHistoGroup(m_clusterTimeV, c_nSVDSensors);
    } else {
      TString Name = Form("DQMER_SVD_NoOfEvents_Ref");
      DBObjPtr<TVectorD> DQMER_SVD_NoOfEvents_Ref(Name.Data());
      if (DQMER_SVD_NoOfEvents_Ref.isValid()) {
        m_NoOfEventsRef = (int)DQMER_SVD_NoOfEvents_Ref->GetMatrixArray()[0];
      } else {
        B2INFO("ERROR to open reference counter: DQMER_SVD_NoOfEvents_Ref");
        return;
      }

      int load = 1;
      if (!LoadDBHisto(r_hitMapCountsU)) load = 0;
      if (!LoadDBHisto(r_hitMapCountsV)) load = 0;
      if (!LoadDBHisto(r_hitMapClCountsU)) load = 0;
      if (!LoadDBHisto(r_hitMapClCountsV)) load = 0;
      if (!LoadDBHistoGroup(r_firedU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_firedV, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clustersU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clustersV, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterChargeU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterChargeV, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_stripSignalU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_stripSignalV, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterSizeU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterSizeV, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterTimeU, c_nSVDSensors)) load = 0;
      if (!LoadDBHistoGroup(r_clusterTimeV, c_nSVDSensors)) load = 0;
      if (!load) {
        B2INFO("ERROR to open of reference information");
        return;
      }
    }
  }

  fNoOfEventsRef[0] = m_NoOfEventsRef;
  NoOfEventsRef->SetElements(fNoOfEventsRef);
  NoOfEventsRef->Write("DQMER_SVD_NoOfEvents_Ref");

  // Compare histograms with reference histograms and create flags:
  if (m_CreateDB == 0) {
    for (int i = 0; i < c_nSVDSensors; i++) {
      double pars[2];
      pars[0] = 0.01;
      pars[1] = 0.05;
      SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_hitMapCountsU, r_hitMapCountsU, m_fHitMapCountsUFlag);
      SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_hitMapCountsV, r_hitMapCountsV, m_fHitMapCountsVFlag);
      SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_hitMapClCountsU, r_hitMapClCountsU, m_fHitMapClCountsUFlag);
      SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_hitMapClCountsV, r_hitMapClCountsV, m_fHitMapClCountsVFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_firedU[i], r_firedU[i], m_fFiredUFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_firedV[i], r_firedV[i], m_fFiredVFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clustersU[i], r_clustersU[i], m_fClustersUFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clustersV[i], r_clustersV[i], m_fClustersVFlag);
      SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterChargeU[i], r_clusterChargeU[i], m_fClusterChargeUFlag);
      SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterChargeV[i], r_clusterChargeV[i], m_fClusterChargeVFlag);
      SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_stripSignalU[i], r_stripSignalU[i], m_fStripSignalUFlag);
      SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_stripSignalV[i], r_stripSignalV[i], m_fStripSignalVFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterSizeU[i], r_clusterSizeU[i], m_fClusterSizeUFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterSizeV[i], r_clusterSizeV[i], m_fClusterSizeVFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterTimeU[i], r_clusterTimeU[i], m_fClusterTimeUFlag);
      SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
              m_clusterTimeV[i], r_clusterTimeV[i], m_fClusterTimeVFlag);
    }
  }
}

void SVDDQMExpressRecoModule::terminate()
{
}

int SVDDQMExpressRecoModule::getSensorIndex(int Layer, int Ladder, int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  // printf("  --> SVD uvnitr sensor %i: %i_%i_%i\n", tempcounter, Layer, Ladder, Sensor);
  return tempcounter;
}

void SVDDQMExpressRecoModule::getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= c_lastPXDLayer) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          *Layer = layer.getLayerNumber();
          *Ladder = ladder.getLadderNumber();
          *Sensor = sensor.getSensorNumber();
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  // printf("  --> SVD sensor %i: %i_%i_%i\n", Index, *Layer, *Ladder, *Sensor);
}

int SVDDQMExpressRecoModule::SetFlag(int Type, int bin, double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist)
{
  int iret = 0;
  float WarningLevel = 6.0;
  float ErrorLevel = 10.0;
  TH1F* temp = new TH1F("temp", "temp", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
  double NEvents = 0;
  double flagInt = 0;
  double flagrInt = 0;
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    double val = hist->GetBinContent(j + 1);
    NEvents += val;
    val = val / ratio;
    temp->SetBinContent(j + 1, val);
    flagInt += temp->GetBinContent(j + 1);
    flagrInt += refhist->GetBinContent(j + 1);
  }
  if (NEvents < 100) {  // not enough information for comparition
    delete temp;
    iret = -1;
    flaghist->SetBinContent(bin + 1, -1);
    return iret;
  }
  double flag  = temp->GetMean();
  double flagErr  = temp->GetMeanError();
  double flagRMS  = temp->GetRMS();
  double flagRMSErr  = temp->GetRMSError();
  double flagr  = refhist->GetMean();
  double flagrErr  = refhist->GetMeanError();
  double flagrRMS  = refhist->GetRMS();
  double flagrRMSErr  = refhist->GetRMSError();
  TString strDebugInfo = Form("Conditions for Flag--->\n   source %f %f+-%f %f+-%f\n  referen %f %f+-%f %f+-%f\n",
                              flagInt, flag, flagErr, flagRMS, flagRMSErr,
                              flagrInt, flagr, flagrErr, flagrRMS, flagrRMSErr
                             );
  B2DEBUG(130, strDebugInfo.Data());
  if (Type == 1) {  // counts+mean+RMS use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 2) { // counts use
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 3) { // mean use
    if (fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 4) { // RMS use
    if (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 5) { // counts+mean use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 9) { // bin content use
    flagInt = temp->GetBinContent(bin + 1);
    flagrInt = refhist->GetBinContent(bin + 1);
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 10) {
    float flag2  = refhist->Chi2Test(temp);
    flaghist->SetBinContent(bin + 1, 0);
    if (flag2 > pars[1])
      flaghist->SetBinContent(bin + 1, 2);
    if (flag2 > pars[0])
      flaghist->SetBinContent(bin + 1, 1);
    iret = 1;
  } else if (Type == 100) {
    flaghist->SetBinContent(bin + 1, 0);
    iret = 1;
  } else {
    flaghist->SetBinContent(bin + 1, -3);
    iret = -1;
  }
  delete temp;
  strDebugInfo = Form("SetFlag---> %f, type %i\n", flaghist->GetBinContent(bin + 1), Type);
  B2DEBUG(130, strDebugInfo.Data());
  return iret;
}

int SVDDQMExpressRecoModule::SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist)
{
  TH1F* histF = new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
  TH1F* refhistF = new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
                            refhist->GetXaxis()->GetXmax());
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    histF->SetBinContent(j + 1, hist->GetBinContent(j + 1));
    refhistF->SetBinContent(j + 1, refhist->GetBinContent(j + 1));
  }
  int ret = SetFlag(Type, bin, pars, ratio, histF, refhistF, flaghist);
  delete histF;
  delete refhistF;
  return ret;
}

void SVDDQMExpressRecoModule::CreateDBHisto(TH1I* HistoDB)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(HistoDB->GetNbinsX() + 3);
  double* Content = new double[HistoDB->GetNbinsX() + 3];
  Content[0] = HistoDB->GetNbinsX();
  Content[1] = HistoDB->GetXaxis()->GetXmin();
  Content[2] = HistoDB->GetXaxis()->GetXmax();
  for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
    Content[i + 3] = HistoDB->GetBinContent(i + 1);
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}

void SVDDQMExpressRecoModule::CreateDBHisto(TH1F* HistoDB)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(HistoDB->GetNbinsX() + 3);
  double* Content = new double[HistoDB->GetNbinsX() + 3];
  Content[0] = HistoDB->GetNbinsX();
  Content[1] = HistoDB->GetXaxis()->GetXmin();
  Content[2] = HistoDB->GetXaxis()->GetXmax();
  for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
    Content[i + 3] = HistoDB->GetBinContent(i + 1);
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}

void SVDDQMExpressRecoModule::CreateDBHistoGroup(TH1I** HistoDB, int Number)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(Number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[Number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < Number; j++) {
    for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
      Content[j * HistoDB[j]->GetNbinsX() + i + 3] = HistoDB[j]->GetBinContent(i + 1);
    }
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}

void SVDDQMExpressRecoModule::CreateDBHistoGroup(TH1F** HistoDB, int Number)
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(Number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[Number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < Number; j++) {
    for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
      Content[j * HistoDB[j]->GetNbinsX() + i + 3] = HistoDB[j]->GetBinContent(i + 1);
    }
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}

int SVDDQMExpressRecoModule::LoadDBHisto(TH1I* HistoDB)
{
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    if (ret) {
      for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
        HistoDB->SetBinContent(i + 1, (int)DBHisto->GetMatrixArray()[i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}

int SVDDQMExpressRecoModule::LoadDBHisto(TH1F* HistoDB)
{
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    if (ret) {
      for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
        HistoDB->SetBinContent(i + 1, (int)DBHisto->GetMatrixArray()[i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}

int SVDDQMExpressRecoModule::LoadDBHistoGroup(TH1I** HistoDB, int Number)
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < Number; j++) {
      for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
        HistoDB[j]->SetBinContent(i + 1, DBHisto->GetMatrixArray()[j * HistoDB[j]->GetNbinsX() + i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}

int SVDDQMExpressRecoModule::LoadDBHistoGroup(TH1F** HistoDB, int Number)
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < Number; j++) {
      for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
        HistoDB[j]->SetBinContent(i + 1, DBHisto->GetMatrixArray()[j * HistoDB[j]->GetNbinsX() + i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}
