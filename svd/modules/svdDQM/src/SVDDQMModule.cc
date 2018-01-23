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

#include "svd/modules/svdDQM/SVDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"

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
  addParam("UseDigits", m_UseStrips,
           "flag <0,1> for using digits only, no cluster information will be required, default = 0 ", m_UseStrips);
  addParam("SaveOtherHistos", m_SaveOtherHistos,
           "flag <0,1> for creation of advance plots, default = 0 ", m_SaveOtherHistos);
  addParam("CutSVDCharge", m_CutSVDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 22 ", m_CutSVDCharge);

}


SVDDQMModule::~SVDDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDDQMModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;

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
  TDirectory* DirSVDAdvance = NULL;
  DirSVDBasic = oldDir->mkdir("SVD_Basic");
  if (m_SaveOtherHistos) {
    DirSVDAdvance = oldDir->mkdir("SVD_Advance");
  }

  m_firedU = new TH1F*[c_nSVDSensors];
  m_firedV = new TH1F*[c_nSVDSensors];
  m_clustersU = new TH1F*[c_nSVDSensors];
  m_clustersV = new TH1F*[c_nSVDSensors];
  m_hitMapU = new TH2F*[c_nSVDSensors];
  m_hitMapV = new TH2F*[c_nSVDSensors];
  m_hitMapUCl = new TH1F*[c_nSVDSensors];
  m_hitMapVCl = new TH1F*[c_nSVDSensors];
  m_chargeU = new TH1F*[c_nSVDSensors];
  m_chargeV = new TH1F*[c_nSVDSensors];
  m_chargeUSt = new TH1F*[c_nSVDSensors];
  m_chargeVSt = new TH1F*[c_nSVDSensors];
  m_seedU = new TH1F*[c_nSVDSensors];
  m_seedV = new TH1F*[c_nSVDSensors];
  m_sizeU = new TH1F*[c_nSVDSensors];
  m_sizeV = new TH1F*[c_nSVDSensors];
  m_timeU = new TH1F*[c_nSVDSensors];
  m_timeV = new TH1F*[c_nSVDSensors];
  for (int i = 0; i < c_nSVDSensors; i++) {
    m_firedU[i] = NULL;
    m_firedV[i] = NULL;
    m_clustersU[i] = NULL;
    m_clustersV[i] = NULL;
    m_hitMapU[i] = NULL;
    m_hitMapV[i] = NULL;
    m_hitMapUCl[i] = NULL;
    m_hitMapVCl[i] = NULL;
    m_chargeU[i] = NULL;
    m_chargeV[i] = NULL;
    m_chargeUSt[i] = NULL;
    m_chargeVSt[i] = NULL;
    m_seedU[i] = NULL;
    m_seedV[i] = NULL;
    m_sizeU[i] = NULL;
    m_sizeV[i] = NULL;
    m_timeU[i] = NULL;
    m_timeV[i] = NULL;
  }
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
    // Fired strip counts U
    string name = str(format("SVD_%1%_FiredU") % sensorDescr);
    string title = str(format("SVD Sensor %1% Fired strips in U") % sensorDescr);
    m_firedU[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedU[i]->GetXaxis()->SetTitle("# of fired u strips");
    m_firedU[i]->GetYaxis()->SetTitle("count");
    // Fired strip counts V
    name = str(format("SVD_%1%_FiredV") % sensorDescr);
    title = str(format("SVD Sensor %1% Fired strips in V") % sensorDescr);
    m_firedV[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_firedV[i]->GetXaxis()->SetTitle("# of fired v strips");
    m_firedV[i]->GetYaxis()->SetTitle("count");
    if (!m_UseStrips) {
      //----------------------------------------------------------------
      // Number of clusters per frame
      //----------------------------------------------------------------
      // Number of clusters U
      name = str(format("SVD_%1%_ClustersU") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of clusters in U") % sensorDescr);
      m_clustersU[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersU[i]->GetXaxis()->SetTitle("# of u clusters");
      m_clustersU[i]->GetYaxis()->SetTitle("count");
      // Number of clusters V
      name = str(format("SVD_%1%_ClustersV") % sensorDescr);
      title = str(format("SVD Sensor %1% Number of clusters in V") % sensorDescr);
      m_clustersV[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
      m_clustersV[i]->GetXaxis()->SetTitle("# of v clusters");
      m_clustersV[i]->GetYaxis()->SetTitle("count");
    }

    if (m_SaveOtherHistos) {
      DirSVDAdvance->cd();
      //----------------------------------------------------------------
      // Hitmaps: Number of strips by coordinate
      //----------------------------------------------------------------
      // Hitmaps in U
      name = str(format("SVD_%1%_StripHitmapU") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in U") % sensorDescr);
      int nStrips = SensorInfo.getUCells();
      m_hitMapU[i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, 6, 0, 6);
      m_hitMapU[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapU[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapU[i]->GetZaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("SVD_%1%_StripHitmapV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip Hitmap in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapV[i] = new TH2F(name.c_str(), title.c_str(), nStrips, 0, nStrips, 6, 0, 6);
      m_hitMapV[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapV[i]->GetYaxis()->SetTitle("timebin [time units]");
      m_hitMapV[i]->GetZaxis()->SetTitle("hits");
    }
    DirSVDBasic->cd();
    if (!m_UseStrips) {
      //----------------------------------------------------------------
      // Hitmaps: Number of clusters by coordinate
      //----------------------------------------------------------------
      // Hitmaps in U
      name = str(format("SVD_%1%_HitmapClstU") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in U") % sensorDescr);
      int nStrips = SensorInfo.getUCells();
      m_hitMapUCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapUCl[i]->GetXaxis()->SetTitle("u position [pitch units]");
      m_hitMapUCl[i]->GetYaxis()->SetTitle("hits");
      // Hitmaps in V
      name = str(format("SVD_%1%_HitmapClstV") % sensorDescr);
      title = str(format("SVD Sensor %1% Hitmap Clusters in V") % sensorDescr);
      nStrips = SensorInfo.getVCells();
      m_hitMapVCl[i] = new TH1F(name.c_str(), title.c_str(), nStrips, 0, nStrips);
      m_hitMapVCl[i]->GetXaxis()->SetTitle("v position [pitch units]");
      m_hitMapVCl[i]->GetYaxis()->SetTitle("hits");
      //----------------------------------------------------------------
      // Charge of clusters
      //----------------------------------------------------------------
      // u charge by sensor
      name = str(format("SVD_%1%_ClusterChargeU") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster charge in U") % sensorDescr);
      m_chargeU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
      m_chargeU[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
      m_chargeU[i]->GetYaxis()->SetTitle("count");
      // v charge by sensor
      name = str(format("SVD_%1%_ClusterChargeV") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster charge in V") % sensorDescr);
      m_chargeV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
      m_chargeV[i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
      m_chargeV[i]->GetYaxis()->SetTitle("count");
    }
    if (m_SaveOtherHistos) {
      DirSVDAdvance->cd();
      //----------------------------------------------------------------
      // Charge of strips
      //----------------------------------------------------------------
      // u charge by sensor
      name = str(format("SVD_%1%_StripChargeU") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip charge in U") % sensorDescr);
      m_chargeUSt[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
      m_chargeUSt[i]->GetXaxis()->SetTitle("charge of u clusters [ADU]");
      m_chargeUSt[i]->GetYaxis()->SetTitle("count");
      // v charge by sensor
      name = str(format("SVD_%1%_StripChargeV") % sensorDescr);
      title = str(format("SVD Sensor %1% Strip charge in V") % sensorDescr);
      m_chargeVSt[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
      m_chargeVSt[i]->GetXaxis()->SetTitle("charge of v clusters [ADU]");
      m_chargeVSt[i]->GetYaxis()->SetTitle("count");
    }
    DirSVDBasic->cd();
    if (!m_UseStrips) {
      if (m_SaveOtherHistos) {
        DirSVDAdvance->cd();
        //----------------------------------------------------------------
        // Cluster seed charge distribution
        //----------------------------------------------------------------
        // u seed by sensor
        name = str(format("SVD_%1%_SeedU") % sensorDescr);
        title = str(format("SVD Sensor %1% Seed charge in U") % sensorDescr);
        m_seedU[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
        m_seedU[i]->GetXaxis()->SetTitle("seed charge of u clusters [ADU]");
        m_seedU[i]->GetYaxis()->SetTitle("count");
        // v seed by sensor
        name = str(format("SVD_%1%_SeedV") % sensorDescr);
        title = str(format("SVD Sensor %1% Seed charge in V") % sensorDescr);
        m_seedV[i] = new TH1F(name.c_str(), title.c_str(), 200, 0, 600);
        m_seedV[i]->GetXaxis()->SetTitle("seed charge of v clusters [ADU]");
        m_seedV[i]->GetYaxis()->SetTitle("count");
      }
      DirSVDBasic->cd();
      //----------------------------------------------------------------
      // Cluster size distribution
      //----------------------------------------------------------------
      // u size by sensor
      name = str(format("SVD_%1%_ClusterSizeU") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster size in U") % sensorDescr);
      m_sizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeU[i]->GetXaxis()->SetTitle("size of u clusters");
      m_sizeU[i]->GetYaxis()->SetTitle("count");
      // v size by sensor
      name = str(format("SVD_%1%_ClusterSizeV") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster size in V") % sensorDescr);
      m_sizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
      m_sizeV[i]->GetXaxis()->SetTitle("size of v clusters");
      m_sizeV[i]->GetYaxis()->SetTitle("count");
      //----------------------------------------------------------------
      // Cluster time distribution
      //----------------------------------------------------------------
      // u time by sensor
      name = str(format("SVD_%1%_ClusterTimeU") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster time in U") % sensorDescr);
      m_timeU[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeU[i]->GetXaxis()->SetTitle("time of u clusters [ns]");
      m_timeU[i]->GetYaxis()->SetTitle("count");
      // v time by sensor
      name = str(format("SVD_%1%_ClusterTimeV") % sensorDescr);
      title = str(format("SVD Sensor %1% Cluster time in V") % sensorDescr);
      m_timeV[i] = new TH1F(name.c_str(), title.c_str(), 50, -50, 150);
      m_timeV[i]->GetXaxis()->SetTitle("time of v clusters [ns]");
      m_timeV[i]->GetYaxis()->SetTitle("count");
    }
  }

  oldDir->cd();
}


void SVDDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_relSVDClusterDigitName = relSVDClusterDigits.getName();

  //Store names to speed up creation later
  m_storeSVDDigitsName = storeSVDDigits.getName();

}

void SVDDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (int i = 0; i < c_nSVDSensors; i++) {
    if (m_firedU[i] != NULL) m_firedU[i]->Reset();
    if (m_firedV[i] != NULL) m_firedV[i]->Reset();
    if (m_clustersU[i] != NULL) m_clustersU[i]->Reset();
    if (m_clustersV[i] != NULL) m_clustersV[i]->Reset();
    if (m_hitMapU[i] != NULL) m_hitMapU[i]->Reset();
    if (m_hitMapV[i] != NULL) m_hitMapV[i]->Reset();
    if (m_hitMapUCl[i] != NULL) m_hitMapUCl[i]->Reset();
    if (m_hitMapVCl[i] != NULL) m_hitMapVCl[i]->Reset();
    if (m_chargeU[i] != NULL) m_chargeU[i]->Reset();
    if (m_chargeV[i] != NULL) m_chargeV[i]->Reset();
    if (m_chargeUSt[i] != NULL) m_chargeUSt[i]->Reset();
    if (m_chargeVSt[i] != NULL) m_chargeVSt[i]->Reset();
    if (m_seedU[i] != NULL) m_seedU[i]->Reset();
    if (m_seedV[i] != NULL) m_seedV[i]->Reset();
    if (m_sizeU[i] != NULL) m_sizeU[i]->Reset();
    if (m_sizeV[i] != NULL) m_sizeV[i]->Reset();
    if (m_timeU[i] != NULL) m_timeU[i]->Reset();
    if (m_timeV[i] != NULL) m_timeV[i]->Reset();
  }
}


void SVDDQMModule::event()
{
  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);

  // If there are no digits, leave
  if (!storeSVDDigits || !storeSVDDigits.getEntries()) return;

  // SVD basic histograms:
  // Fired strips
  vector< set<int> > uStrips(c_nSVDSensors); // sets to eliminate multiple samples per strip
  vector< set<int> > vStrips(c_nSVDSensors);
  for (const SVDDigit& digitIn : storeSVDDigits) {
    int iLayer = digitIn.getSensorID().getLayerNumber();
    if ((iLayer < c_firstSVDLayer) || (iLayer > c_lastSVDLayer)) continue;
    int iLadder = digitIn.getSensorID().getLadderNumber();
    int iSensor = digitIn.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    SVD::SensorInfo SensorInfo = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    if (digitIn.isUStrip()) {
      uStrips.at(index).insert(digitIn.getCellID());
      if (m_chargeUSt[index] != NULL) m_chargeUSt[index]->Fill(digitIn.getCharge());
      if ((m_hitMapU[index] != NULL) && (digitIn.getCharge() > m_CutSVDCharge))
        m_hitMapU[index]->Fill(digitIn.getCellID(), digitIn.getIndex());
//        m_hitMapU[index]->Fill(SensorInfo.getUCellID(digitIn.getCellPosition()), digitIn.getIndex());
    } else {
      vStrips.at(index).insert(digitIn.getCellID());
      if (m_chargeVSt[index] != NULL) m_chargeVSt[index]->Fill(digitIn.getCharge());
      if ((m_hitMapV[index] != NULL) && (digitIn.getCharge() > m_CutSVDCharge))
        m_hitMapV[index]->Fill(digitIn.getCellID(), digitIn.getIndex());
//        m_hitMapV[index]->Fill(SensorInfo.getVCellID(digitIn.getCellPosition()), digitIn.getIndex());
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
  if (!m_UseStrips) {
    // Hitmaps, Charge, Seed, Size, Time
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
        if (m_hitMapUCl[index] != NULL) m_hitMapUCl[index]->Fill(SensorInfo.getUCellID(cluster.getPosition()));
        if (m_chargeU[index] != NULL) m_chargeU[index]->Fill(cluster.getCharge());
        if (m_seedU[index] != NULL) m_seedU[index]->Fill(cluster.getSeedCharge());
        if (m_sizeU[index] != NULL) m_sizeU[index]->Fill(cluster.getSize());
        if (m_timeU[index] != NULL) m_timeU[index]->Fill(cluster.getClsTime());
      } else {
        countsV.at(index).insert(SensorInfo.getVCellID(cluster.getPosition()));
        if (m_hitMapVCl[index] != NULL) m_hitMapVCl[index]->Fill(SensorInfo.getVCellID(cluster.getPosition()));
        if (m_chargeV[index] != NULL) m_chargeV[index]->Fill(cluster.getCharge());
        if (m_seedV[index] != NULL) m_seedV[index]->Fill(cluster.getSeedCharge());
        if (m_sizeV[index] != NULL) m_sizeV[index]->Fill(cluster.getSize());
        if (m_timeV[index] != NULL) m_timeV[index]->Fill(cluster.getClsTime());
      }
    }
    for (int i = 0; i < c_nSVDSensors; i++) {
      if ((m_clustersU[i] != NULL) && (countsU[i].size() > 0))
        m_clustersU[i]->Fill(countsU[i].size());
      if ((m_clustersV[i] != NULL) && (countsV[i].size() > 0))
        m_clustersV[i]->Fill(countsV[i].size());
    }
  }
}


void SVDDQMModule::endRun()
{
}


void SVDDQMModule::terminate()
{
}

int SVDDQMModule::getSensorIndex(int Layer, int Ladder, int Sensor)
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

void SVDDQMModule::getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor)
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
