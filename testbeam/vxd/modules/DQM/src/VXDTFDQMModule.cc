#include "testbeam/vxd/modules/DQM/VXDTFDQMModule.h"
#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>
#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>

#include <boost/format.hpp>

#include "TVector3.h"
#include "TVectorD.h"

#include "TDirectory.h"



using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDTFDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDTFDQMModule::VXDTFDQMModule() : HistoModule()
{
  B2DEBUG(1, "start constructor");
  //Set module properties
  setDescription("VXDTF DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("GFTrackCandidatesColName", m_storeTrackCandsName,
           "Name of collection holding the genfit::TrackCandidates", std::string("caTracks"));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit Validation histograms will be put into this directory", std::string("vxdtf"));
  addParam("storeSvdClustersName", m_storeSvdClustersName,
           "Name of collection holding the SVDClusters", std::string(""));
  addParam("storePxdClustersName", m_storePxdClustersName,
           "Name of collection holding the PXDClusters", std::string(""));
  addParam("storeTelClustersName", m_storeTelClustersName,
           "Name of collection holding the TELClusters wit TelHits", std::string("TELClusters"));

  addParam("ReferenceTCsColName", m_storeReferenceTCsColName,
           "Name of collection holding the genfit::trackCand reference TCs", std::string("caTracks"));
}


VXDTFDQMModule::~VXDTFDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void VXDTFDQMModule::defineHisto()
{
  B2DEBUG(1, "start defineHisto");
  TDirectory* oldDir = gDirectory;
  TDirectory* histDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
  histDir->cd();
//  gStyle->SetOptStat(111111); // includes extra info like underflow and overflow into the legend of the histogram

  m_histoMomentum = new TH1F("VXDTFMomentum", "VXDTF: Total momentum estimated", 100, 0, 15);
  m_histoMomentum->GetXaxis()->SetTitle("value of estimated momentum");
  m_histoMomentum->GetYaxis()->SetTitle("count");

  m_histoPT = new TH1F("VXDTFMomentumT", "VXDTF: Transverse momentum estimated", 100, 0, 15);
  m_histoPT->GetXaxis()->SetTitle("value of estimated transverse momentum");
  m_histoPT->GetYaxis()->SetTitle("count");

  m_histoMomentumX = new TH1F("VXDTFMomentumX", "VXDTF: Momentum in X estimated", 150, -10, 10);
  m_histoMomentumX->GetXaxis()->SetTitle("value of estimated momentum in X");
  m_histoMomentumX->GetYaxis()->SetTitle("count");

  m_histoMomentumY = new TH1F("VXDTFMomentumY", "VXDTF: Momentum in Y  estimated", 150, -2.5, 2.5);
  m_histoMomentumY->GetXaxis()->SetTitle("value of estimated momentum in Y");
  m_histoMomentumY->GetYaxis()->SetTitle("count");

  m_histoMomentumZ = new TH1F("VXDTFMomentumZ", "VXDTF: Momentum in Z  estimated", 150, -2.5, 2.5);
  m_histoMomentumZ->GetXaxis()->SetTitle("value of estimated momentum in Z");
  m_histoMomentumZ->GetYaxis()->SetTitle("count");

  m_histoNumHitsUsed = new TH1I("VXDTFNHits", "VXDTF: Num of hits used for TC", 19, -0.5, 18.5);
  m_histoNumHitsUsed->GetXaxis()->SetTitle("Num of Hits used for TC");
  m_histoNumHitsUsed->GetYaxis()->SetTitle("count");

  m_histoNumHitsIgnored = new TH1I("VXDTFNHitDifference", "VXDTF: Total num of Hits - num of hits used for TC", 19, -0.5, 18.5);
  m_histoNumHitsIgnored->GetXaxis()->SetTitle("Difference in num of hits");
  m_histoNumHitsIgnored->GetYaxis()->SetTitle("count");

  m_histoNumTCsPerEvent = new TH1I("VXDTFNTCs", "VXDTF: Total num of TCs per event", 19, -0.5, 18.5);
  m_histoNumTCsPerEvent->GetXaxis()->SetTitle("num of TCs per event");
  m_histoNumTCsPerEvent->GetYaxis()->SetTitle("count");

  m_histoHitsIgnoredRatio = new TH1D("VXDTFNHitDifferenceRatio", "VXDTF: Num of hits used for TC / Total num of Hits", 100, 0, 1);
  m_histoHitsIgnoredRatio->GetXaxis()->SetTitle("Difference ratio -> 0 means bad, 1, good");
  m_histoHitsIgnoredRatio->GetYaxis()->SetTitle("count");

  m_histHitPosFoundU = new TH1D("HitPosFoundU", "Position (U) of hit at layer 5 of all TCs produced", 100, -5, 5);
  m_histHitPosTotalU = new TH1D("HitPosTotalU", "Position (U) of hit at layer 5 of all hits produced", 100, -5, 5);
  m_hitPosUsedVshitPosTotalU = new TGraphAsymmErrors(m_histHitPosFoundU, m_histHitPosTotalU);

  int minBinI = -1, maxBinI = 100;
  int nBinsI = abs(maxBinI - minBinI);
  m_histoNHitsAtPXD = new TH1I("VXDTFNHitsOfTCAtPXD", "VXDTF: Num of hits used for TC at the PXD", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtPXD->GetXaxis()->SetTitle("Num of Hits used for TC at the PXD");
  m_histoNHitsAtPXD->GetYaxis()->SetTitle("count");
  m_histoNHitsAtSVD = new TH1I("VXDTFNHitsOfTCAtSVD", "VXDTF: Num of hits used for TC at the SVD", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtSVD->GetXaxis()->SetTitle("Num of Hits used for TC at the SVD");
  m_histoNHitsAtSVD->GetYaxis()->SetTitle("count");
  m_histoNHitsAtTEL = new TH1I("VXDTFNHitsOfTCAtTEL", "VXDTF: Num of hits used for TC at the TEL", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtTEL->GetXaxis()->SetTitle("Num of Hits used for TC at the TEL");
  m_histoNHitsAtTEL->GetYaxis()->SetTitle("count");

  m_histoNHitsAtLayer0 = new TH1I("VXDTFNHitsOfTCAtLayer0", "VXDTF: Num of hits used for TC at Layer 0", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer0->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 0");
  m_histoNHitsAtLayer0->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer1 = new TH1I("VXDTFNHitsOfTCAtLayer1", "VXDTF: Num of hits used for TC at Layer 1", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer1->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 1");
  m_histoNHitsAtLayer1->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer2 = new TH1I("VXDTFNHitsOfTCAtLayer2", "VXDTF: Num of hits used for TC at Layer 2", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer2->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 2");
  m_histoNHitsAtLayer2->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer3 = new TH1I("VXDTFNHitsOfTCAtLayer3", "VXDTF: Num of hits used for TC at Layer 3", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer3->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 3");
  m_histoNHitsAtLayer3->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer4 = new TH1I("VXDTFNHitsOfTCAtLayer4", "VXDTF: Num of hits used for TC at Layer 4", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer4->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 4");
  m_histoNHitsAtLayer4->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer5 = new TH1I("VXDTFNHitsOfTCAtLayer5", "VXDTF: Num of hits used for TC at Layer 5", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer5->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 5");
  m_histoNHitsAtLayer5->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer6 = new TH1I("VXDTFNHitsOfTCAtLayer6", "VXDTF: Num of hits used for TC at Layer 6", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer6->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 6");
  m_histoNHitsAtLayer6->GetYaxis()->SetTitle("count");
  m_histoNHitsAtLayer7 = new TH1I("VXDTFNHitsOfTCAtLayer7", "VXDTF: Num of hits used for TC at Layer 7", nBinsI, minBinI, maxBinI);
  m_histoNHitsAtLayer7->GetXaxis()->SetTitle("Num of Hits used for TC at Layer 7");
  m_histoNHitsAtLayer7->GetYaxis()->SetTitle("count");


  vector<VxdID> buff = VXD::GeoCache::getInstance().getListOfSensors();
  vector<uint> sensorIDs;
  vector<uint> SVDsensorIDs;
  //cast the vxdid to uint
  for (int ibuff = 0; ibuff < (int)buff.size(); ibuff++) {
    B2DEBUG(1, "vxdid " << buff[ibuff] << std::endl);
    sensorIDs.push_back((uint)(buff[ibuff]));
    if (buff[ibuff].getLayerNumber() <= c_lastSVDPlane && buff[ibuff].getLayerNumber() >= c_firstSVDPlane)
      SVDsensorIDs.push_back((uint)(buff[ibuff]));
  }
  m_sensorIDs = sensorIDs;



  string headLine = "VXDTF: nHits found by TF divided by total nHits ";
  string title = "VXDTFnTCHitsDivnTotHitsatSensor";
  int nBins = 100, minBin = -2, maxBin = 18;

  // for each sensor the histoMaps get filled with an entry
  for (uint aSensorID : m_sensorIDs) {
    m_histoTCvsTotalHitsForEachSensor[aSensorID];
    m_hitMapHitsTotal[aSensorID];
    m_hitMapTCsOnly[aSensorID];
    m_hitMapReferenceTCs[aSensorID];

    VxdID thisID = aSensorID;
    m_countTCHitsForEachSensor.insert({thisID, 0});
    m_countTotalHitsForEachSensor.insert({thisID, 0});
  }
  for (uint aSensorID : SVDsensorIDs) {
    m_hitMapHitsTotalSVDOnlyU[aSensorID];
    m_hitMapTCsOnlySVDOnlyU[aSensorID];
    m_hitMapReferenceTCsSVDOnlyU[aSensorID];
    m_hitMapHitsTotalSVDOnlyV[aSensorID];
    m_hitMapTCsOnlySVDOnlyV[aSensorID];
    m_hitMapReferenceTCsSVDOnlyV[aSensorID];
  }

  string currentTitle;
  string currentHeadLine;
  for (auto& anEntry : m_histoTCvsTotalHitsForEachSensor) {

    VxdID thisID = anEntry.first;
    stringstream titleStream, headStream;
    titleStream << title << thisID;
    headStream << headLine << "at sensor " << thisID;
    currentTitle = titleStream.str();
    currentHeadLine = headStream.str();

    TH1F* currentHisto = new TH1F(currentTitle.c_str(), currentHeadLine.c_str(), nBins, minBin, maxBin);
    currentHisto->GetXaxis()->SetTitle(headLine.c_str());
    currentHisto->GetYaxis()->SetTitle("count");

    anEntry.second = currentHisto;
  }

  // doing the hitMaps:
  headLine = "VXDTF: hitMapTotal U and V for ";
  title = "hitMapTotal4Sensor";
  Prepare2DHistMap(headLine, title, m_hitMapHitsTotal);

  headLine = "VXDTF: hitMapTCOnly U and V for ";
  title = "hitMapTCOnly4Sensor";
  Prepare2DHistMap(headLine, title, m_hitMapTCsOnly);

  headLine = "VXDTF: hitMapReferenceTCs U and V for ";
  title = "hitMapReferenceTCs4Sensor";
  Prepare2DHistMap(headLine, title, m_hitMapReferenceTCs);

  headLine = "VXDTF: hitMapTotal for SVD U for ";
  title = "hitMapTotalSVDOnlyU";
  Prepare1DHistMap(headLine, title, m_hitMapHitsTotalSVDOnlyU, true);

  headLine = "VXDTF: hitMapTotal for SVD V for ";
  title = "hitMapTotalSVDOnlyV";
  Prepare1DHistMap(headLine, title, m_hitMapHitsTotalSVDOnlyV, false);

  headLine = "VXDTF: hitMapTCOnly for SVD U for ";
  title = "hitMapTCsOnlySVDOnlyU";
  Prepare1DHistMap(headLine, title, m_hitMapTCsOnlySVDOnlyU, true);

  headLine = "VXDTF: hitMapTCOnly for SVD V for ";
  title = "hitMapTCsOnlySVDOnlyV";
  Prepare1DHistMap(headLine, title, m_hitMapTCsOnlySVDOnlyV, false);

  headLine = "VXDTF: hitMapReferenceTCs for SVD U for ";
  title = "hitMapReferenceTCsSVDOnlyU";
  Prepare1DHistMap(headLine, title, m_hitMapReferenceTCsSVDOnlyU, true);

  headLine = "VXDTF: hitMapReferenceTCs for SVD V for ";
  title = "hitMapReferenceTCsSVDOnlyV";
  Prepare1DHistMap(headLine, title, m_hitMapReferenceTCsSVDOnlyV, false);


  oldDir->cd();
}


void VXDTFDQMModule::initialize()
{
  B2DEBUG(1, "start initialize");
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
  B2DEBUG(1, "end REG_HISTOGRAM");

  //Register collections
  StoreArray<SVDCluster> SVDCluster;  SVDCluster.isRequired();
//   StoreArray<SVDCluster> svdClusters(m_storeClustersName);
  StoreArray<genfit::TrackCand> TrackCandName(m_storeTrackCandsName);    TrackCandName.isRequired();
  StoreArray<genfit::TrackCand> TrackCandRefCol(m_storeReferenceTCsColName);  TrackCandRefCol.isOptional();
//   StoreArray<genfit::TrackCand> caTrackCandidates(m_storeTrackCandsName);
  B2DEBUG(1, "end get StoreArrays");

  //Store names to speed up creation later
//   m_storeClustersName = svdClusters.getName();
//   m_storeTrackCandsName = caTrackCandidates.getName();

  m_countNoSensorFound = 0;
  m_countAllHits = 0;
  m_badSensorIDs.clear();
  B2DEBUG(1, "end clear");
}

void VXDTFDQMModule::beginRun()
{
  B2DEBUG(1, "start beginRun");
  // Just to make sure, reset all the histograms.
  m_histoMomentum->Reset();
  m_histoPT->Reset();
  m_histoMomentumX->Reset();
  m_histoMomentumY->Reset();
  m_histoMomentumZ->Reset();
  m_histoNumHitsUsed->Reset();
  m_histoNumHitsIgnored->Reset();
  m_histoNumTCsPerEvent->Reset();
  m_histoHitsIgnoredRatio->Reset();
  m_histHitPosFoundU->Reset();
  m_histHitPosTotalU->Reset();

  m_histoNHitsAtPXD->Reset();
  m_histoNHitsAtSVD->Reset();
  m_histoNHitsAtTEL->Reset();
  m_histoNHitsAtLayer0->Reset();
  m_histoNHitsAtLayer1->Reset();
  m_histoNHitsAtLayer2->Reset();
  m_histoNHitsAtLayer3->Reset();
  m_histoNHitsAtLayer4->Reset();
  m_histoNHitsAtLayer5->Reset();
  m_histoNHitsAtLayer6->Reset();
  m_histoNHitsAtLayer7->Reset();
//  m_hitPosUsedVshitPosTotalU->Reset();

  for (auto& anEntry : m_histoTCvsTotalHitsForEachSensor) {
    anEntry.second->Reset();
  }
  for (auto& anEntry : m_hitMapHitsTotal) {
    anEntry.second->Reset();
  }
  for (auto& anEntry : m_hitMapTCsOnly) {
    anEntry.second->Reset();
  }
  for (auto& anEntry : m_hitMapReferenceTCs) {
    anEntry.second->Reset();
  }
}


void VXDTFDQMModule::event()
{
  B2DEBUG(1, "<<<<<<<<<<<<<<<<<<<<< VXDTFDQMModule::event()...");
  resetCounters();
  const StoreArray<genfit::TrackCand> caTrackCandidates(m_storeTrackCandsName);
  const StoreArray<PXDCluster> pxdClusters(m_storeSvdClustersName);
  const StoreArray<SVDCluster> svdClusters(m_storePxdClustersName);
  const StoreArray<TelCluster> telClusters(m_storeTelClustersName);
  const StoreArray<genfit::TrackCand> refTrackCandidates(m_storeReferenceTCsColName);


  B2INFO("2");
  // collecting info from TCs (estimated momentum, nHits, ..)
  int nCaTCs = caTrackCandidates.getEntries();
//   int nRefTCs = refTrackCandidates.getEntries();
  int nSVDClusters = svdClusters.getEntries();
  vector<int> allHitIndicesUsed, tempIndices, detectorIDs, hitIDsOfCurrentDetector;
  vector<uint> countedHitsInLayers { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // there are 7 layers so far, position in vector = layerNumber
  std::map<VxdID, uint> countedHitsInSensorsTC;
  std::map<VxdID, uint> countedHitsInSensors;
  TVector3 momentum;
  uint nPXDhits = 0, nSVDhits = 0, nTELhits = 0; // count hits of detectorType used for TCs
  for (const auto aTrackCand : caTrackCandidates) {
    B2INFO("3");
    int nHits = aTrackCand.getNHits();
    tempIndices = aTrackCand.getHitIDs();
    allHitIndicesUsed.insert(allHitIndicesUsed.end(), tempIndices.begin(), tempIndices.end());
    TVectorD stateSeed = aTrackCand.getStateSeed();
    momentum[0] = stateSeed(3); momentum[1] = stateSeed(4); momentum[2] = stateSeed(5);
    double fullMomentum = momentum.Mag();
    double traverseMomentum = momentum.Perp();
    double momentumX = momentum.X();
    double momentumY = momentum.Y();
    double momentumZ = momentum.Z();

    m_histoMomentum->Fill(fullMomentum);
    m_histoPT->Fill(traverseMomentum);
    m_histoMomentumX->Fill(momentumX);
    m_histoMomentumY->Fill(momentumY);
    m_histoMomentumZ->Fill(momentumZ);
    m_histoNumHitsUsed->Fill(nHits);
    m_histoNumHitsIgnored->Fill(nSVDClusters - nHits);
    m_histoHitsIgnoredRatio->Fill(double(nHits) / double(nSVDClusters));

    detectorIDs = aTrackCand.getDetIDs(); // detectorIDs of all Hits
    VxdID aVxdID;
    for (auto detectorID : detectorIDs) {
      B2INFO("4");
      hitIDsOfCurrentDetector = aTrackCand.getHitIDs(detectorID); //Get hit ids of from a specific detector.

      for (auto hitID : hitIDsOfCurrentDetector) {
        B2INFO("5");
        if (detectorID == Const::PXD) {
          ++nPXDhits;
          aVxdID = pxdClusters[hitID]->getSensorID();
          countedHitsInLayers.at(aVxdID.getLayerNumber()) += 1;

          B2DEBUG(1, "getSensorinfo 1");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          m_hitMapTCsOnly.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(pxdClusters[hitID]->getU()),
                                                           aSensorInfo->getVCellID(pxdClusters[hitID]->getV()));
        } else if (detectorID == Const::SVD) {
          ++nSVDhits;
          aVxdID = svdClusters[hitID]->getSensorID();
          countedHitsInLayers.at(aVxdID.getLayerNumber()) += 1;

          B2DEBUG(1, "getSensorinfo 2");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          B2DEBUG(1, "end getSensorinfo 2");

          bool isU = svdClusters[hitID]->isUCluster();
          if (isU) {
            m_hitMapTCsOnlySVDOnlyU.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(svdClusters[hitID]->getPosition()));
          } else {
            m_hitMapTCsOnlySVDOnlyV.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(svdClusters[hitID]->getPosition()));
          }
        } else { // hopefully telescope clusters
          ++nTELhits;
          aVxdID = telClusters[hitID]->getSensorID();
          countedHitsInLayers.at(aVxdID.getLayerNumber()) += 1;

          B2DEBUG(1, "getSensorinfo 3");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          m_hitMapTCsOnly.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(telClusters[hitID]->getU()),
                                                           aSensorInfo->getVCellID(telClusters[hitID]->getV()));
        }

        if (m_countTCHitsForEachSensor.find(aVxdID) != m_countTCHitsForEachSensor.end()) {
          m_countTCHitsForEachSensor[aVxdID]++;
        }
      }
    }
  }
  B2INFO("6");
  m_histoNHitsAtPXD->Fill(nPXDhits);
  m_histoNHitsAtSVD->Fill(nSVDhits);
  m_histoNHitsAtTEL->Fill(nTELhits);
  m_histoNHitsAtLayer0->Fill(countedHitsInLayers.at(0));
  m_histoNHitsAtLayer1->Fill(countedHitsInLayers.at(1));
  m_histoNHitsAtLayer2->Fill(countedHitsInLayers.at(2));
  m_histoNHitsAtLayer3->Fill(countedHitsInLayers.at(3));
  m_histoNHitsAtLayer4->Fill(countedHitsInLayers.at(4));
  m_histoNHitsAtLayer5->Fill(countedHitsInLayers.at(5));
  m_histoNHitsAtLayer6->Fill(countedHitsInLayers.at(6));
  m_histoNHitsAtLayer7->Fill(countedHitsInLayers.at(7));

  if (nCaTCs == 0) { // in this case, the for-loop was not executed
    m_histoNumHitsUsed->Fill(0);
    m_histoNumHitsIgnored->Fill(nSVDClusters);
    m_histoHitsIgnoredRatio->Fill(0);
  }
  m_histoNumTCsPerEvent->Fill(nCaTCs);

  for (const auto aTrackCand : refTrackCandidates) {
    tempIndices = aTrackCand.getHitIDs();
    allHitIndicesUsed.insert(allHitIndicesUsed.end(), tempIndices.begin(), tempIndices.end());



    detectorIDs = aTrackCand.getDetIDs(); // detectorIDs of all Hits
    VxdID aVxdID;
    for (auto detectorID : detectorIDs) {
      hitIDsOfCurrentDetector = aTrackCand.getHitIDs(detectorID); //Get hit ids of from a specific detector.

      for (auto hitID : hitIDsOfCurrentDetector) {
        if (detectorID == Const::PXD) {
          aVxdID = pxdClusters[hitID]->getSensorID();

          B2DEBUG(1, "getSensorinfo 4");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          m_hitMapReferenceTCs.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(pxdClusters[hitID]->getU()),
                                                                aSensorInfo->getVCellID(pxdClusters[hitID]->getV()));
        } else if (detectorID == Const::SVD) {
          aVxdID = svdClusters[hitID]->getSensorID();

          B2DEBUG(1, "getSensorinfo 5");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          B2DEBUG(1, "end getSensorinfo 5");

          bool isU = svdClusters[hitID]->isUCluster();
          if (isU) {
            m_hitMapReferenceTCsSVDOnlyU.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(svdClusters[hitID]->getPosition()));
          } else {
            m_hitMapReferenceTCsSVDOnlyV.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(svdClusters[hitID]->getPosition()));
          }
        } else { // hopefully telescope clusters
          aVxdID = telClusters[hitID]->getSensorID();

          B2DEBUG(1, "getSensorinfo 6");
          const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
          m_hitMapReferenceTCs.find(uint(aVxdID))->second->Fill(aSensorInfo->getUCellID(telClusters[hitID]->getU()),
                                                                aSensorInfo->getVCellID(telClusters[hitID]->getV()));
        }
      }
    }
  }
  B2INFO("7");
  // collecting Info from Clusters
  for (const SVDCluster& aCluster : svdClusters) {
    VxdID aVxdID = aCluster.getSensorID();
    if (aCluster.isUCluster()) {
      m_histHitPosTotalU->Fill(aCluster.getPosition());


      for (int index : allHitIndicesUsed) {
        if (index == aCluster.getArrayIndex()) {
          m_histHitPosFoundU->Fill(aCluster.getPosition());
          break;
        }
      }
    }

    if (m_countTotalHitsForEachSensor.find(aVxdID) != m_countTotalHitsForEachSensor.end()) {
      m_countTotalHitsForEachSensor[aVxdID]++;
    } else {
      m_countNoSensorFound++;
      B2DEBUG(10, "svdClusters: vxdID " << VxdID(aVxdID) << " with layerNumber " << VxdID(aVxdID).getLayerNumber() << " not found...");
      m_badSensorIDs.push_back(aVxdID);
    }
    B2INFO("ClusterCharge: " << aCluster.getCharge());
    ++m_countAllHits;
  }

  for (const PXDCluster& aCluster : pxdClusters) {
    VxdID aVxdID = aCluster.getSensorID();
    uint vxdInt = aVxdID;

    if (m_countTotalHitsForEachSensor.find(aVxdID) != m_countTotalHitsForEachSensor.end()) {
      m_countTotalHitsForEachSensor[aVxdID]++;
    } else {
      m_countNoSensorFound++;
      B2DEBUG(10, "pxdClusters: vxdID " << VxdID(aVxdID) << " not found...");
      m_badSensorIDs.push_back(aVxdID);
    }
    ++m_countAllHits;

    const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
    m_hitMapHitsTotal.find(vxdInt)->second->Fill(aSensorInfo->getUCellID(aCluster.getU()), aSensorInfo->getVCellID(aCluster.getV()));
  }

  for (const TelCluster& aCluster : telClusters) {
    VxdID aVxdID = aCluster.getSensorID();
    uint vxdInt = aVxdID;

    if (m_countTotalHitsForEachSensor.find(aVxdID) != m_countTotalHitsForEachSensor.end()) {
      m_countTotalHitsForEachSensor[aVxdID]++;
    } else {
      m_countNoSensorFound++;
      B2DEBUG(10, "telClusters: vxdID " << VxdID(aVxdID) << " not found...");
      m_badSensorIDs.push_back(aVxdID);
    }
    ++m_countAllHits;

    const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVxdID);
    m_hitMapHitsTotal.find(vxdInt)->second->Fill(aSensorInfo->getUCellID(aCluster.getU()), aSensorInfo->getVCellID(aCluster.getV()));
  }

  for (auto& anEntry : m_histoTCvsTotalHitsForEachSensor) {
    VxdID thisID = anEntry.first;
    float result = -1, nTotalHits = 0, nTCHits = 0;
    if (m_countTotalHitsForEachSensor.find(thisID) != m_countTotalHitsForEachSensor.end()) {
      nTotalHits = float(m_countTotalHitsForEachSensor[thisID]);
    }
    if (m_countTCHitsForEachSensor.find(thisID) != m_countTCHitsForEachSensor.end()) {
      nTCHits = float(m_countTCHitsForEachSensor[thisID]);
    }
    if (nTotalHits != 0) {
      result = 0;
      if (nTCHits != 0) {
        result = nTCHits / nTotalHits;
      }
    }
    if (result > 1) {  // in this case, hits were used more than once for TCs, therefore high number possible
      result = 1 + 0.1 * result; // original 1-10 -> 1-2 -> new 10 = original 90
      if (result > 10) {  // original 90-1000 -> 11-20
        result = 10 + 0.1 * result;
      }
    }
    anEntry.second->Fill(result);
  }
}


void VXDTFDQMModule::endRun()
{
  std::sort(m_badSensorIDs.begin(), m_badSensorIDs.end());
  int before = m_badSensorIDs.size();
  vector<uint> notCleaned = m_badSensorIDs;
  auto newEndOfVector = std::unique(m_badSensorIDs.begin(), m_badSensorIDs.end());
  m_badSensorIDs.resize(std::distance(m_badSensorIDs.begin(), newEndOfVector));

  stringstream vxdIDOutput;
  for (uint iD : m_badSensorIDs) {
    VxdID vID = iD;
    vxdIDOutput << iD << "/" << vID << " ";
  }
  vxdIDOutput << "\n";
  for (uint iD : notCleaned) {
    vxdIDOutput << iD << "/" << VxdID(iD) << " ";
  }
  B2WARNING("VXDTFDQMModule: there were " << m_countNoSensorFound << " of " << m_countAllHits <<
            " cases where sensorID of Hit could not be found in sensorID-specific histograms. Total number of bad sensors before/after unique: "
            << before << "/" << m_badSensorIDs.size() << ", which were:\n" << vxdIDOutput.str());
}


void VXDTFDQMModule::terminate()
{
}



void VXDTFDQMModule::Prepare2DHistMap(std::string headLine, std::string title, std::map<uint, TH2F*>& thisMap)
{
  for (auto& anEntry : thisMap) {
    VxdID thisID = anEntry.first;
    stringstream titleStream, headStream;
    titleStream << title << thisID;
    headStream << headLine << "at sensor " << thisID;
    string currentTitle = titleStream.str();
    string currentHeadLine = headStream.str();

    B2DEBUG(1, "getSensorinfo 9");
    const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(thisID);
    B2DEBUG(1, "end getSensorinfo 9");

    int nPixelsU = aSensorInfo->getUCells();
    int nPixelsV = aSensorInfo->getVCells();

    anEntry.second = new TH2F(currentTitle.c_str(), currentHeadLine.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
    anEntry.second->GetXaxis()->SetTitle("u position [pitch units]");
    anEntry.second->GetYaxis()->SetTitle("v position [pitch units]");
    anEntry.second->GetZaxis()->SetTitle("hits");
  }
}



void VXDTFDQMModule::Prepare1DHistMap(std::string headLine, std::string title, std::map<uint, TH1F*>& thisMap, bool isU)
{
  for (auto& anEntry : thisMap) {
    VxdID thisID = anEntry.first;
    stringstream titleStream, headStream;
    titleStream << title << thisID;
    headStream << headLine << "at sensor " << thisID;
    string currentTitle = titleStream.str();
    string currentHeadLine = headStream.str();

    B2DEBUG(1, "getSensorinfo 10");
    const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(thisID);
    B2DEBUG(1, "end getSensorinfo 10");
    int nPixels = aSensorInfo->getVCells();
    string axisLabel = "v position [pitch units]";
    if (isU) {
      nPixels = aSensorInfo->getUCells();
      axisLabel = "u position [pitch units]";
    }

    anEntry.second = new TH1F(currentTitle.c_str(), currentHeadLine.c_str(), nPixels, 0, nPixels);
    anEntry.second->GetXaxis()->SetTitle(axisLabel.c_str());
    anEntry.second->GetYaxis()->SetTitle("hits");
  }
}
