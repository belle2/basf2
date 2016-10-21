/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/


//fw-stuff
#include "tracking/modules/VXDTFHelperTools/TFAnalizerModule.h"
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/gearbox/Const.h>


//C++-stuff
#include <iostream>
#include <iomanip> // for setprecision on cout-doubles
#include <string>
#include <functional>



//stl-stuff
#include <map>
#include <list>
#include <utility>

//root-stuff
#include <TMath.h>

//boost stuff
#include <boost/foreach.hpp>
#include <boost/math/special_functions/sign.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;
using boost::tuple;
using boost::math::sign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TFAnalizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TFAnalizerModule::TFAnalizerModule() : Module()
{
  InitializeVariables();

  vector<string> rootFileNameVals;
  rootFileNameVals.push_back("TFAnalizerResults");
  rootFileNameVals.push_back("RECREATE");

  //Set module properties
  setDescription("analyzes quality of cell-o-mat versus mcTrackFinder");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("fileExportMcTracks", m_PARAMFileExportMcTracks, "export mc Trackfinder tracks into file", bool(false));
  addParam("fileExportTfTracks", m_PARAMFileExportTfTracks, "export vxd Trackfinder tracks into file", bool(false));
  addParam("mcTCname", m_PARAMmcTCname, "special name for mcTF track candidates", string("mcTracks"));
  addParam("caTCname", m_PARAMcaTCname, "special name for caTF track candidates", string(""));
  addParam("acceptedTCname", m_PARAMacceptedTCname, "special name for accepted/successfully reconstructed track candidates",
           string("acceptedVXDTFTracks"));
  addParam("lostTCname", m_PARAMlostTCname, "special name for lost track candidates", string("lostTracks"));
  addParam("InfoBoardName", m_PARAMinfoBoardName, "Name of container used for data transfer from VXDTFModule", string(""));
  addParam("qiThreshold", m_PARAMqiThreshold,
           " chose value to filter TCs found by VXDTF. TCs having QIs lower than this value won't be marked as reconstructed", double(0.7));
  addParam("minNumOfHitsThreshold", m_PARAMminNumOfHitsThreshold,
           " defines how many hits of current TC has to be found again to be accepted as recovered, standard is 3 hits", int(3));
  addParam("printExtentialAnalysisData", m_PARAMprintExtentialAnalysisData, "set true, if you want to cout special Info to the shell",
           bool(false));

  addParam("minTMomentumFilter", m_PARAMminTMomentumFilter,
           "to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum in GeV/c than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges - WARNING for some cases, this is a typical source for strange results!",
           double(0.));
  addParam("maxTMomentumFilter", m_PARAMmaxTMomentumFilter,
           "to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum in GeV/c than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges - WARNING for some cases, this is a typical source for strange results!",
           double(500.));
  addParam("writeToRoot", m_PARAMwriteToRoot,
           " if true, analysis data is stored to root file with file name chosen by 'rootFileName'", bool(true));
  addParam("rootFileName", m_PARAMrootFileName,
           " only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot'=true  ",
           rootFileNameVals);

  // Display
  addParam("collectorDisplayId", m_display,
           "Flag for Collector (collects information about trackFinder) Settings: 0 = deactivate Collector, 1 = activate for analysis, 2 = activate for display",
           int(0));

  addParam("collectorFilePath", m_collectorFilePath, "File Path for the Collector", string(""));

  addParam("collectorThreshold", m_collectorThreshold, "Threshold for contaminated TC", double(0.7));
}


TFAnalizerModule::~TFAnalizerModule()
{

}

void TFAnalizerModule::initialize()
{
  StoreArray<genfit::TrackCand>::required(m_PARAMmcTCname);
  StoreArray<genfit::TrackCand>::required(m_PARAMcaTCname);
  StoreArray<genfit::TrackCand>::registerPersistent(m_PARAMacceptedTCname);
  StoreArray<genfit::TrackCand>::registerPersistent(m_PARAMlostTCname);

  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::required();
  StoreArray<PXDTrueHit>::optional();
  StoreArray<SVDTrueHit>::required();
  StoreArray<VXDTFInfoBoard>::optional(
    m_PARAMinfoBoardName); /// WARNING TODO: implement a minimal analyzing mode which can deal with TF's without using the InfoBoards...

  if (m_PARAMwriteToRoot == true) {
    if ((m_PARAMrootFileName.size()) != 2) {
      string output;
      BOOST_FOREACH(string entry, m_PARAMrootFileName) {
        output += "'" + entry + "' ";
      }
      B2FATAL("TFAnalizer::initialize: rootFileName is set wrong, although parameter 'writeToRoot' is enabled! Actual entries are: " <<
              output);
    }
    m_PARAMrootFileName[0] += ".root";
    m_rootFilePtr = new TFile(m_PARAMrootFileName[0].c_str(), m_PARAMrootFileName[1].c_str()); // alternative: UPDATE
    m_treePtr = new TTree("m_treePtr", "aTree");

    m_treePtr->Branch("TotalPXresiduals", &m_rootTotalPXresiduals);
    m_treePtr->Branch("TotalPYresiduals", &m_rootTotalPYresiduals);
    m_treePtr->Branch("TotalPZresiduals", &m_rootTotalPZresiduals);

    m_treePtr->Branch("CleanPXresiduals", &m_rootCleanPXresiduals);
    m_treePtr->Branch("CleanPYresiduals", &m_rootCleanPYresiduals);
    m_treePtr->Branch("CleanPZresiduals", &m_rootCleanPZresiduals);

    m_treePtr->Branch("CompletePXresiduals", &m_rootCompletePXresiduals);
    m_treePtr->Branch("CompletePYresiduals", &m_rootCompletePYresiduals);
    m_treePtr->Branch("CompletePZresiduals", &m_rootCompletePZresiduals);

    m_treePtr->Branch("TotalMCMomValues", &m_rootTotalMCMomValues);
    m_treePtr->Branch("TotalCAMomValues", &m_rootTotalCAMomValues);
    m_treePtr->Branch("CleanCAMomValues", &m_rootCleanCAMomValues);
    m_treePtr->Branch("CompleteCAMomValues", &m_rootCompleteCAMomValues);
    m_treePtr->Branch("TotalMomValues", &m_rootTotalMomValues);

    m_treePtr->Branch("TotalCAMomResiduals", &m_rootTotalCAMomResiduals);
    m_treePtr->Branch("CleanCAMomResiduals", &m_rootCleanCAMomResiduals);
    m_treePtr->Branch("CompleteCAMomResiduals", &m_rootCompleteCAMomResiduals);

    m_treePtr->Branch("TotalMCpTValues", &m_rootTotalMCpTValues);
    m_treePtr->Branch("TotalCApTValues", &m_rootTotalCApTValues);
    m_treePtr->Branch("CleanCApTValues", &m_rootCleanCApTValues);
    m_treePtr->Branch("CompleteCApTValues", &m_rootCompleteCApTValues);
    m_treePtr->Branch("TotalpTValues", &m_rootTotalpTValues);

    m_treePtr->Branch("TotalCApTResiduals", &m_rootTotalCApTResiduals);
    m_treePtr->Branch("CleanCApTResiduals", &m_rootCleanCApTResiduals);
    m_treePtr->Branch("CompleteCApTResiduals", &m_rootCompleteCApTResiduals);

    m_treePtr->Branch("TotalMCThetaValues", &m_rootTotalMCThetaValues);
    m_treePtr->Branch("TotalCAThetaValues", &m_rootTotalCAThetaValues);
    m_treePtr->Branch("CleanCAThetaValues", &m_rootCleanCAThetaValues);
    m_treePtr->Branch("CompleteCAThetaValues", &m_rootCompleteCAThetaValues);
    m_treePtr->Branch("TotalThetaValues", &m_rootTotalThetaValues);

    m_treePtr->Branch("TotalCAThetaResiduals", &m_rootTotalCAThetaResiduals);
    m_treePtr->Branch("CleanCAThetaResiduals", &m_rootCleanCAThetaResiduals);
    m_treePtr->Branch("CompleteCAThetaResiduals", &m_rootCompleteCAThetaResiduals);

    m_treePtr->Branch("TotalMCPhiValues", &m_rootTotalMCPhiValues);
    m_treePtr->Branch("TotalCAPhiValues", &m_rootTotalCAPhiValues);
    m_treePtr->Branch("CleanCAPhiValues", &m_rootCleanCAPhiValues);
    m_treePtr->Branch("CompleteCAPhiValues", &m_rootCompleteCAPhiValues);
    m_treePtr->Branch("TotalPhiValues", &m_rootTotalPhiValues);

    m_treePtr->Branch("TotalMCVertex2IP3DValues", &m_rootTotalMCVertex2IP3DValues);
    m_treePtr->Branch("TotalCAVertex2IP3DValues", &m_rootTotalCAVertex2IP3DValues);
    m_treePtr->Branch("CleanCAVertex2IP3DValues", &m_rootCleanCAVertex2IP3DValues);
    m_treePtr->Branch("CompleteCAVertex2IP3DValues", &m_rootCompleteCAVertex2IP3DValues);
    m_treePtr->Branch("TotalVertex2IP3DValues", &m_rootTotalVertex2IP3DValues);

    m_treePtr->Branch("TotalMCVertex2IPXYValues", &m_rootTotalMCVertex2IPXYValues);
    m_treePtr->Branch("TotalCAVertex2IPXYValues", &m_rootTotalCAVertex2IPXYValues);
    m_treePtr->Branch("CleanCAVertex2IPXYValues", &m_rootCleanCAVertex2IPXYValues);
    m_treePtr->Branch("CompleteCAVertex2IPXYValues", &m_rootCompleteCAVertex2IPXYValues);
    m_treePtr->Branch("TotalVertex2IPXYValues", &m_rootTotalVertex2IPXYValues);

    m_treePtr->Branch("TotalMCVertex2IPZValues", &m_rootTotalMCVertex2IPZValues);
    m_treePtr->Branch("TotalCAVertex2IPZValues", &m_rootTotalCAVertex2IPZValues);
    m_treePtr->Branch("CleanCAVertex2IPZValues", &m_rootCleanCAVertex2IPZValues);
    m_treePtr->Branch("CompleteCAVertex2IPZValues", &m_rootCompleteCAVertex2IPZValues);
    m_treePtr->Branch("TotalVertex2IPZValues", &m_rootTotalVertex2IPZValues);

    m_treePtr->Branch("TotalCAThetaResiduals", &m_rootTotalCAThetaResiduals);
    m_treePtr->Branch("CleanCAThetaResiduals", &m_rootCleanCAThetaResiduals);
    m_treePtr->Branch("CompleteCAThetaResiduals", &m_rootCompleteCAThetaResiduals);

    m_treePtr->Branch("TotalCAMomResidualsAngles", &m_rootTotalCAMomResidualsAngles);
    m_treePtr->Branch("CleanCAMomResidualsAngles", &m_rootCleanCAMomResidualsAngles);
    m_treePtr->Branch("CompleteCAMomResidualsAngles", &m_rootCompleteCAMomResidualsAngles);

    m_treePtr->Branch("TotalCApTResidualsAngles", &m_rootTotalCApTResidualsAngles);
    m_treePtr->Branch("CleanCApTResidualsAngles", &m_rootCleanCApTResidualsAngles);
    m_treePtr->Branch("CompleteCApTResidualsAngles", &m_rootCompleteCApTResidualsAngles);

    m_treePtr->Branch("TotalCASeedPositionResiduals", &m_rootTotalCASeedPositionResiduals);
    m_treePtr->Branch("CleanCASeedPositionResiduals", &m_rootCleanCASeedPositionResiduals);
    m_treePtr->Branch("CompleteCASeedPositionResiduals", &m_rootCompleteCASeedPositionResiduals);

    m_treePtr->Branch("MCreconstructedTrackLength", &m_rootMCreconstructedTrackLength);
    m_treePtr->Branch("CAreconstructedTrackLength", &m_rootCAreconstructedTrackLength);

    m_treePtr->Branch("LostUClusters", &m_rootLostUClusters);
    m_treePtr->Branch("LostVClusters", &m_rootLostVClusters);
    m_treePtr->Branch("TotalMcUClusters", &m_rootTotalMcUClusters);
    m_treePtr->Branch("TotalMcVClusters", &m_rootTotalMcVClusters);

    m_treePtr->Branch("LostUClusterEDep", &m_rootLostUClusterEDep);
    m_treePtr->Branch("LostVClusterEDep", &m_rootLostVClusterEDep);
    m_treePtr->Branch("TotalMcUClusterEDep", &m_rootTotalMcUClusterEDep);
    m_treePtr->Branch("TotalMcVClusterEDep", &m_rootTotalMcVClusterEDep);

  } else {
    m_rootFilePtr = NULL;
    m_treePtr = NULL;
  }
}


void TFAnalizerModule::beginRun()
{

}


void TFAnalizerModule::event()
{


  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(1, "################## entering TFAnalizer - event " << m_eventCounter << " ######################");

  RootVariables rootVariables; // storing all root related infos
  m_forRootCountFoundIDs.clear(); // reset foundIDs-counter

  /// import all genfit::TrackCands (McFinder, TFinder)
  StoreArray<genfit::TrackCand> mcTrackCandidates(m_PARAMmcTCname);
  StoreArray<genfit::TrackCand> caTrackCandidates(m_PARAMcaTCname);
  // preparing storearray for trackCandidates and fitted tracks
  StoreArray<genfit::TrackCand> acceptedTrackCandidates(m_PARAMacceptedTCname);

  StoreArray<genfit::TrackCand> lostTrackCandidates(m_PARAMlostTCname);


  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  StoreArray<PXDTrueHit> pxdTrueHits;
  StoreArray<SVDTrueHit> svdTrueHits;

  RelationIndex<PXDCluster, PXDTrueHit> relPXDCluster2TrueHit; /* <FROM, TO> */
  RelationIndex<SVDCluster, SVDTrueHit> relSVDCluster2TrueHit;

  StoreArray<VXDTFInfoBoard> extraInfos(m_PARAMinfoBoardName);

  StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");

  int numOfMcTCs = mcTrackCandidates.getEntries();
  int numOfCaTCs = caTrackCandidates.getEntries();
  //int numOfInfoBoards = extraInfos.getEntries();

  m_mcTrackCounter += numOfMcTCs;
  m_caTrackCounter += numOfCaTCs;

  vector<VXDTrackCandidate> caTcVector;
  vector<VXDTrackCandidate> mcTcVector;


  /// get info needed for comparison and coord-export:
  B2DEBUG(1, "importing " << numOfMcTCs << " mcTrackCandidates...");

  for (int i = 0; i not_eq numOfMcTCs; ++i) {
    B2DEBUG(10, "--importing trackCandidate " << i << "...");
    genfit::TrackCand* aTC =  mcTrackCandidates[i];
    extractHits(aTC, relPXDCluster2TrueHit, relSVDCluster2TrueHit, pxdClusters, svdClusters, extraInfos, tfcandTFInfo, mcTcVector, true,
                i);    /// extractHits
    /// missing: export2File!
  }
  m_mcTrackVectorCounter += mcTcVector.size();


  B2DEBUG(1, "importing " << numOfCaTCs << " caTrackCandidates...");
  for (int i = 0; i not_eq numOfCaTCs; ++i) {
    B2DEBUG(10, "--importing trackCandidate " << i << "...");
    genfit::TrackCand* aTC =  caTrackCandidates[i];
    extractHits(aTC, relPXDCluster2TrueHit, relSVDCluster2TrueHit, pxdClusters, svdClusters, extraInfos,  tfcandTFInfo, caTcVector,
                false, i); /// extractHits
  }

  B2DEBUG(1, " before checking compatibility: there are " << mcTcVector.size() << "/" << caTcVector.size() << " mc/ca-tcs");
  for (VXDTrackCandidate& mcTC : mcTcVector) {
    for (VXDTrackCandidate& caTC : caTcVector) {
      B2DEBUG(10, "-before checkCompatibility: caTC.indexNumber: " << caTC.indexNumber << ", caTC.qualityIndex: " << caTC.qualityIndex);
      B2DEBUG(10, "-run checkCompatibility for mcTC " << mcTC.indexNumber << " using caTC " << caTC.indexNumber);
      checkCompatibility(mcTC, caTC); /// checkCompatibility
      B2DEBUG(10, "-after checkCompatibility: caTC.indexNumber: " << caTC.indexNumber << ", caTC.qualityIndex: " << caTC.qualityIndex);
    }
  } // determine compatibility matrix

  list<foundIDentry> foundIDs; // .first: id of mcTC assigned, .second: qi of tc, the higher, the better
  int countedDoubleEntries = 0; // counts IDs which were found more than once

  if (int(caTcVector.size()) != 0) {   // ! caTcVector.empty()
    B2DEBUG(1, " between loops: caTcVector.size():" << caTcVector.size() << ", caTcVector[0].indexNumber: " << caTcVector[0].indexNumber
            << ", finAssID: " << caTcVector[0].finalAssignedID << ", QI: " << caTcVector[0].qualityIndex);
  }
  for (VXDTrackCandidate& caTC : caTcVector) {

    B2DEBUG(10, " caTC " << caTC.indexNumber << ": has got the following assigned mc trackCandidates: (best value: mcTCID: " <<
            caTC.finalAssignedID << ", QI: " << caTC.qualityIndex << ")");
    for (CompatibilityIndex& thisEntry : caTC.compatiblePartners) {
      B2DEBUG(10, "	Partner: " << boost::get<0>(thisEntry) << ", shares " << boost::get<1>(thisEntry) << " hits, thisTC has got " <<
              boost::get<2>(thisEntry) << " dirty hits, " << boost::get<3>(thisEntry) <<
              " hits are only in partner and they have a qualityRelation of " << boost::get<4>(thisEntry));
    }

    if (caTC.finalAssignedID == -1) {  caTrackCandidates[caTC.indexNumber]->setMcTrackId(-1); continue; }  // in this case, absolutely no hit of caTC is of any mcTC
    if (caTC.numOfCorrectlyAssignedHits < m_PARAMminNumOfHitsThreshold) {
      caTrackCandidates[caTC.indexNumber]->setMcTrackId(-1);
      printInfo(0, mcTcVector[caTC.finalAssignedID], caTC, rootVariables);
      continue;
    }

    // adding MCiD-information to genfit::TrackCand (needed for trackFitChecker) and storing correctly recognized TC into new storearray:
    caTrackCandidates[caTC.indexNumber]->setMcTrackId(mcTrackCandidates[mcTcVector[caTC.finalAssignedID].indexNumber]->getMcTrackId());
    acceptedTrackCandidates.appendNew(*caTrackCandidates[caTC.indexNumber]);

    int totalHits = caTC.numOfCorrectlyAssignedHits + caTC.numOfBadAssignedHits;
    double trackQuality = double(caTC.numOfCorrectlyAssignedHits) / double(totalHits);
    if (trackQuality < m_PARAMqiThreshold) {
      printInfo(0, mcTcVector[caTC.finalAssignedID], caTC, rootVariables);
      continue;
    }


//     typedef pair <int, double> idEntry;

    for (foundIDentry iD : foundIDs) { if (iD.first == caTC.finalAssignedID && iD.second > trackQuality) { countedDoubleEntries++; continue; } } // no output here since it has been recovered several times, we take the best caTC


    foundIDs.push_back(make_pair(caTC.finalAssignedID, trackQuality));
    if (caTC.qualityIndex > 0.99) { // harder method: caTC.qualityIndex which means totally reconstructed
      printInfo(2, mcTcVector[caTC.finalAssignedID], caTC, rootVariables);
      m_countedPerfectRecoveries++;
    } else if (trackQuality > 0.99) { // no foreign hits but not all hits reconstructed
      printInfo(3, mcTcVector[caTC.finalAssignedID], caTC, rootVariables);
      m_countedCleanRecoveries++;
    } else {
      printInfo(1, mcTcVector[caTC.finalAssignedID], caTC, rootVariables);
    }
  }
  m_countedDoubleEntries += countedDoubleEntries;

  foundIDs.sort(isFirstValueBigger);
  foundIDs.unique(isFirstValueTheSame);
  /// old version using a vector:
//   vector<int>::iterator newEndOfVector;
//   /// WARNING: std:unique does delete double entries but does NOT resize the vector! This means that for every removed element, at the end of the vector remains one random value stored
//   std::sort(foundIDs.begin(), foundIDs.end());
//   newEndOfVector = std::unique(foundIDs.begin(), foundIDs.end());
//   foundIDs.resize(std::distance(foundIDs.begin(), newEndOfVector));
  int numOfFoundIDs = foundIDs.size(), mcPXDHits = 0, mcSVDHits = 0, caPXDHits = 0,
      caSVDHits = 0; // xHits counts number of hits per detectortype used by current tc
  m_countReconstructedTCs += numOfFoundIDs;

  for (VXDTrackCandidate& mcTC : mcTcVector) {
    bool foundFlag = false;
    for (foundIDentry iD : foundIDs) { if (iD.first == mcTC.indexNumber) { foundFlag = true; } }
    m_totalRealHits += int(mcTC.coordinates.size() * 0.5);
    if (foundFlag == true) {
      if (m_PARAMprintExtentialAnalysisData == true or m_PARAMwriteToRoot == true) { printMC(true, mcTC, rootVariables); }  /// printMC
      m_totalRealHits += int(mcTC.coordinates.size() * 0.5);
    } else {
      if (m_PARAMprintExtentialAnalysisData == true or m_PARAMwriteToRoot == true) { printMC(false, mcTC, rootVariables); }  /// printMC

      lostTrackCandidates.appendNew(*mcTrackCandidates[mcTC.indexNumber]);
    }
    mcPXDHits += mcTC.pxdClusterIDs.size();
    mcSVDHits += mcTC.svdClusterIDs.size();
  } // print info about all found and lost mcTCs

  for (VXDTrackCandidate& caTC : caTcVector) {
    if (caTC.finalAssignedID == -1 || caTC.qualityIndex < m_PARAMqiThreshold) {
      if (m_PARAMprintExtentialAnalysisData == true) { printCA(false, caTC); } /// printCA
    } else {
      if (m_PARAMprintExtentialAnalysisData == true) { printCA(true, caTC); } /// printCA
    }
    caPXDHits += caTC.pxdClusterIDs.size();
    caSVDHits += caTC.svdClusterIDs.size();
  } // print info about all ghost and good caTCs

  m_nMcPXDHits += mcPXDHits;
  m_nMcSVDHits += mcSVDHits;
  m_nCaPXDHits += caPXDHits;
  m_nCaSVDHits += caSVDHits;


  B2DEBUG(1, "Event " << m_eventCounter << ": There are " << int(mcTcVector.size()) << " mcTCs, with mean of " << (float(
            mcPXDHits) / float(mcTcVector.size())) << "/" << (float(mcSVDHits) / float(mcTcVector.size())) << " PXD/SVD clusters");
  for (foundIDentry ID : foundIDs) {
    B2DEBUG(1, " - ID " << ID.first << " recovered");
  }
  B2DEBUG(1, "Event " << m_eventCounter << ": There are " << int(caTcVector.size()) << " caTCs, with mean of " << (float(
            caPXDHits) / float(caTcVector.size())) << "/" << (float(caSVDHits) / float(caTcVector.size())) << " PXD/SVD clusters");
  int acceptedTCs = acceptedTrackCandidates.getEntries();
  m_countAcceptedGFTCs += acceptedTCs;
  int lostTCs = lostTrackCandidates.getEntries();
  m_lostGFTCs += lostTCs;
  B2DEBUG(1, " of " << numOfCaTCs << " TCs produced by the tested TrackFinder, " << acceptedTCs <<
          " were recognized safely and stored into the container of accepted TCs, " << lostTCs <<
          " were lost and their MCTF-TCs were stored in lostTCs");

  int ghosts = numOfCaTCs - numOfFoundIDs - countedDoubleEntries;
  int lostTracks = mcTcVector.size() - foundIDs.size();
  B2DEBUG(1, " the tested TrackFinder found total " << numOfFoundIDs << " IDs (perfect/clean/multipleFound/ghost: " <<
          rootVariables.completeCAMomValues.size() << "/" << rootVariables.cleanCAMomValues.size() << "/" << countedDoubleEntries << "/" <<
          ghosts << ") within " << int(caTcVector.size()) << " TCs and lost " << lostTracks);


  if (m_PARAMwriteToRoot == true) {
    m_rootTotalPXresiduals = rootVariables.totalPXresiduals;
    m_rootTotalPYresiduals = rootVariables.totalPYresiduals;
    m_rootTotalPZresiduals = rootVariables.totalPZresiduals;

    m_rootCleanPXresiduals = rootVariables.cleanPXresiduals;
    m_rootCleanPYresiduals = rootVariables.cleanPYresiduals;
    m_rootCleanPZresiduals = rootVariables.cleanPZresiduals;

    m_rootCompletePXresiduals = rootVariables.completePXresiduals;
    m_rootCompletePYresiduals = rootVariables.completePYresiduals;
    m_rootCompletePZresiduals = rootVariables.completePZresiduals;

    m_rootTotalMCMomValues = rootVariables.totalMCMomValues;
    m_rootTotalCAMomValues = rootVariables.totalCAMomValues;
    m_rootCleanCAMomValues = rootVariables.cleanCAMomValues;
    m_rootCompleteCAMomValues = rootVariables.completeCAMomValues;
    m_rootTotalMomValues = rootVariables.totalMomValues;

    m_rootTotalCAMomResiduals = rootVariables.totalCAMomResiduals;
    m_rootCleanCAMomResiduals = rootVariables.cleanCAMomResiduals;
    m_rootCompleteCAMomResiduals = rootVariables.completeCAMomResiduals;

    m_rootTotalMCpTValues = rootVariables.totalMCpTValues;
    m_rootTotalCApTValues = rootVariables.totalCApTValues;
    m_rootCleanCApTValues = rootVariables.cleanCApTValues;
    m_rootCompleteCApTValues = rootVariables.completeCApTValues;
    m_rootTotalpTValues = rootVariables.totalpTValues;

    m_rootTotalCApTResiduals = rootVariables.totalCApTResiduals;
    m_rootCleanCApTResiduals = rootVariables.cleanCApTResiduals;
    m_rootCompleteCApTResiduals = rootVariables.completeCApTResiduals;

    m_rootTotalMCThetaValues = rootVariables.totalMCThetaValues;
    m_rootTotalCAThetaValues = rootVariables.totalCAThetaValues;
    m_rootCleanCAThetaValues = rootVariables.cleanCAThetaValues;
    m_rootCompleteCAThetaValues = rootVariables.completeCAThetaValues;
    m_rootTotalThetaValues = rootVariables.totalThetaValues;

    m_rootTotalCAThetaResiduals = rootVariables.totalCAThetaResiduals;
    m_rootCleanCAThetaResiduals = rootVariables.cleanCAThetaResiduals;
    m_rootCompleteCAThetaResiduals = rootVariables.completeCAThetaResiduals;

    m_rootTotalMCPhiValues = rootVariables.totalMCPhiValues;
    m_rootTotalCAPhiValues = rootVariables.totalCAPhiValues;
    m_rootCleanCAPhiValues = rootVariables.cleanCAPhiValues;
    m_rootCompleteCAPhiValues = rootVariables.completeCAPhiValues;
    m_rootTotalPhiValues = rootVariables.totalPhiValues;

    m_rootTotalCAPhiResiduals = rootVariables.totalCAPhiResiduals;
    m_rootCleanCAPhiResiduals = rootVariables.cleanCAPhiResiduals;
    m_rootCompleteCAPhiResiduals = rootVariables.completeCAPhiResiduals;

    m_rootTotalMCVertex2IP3DValues = rootVariables.totalMCVertex2IP3DValues;
    m_rootTotalCAVertex2IP3DValues = rootVariables.totalCAVertex2IP3DValues;
    m_rootCleanCAVertex2IP3DValues = rootVariables.cleanCAVertex2IP3DValues;
    m_rootCompleteCAVertex2IP3DValues = rootVariables.completeCAVertex2IP3DValues;
    m_rootTotalVertex2IP3DValues = rootVariables.totalVertex2IP3DValues;

    m_rootTotalMCVertex2IPXYValues = rootVariables.totalMCVertex2IPXYValues;
    m_rootTotalCAVertex2IPXYValues = rootVariables.totalCAVertex2IPXYValues;
    m_rootCleanCAVertex2IPXYValues = rootVariables.cleanCAVertex2IPXYValues;
    m_rootCompleteCAVertex2IPXYValues = rootVariables.completeCAVertex2IPXYValues;
    m_rootTotalVertex2IPXYValues = rootVariables.totalVertex2IPXYValues;

    m_rootTotalMCVertex2IPZValues = rootVariables.totalMCVertex2IPZValues;
    m_rootTotalCAVertex2IPZValues = rootVariables.totalCAVertex2IPZValues;
    m_rootCleanCAVertex2IPZValues = rootVariables.cleanCAVertex2IPZValues;
    m_rootCompleteCAVertex2IPZValues = rootVariables.completeCAVertex2IPZValues;
    m_rootTotalVertex2IPZValues = rootVariables.totalVertex2IPZValues;

    m_rootTotalCAMomResidualsAngles = rootVariables.totalCAMomResidualAngles;
    m_rootCleanCAMomResidualsAngles = rootVariables.cleanCAMomResidualAngles;
    m_rootCompleteCAMomResidualsAngles = rootVariables.completeCAMomResidualAngles;

    m_rootTotalCApTResidualsAngles = rootVariables.totalCApTResidualAngles;
    m_rootCleanCApTResidualsAngles = rootVariables.cleanCApTResidualAngles;
    m_rootCompleteCApTResidualsAngles = rootVariables.completeCApTResidualAngles;

    m_rootTotalCASeedPositionResiduals = rootVariables.totalCASeedPositionResiduals;
    m_rootCleanCASeedPositionResiduals = rootVariables.cleanCASeedPositionResiduals;
    m_rootCompleteCASeedPositionResiduals = rootVariables.completeCASeedPositionResiduals;

    m_rootMCreconstructedTrackLength = rootVariables.mCreconstructedTrackLength;
    m_rootCAreconstructedTrackLength = rootVariables.cAreconstructedTrackLength;

    m_rootLostUClusters = rootVariables.lostUClusters;
    m_rootLostVClusters = rootVariables.lostVClusters;

    m_rootTotalMcUClusters = rootVariables.totalMcUClusters;
    m_rootTotalMcVClusters = rootVariables.totalMcVClusters;

    m_rootLostUClusterEDep = rootVariables.lostUClusterEDep;
    m_rootLostVClusterEDep = rootVariables.lostVClusterEDep;

    m_rootTotalMcUClusterEDep = rootVariables.totalMcUClusterEDep;
    m_rootTotalMcVClusterEDep = rootVariables.totalMcVClusterEDep;

    m_treePtr->Fill();
  }

  // Safe information collected from the Collector

  B2DEBUG(100, "m_display " << m_display);


  if (m_display > 0) {
    ana_collector.setAllParticleIDs(m_collectorThreshold);
  }

  if (m_display == 1 && m_collectorFilePath.size() > 0) {

    std::ostringstream oss;
    oss << m_collectorFilePath;
    oss << "all_my_hit_event_" << m_eventCounter << ".csv";
    ana_collector.storeAllHitInformation(oss.str());
    //ana_collector.storeHitInformation(oss.str(), 8);

    std::ostringstream oss_cell;
    oss_cell << m_collectorFilePath;
    oss_cell << "all_my_cell_event_" << m_eventCounter << ".csv";
    ana_collector.storeAllCellInformation(oss_cell.str());
    //ana_collector.storeCellInformation(oss_cell.str(), 8);

    std::ostringstream oss_tc;
    oss_tc << m_collectorFilePath;
    oss_tc << "all_my_tc_event_" << m_eventCounter << ".csv";
    ana_collector.storeAllTCInformation(oss_tc.str());
    //ana_collector.storeTCInformation(oss_tc.str(), 8);

    std::ostringstream oss_clusters;
    oss_clusters << m_collectorFilePath;
    oss_clusters << "all_my_cluster_event_" << m_eventCounter << ".csv";
    ana_collector.storeClustersInformation(oss_clusters.str());

    std::ostringstream oss_sectors;
    oss_sectors << m_collectorFilePath;
    oss_sectors << "all_my_sector_event_" << m_eventCounter << ".csv";
    ana_collector.storeSectorInformation(oss_sectors.str(), false);
  }

  if (m_display == 2) {
    // Test Output for display
    //StoreArray<TrackCandidateTFInfo> tfcandTFInfo("");

    for (auto& currentTC : tfcandTFInfo) {
      B2DEBUG(100, "TC: " << currentTC.getOwnID() << ", AlternativeBox: " << currentTC.getDisplayAlternativeBox() <<
              ", DisplayInformation: " << currentTC.getDisplayInformation());

      std::vector<TVector3> allPosistions = currentTC.getCoordinates();

      for (auto& currentPosition : allPosistions) {
        B2DEBUG(100, "TC Postion: X: " << currentPosition.X() << ", Y: " << currentPosition.Y() << ", Z: " << currentPosition.Z());
      }

    }

    StoreArray<CellTFInfo> cellTFInfo("");

    for (auto& currentCell : cellTFInfo) {
      B2DEBUG(100, "Cell: AlternativeBox: " << currentCell.getDisplayAlternativeBox() << ", DisplayInformation: " <<
              currentCell.getDisplayInformation());

      std::vector<TVector3> allPosistions = currentCell.getCoordinates();

      for (auto& currentPosition : allPosistions) {
        B2DEBUG(100, "Cell Postion: X: " << currentPosition.X() << ", Y: " << currentPosition.Y() << ", Z: " << currentPosition.Z());
      }

    }

    StoreArray<SectorTFInfo> sectorTFInfo("");

    for (auto& currentSector : sectorTFInfo) {
      B2DEBUG(100, "Sector: " << currentSector.getSectorID() << ", AlternativeBox: " << currentSector.getDisplayAlternativeBox() <<
              ", DisplayInformation: " << currentSector.getDisplayInformation());

      std::vector<TVector3> allPosistions = currentSector.getCoordinates();

      for (auto& currentPosition : allPosistions) {
        B2DEBUG(100, "Sector Postion: X: " << currentPosition.X() << ", Y: " << currentPosition.Y() << ", Z: " << currentPosition.Z());
      }

    }


  }

  B2DEBUG(10, "-------------------------------------------------------------------------------");
}


void TFAnalizerModule::endRun()
{
  B2INFO("------------- >>>TFAnalizer::endRun<<< -------------");
  B2DEBUG(1,
          "TFAnalizerModule-explanation: \n perfect recovery means: all hits of mc-TC found again and clean TC. \n clean recovery means: no foreign hits within TC. \n ghost means: QI was below threshold or mcTC was found more than once (e.g. because of curlers) \n found more than once means: that there was more than one TC which was assigned to the same mcTC but each of them were good enough for themselves to be classified as reconstructed");

  B2INFO("TFAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter <<
         " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter <<
         " mcTrackCandidates where used for analysis because of cutoffs.");
  B2INFO("TFAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates in container " << m_PARAMcaTCname << ", of those "
         << m_countAcceptedGFTCs <<
         " were stored in " << m_PARAMacceptedTCname << " and " << m_lostGFTCs << " lost TCs were stored in " << m_PARAMlostTCname <<
         " for further use, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter <<
         ", number of caTCs which produced a double entry: " << m_countedDoubleEntries);
  B2INFO("TFAnalizerModule:  totalCA|totalMC|ratio of pxdHits " << m_nCaPXDHits << "|" << m_nMcPXDHits << "|" << float(
           m_nCaPXDHits) / float(m_nMcPXDHits) <<
         ", svdHits " << m_nCaSVDHits << "|" << m_nMcSVDHits << "|" << float(m_nCaSVDHits) / float(m_nMcSVDHits) << " found by the two TFs");
  B2INFO("TFAnalizerModule: the VXDTF found (total/perfect/clean/ghost)" << m_countReconstructedTCs << "/" <<
         m_countedPerfectRecoveries << "/" << m_countedCleanRecoveries << "/" << (m_caTrackCounter - m_countReconstructedTCs) <<
         " TCs -> efficiency(total/perfect/clean/ghost): " << double(100 * m_countReconstructedTCs) / double(
           m_mcTrackVectorCounter) << "%/" << double(100 * m_countedPerfectRecoveries) / double(m_mcTrackVectorCounter) << "%/" << double(
           100 * m_countedCleanRecoveries) / double(m_mcTrackVectorCounter) << "%/" << double(100 * (m_caTrackCounter -
               m_countReconstructedTCs)) / double(m_countReconstructedTCs) << "%");


}


void TFAnalizerModule::terminate()
{

  if (m_treePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_treePtr->Write();
    m_rootFilePtr->Close();
  }
}


bool TFAnalizerModule::isFirstValueBigger(TFAnalizerModule::foundIDentry& lhs, TFAnalizerModule::foundIDentry& rhs)
{
  if (lhs.first < rhs.first) return true;
  return false;
} // WARNING TODO this function is used by several modules, please create a generalized version for them in a shared container


bool TFAnalizerModule::isFirstValueTheSame(TFAnalizerModule::foundIDentry& lhs, TFAnalizerModule::foundIDentry& rhs)
{
  if (lhs.first == rhs.first) return true;
  return false;
} // WARNING TODO this function is used by several modules, please create a generalized version for them in a shared container


void TFAnalizerModule::printInfo(int recoveryState, VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC, RootVariables& rootVariables)
{

  TVector3 zDir(0., 0., 1.); // vector parallel to z-axis
  TVector3 caDirection = caTC.direction;
  TVector3 mcDirection = mcTC.direction;
  TVector3 caSeedHit = caTC.seedHit;
  TVector3 mcSeedHit = mcTC.seedHit;

  double px = mcDirection[0] - caDirection[0];
  if (px > 7 or px < -7) {
    int i = 0;
    stringstream hitOutput;
    hitOutput << "caTC-Hits:\n";
    for (TVector3 hitPos : caTC.coordinates) { hitOutput << "hit " << i << ": " << hitPos.X() << "/" << hitPos.Y() << "/" << hitPos.Z() << "\n"; ++i;}
    i = 0;
    hitOutput << "mcTC-Hits:\n";
    for (TVector3 hitPos : mcTC.coordinates) { hitOutput << "hit " << i << ": " << hitPos.X() << "/" << hitPos.Y() << "/" << hitPos.Z() << "\n"; ++i;}
    B2DEBUG(10, "printInfo::event " << m_eventCounter << ": pxResidual got bad value (in GeV/c): " << px << " ,ca: " << caDirection[0]
            << ", mc: " << mcDirection[0] << "\n" << hitOutput.str());
  } /// DEBUG
  double py = mcDirection[1] - caDirection[1];
  if (py > 7 or py < -7) { B2DEBUG(10, "printInfo::event " << m_eventCounter << ": pyResidual got bad value (in GeV/c): " << py << " ,ca: " << caDirection[1] << ", mc: " << mcDirection[1]);} /// DEBUG
  double pz = mcDirection[2] - caDirection[2];
  if (pz > 7 or pz < -7) { B2DEBUG(10, "printInfo::event " << m_eventCounter << ": pzResidual got bad value (in GeV/c): " << pz << " ,ca: " << caDirection[2] << ", mc: " << mcDirection[2]);} /// DEBUG

  double thetaMC = mcDirection.Theta() * 180.*TMath::InvPi();
  double thetaCA = caDirection.Theta() * 180.*TMath::InvPi();
  double angle = caDirection.Angle(mcDirection) * 180.*TMath::InvPi(); // angle between the initial momentum vectors in grad

  double phiMC = mcDirection.Phi() * 180.*TMath::InvPi(); // -pi < phi < pi   ->   0 < phi < 360°
  if (phiMC < 0) { phiMC += 360.; }

  double phiCA = caDirection.Phi() * 180.*TMath::InvPi(); // -pi < phi < pi   ->   0 < phi < 360°
  if (phiCA < 0) { phiCA += 360.; }

  caDirection.SetZ(0.);
  mcDirection.SetZ(0.);
  double transverseAngle = caDirection.Angle(mcDirection) * 180.*TMath::InvPi();


  string tcType = "unknown tcType";
  if (recoveryState == 0) {   // not recovered
    tcType = "LostMCTC";
  } else if (recoveryState == 1) {   // contaminated TC
    tcType = "ContaminatedMCTC";
  } else if (recoveryState == 2) {
    tcType = "PerfectMCTC";
    if (m_PARAMwriteToRoot == true) {
      /// why are there values of the mcTC stored? we want to know the real data, not the guesses of the reconstructed data. Guesses of the reconstructed data will be stored in resiudals
      rootVariables.completeCAMomValues.push_back(mcTC.pValue);
      rootVariables.completeCApTValues.push_back(mcTC.pTValue);
      rootVariables.completeCAThetaValues.push_back(thetaMC);
      rootVariables.completeCAPhiValues.push_back(phiMC);
      rootVariables.completeCAVertex2IP3DValues.push_back(mcTC.vertex.Mag());
      rootVariables.completeCAVertex2IPXYValues.push_back(mcTC.vertex.Perp());
      rootVariables.completeCAVertex2IPZValues.push_back(mcTC.vertex.Z());

      rootVariables.completeCAMomResiduals.push_back(mcTC.pValue - caTC.pValue);
      rootVariables.completeCApTResiduals.push_back(mcTC.pTValue - caTC.pTValue);
      rootVariables.completeCAThetaResiduals.push_back(thetaMC - thetaCA);
      rootVariables.completeCAPhiResiduals.push_back(phiMC - phiCA);
      rootVariables.completeCAMomResidualAngles.push_back(angle);
      rootVariables.completeCApTResidualAngles.push_back(transverseAngle);

      rootVariables.completeCASeedPositionResiduals.push_back((mcTC.seedHit - caTC.seedHit).Mag());

      rootVariables.completePXresiduals.push_back(px);
      rootVariables.completePYresiduals.push_back(py);
      rootVariables.completePZresiduals.push_back(pz);

    }
  } else if (recoveryState == 3) {
    tcType = "CleanMCTC";
    if (m_PARAMwriteToRoot == true) {
      /// why are there values of the mcTC stored? we want to know the real data, not the guesses of the reconstructed data. Guesses of the reconstructed data will be stored in resiudals
      rootVariables.cleanCAMomValues.push_back(mcTC.pValue);
      rootVariables.cleanCApTValues.push_back(mcTC.pTValue);
      rootVariables.cleanCAThetaValues.push_back(thetaMC);
      rootVariables.cleanCAPhiValues.push_back(phiMC);
      rootVariables.cleanCAVertex2IP3DValues.push_back(mcTC.vertex.Mag());
      rootVariables.cleanCAVertex2IPXYValues.push_back(mcTC.vertex.Perp());
      rootVariables.cleanCAVertex2IPZValues.push_back(mcTC.vertex.Z());

      rootVariables.cleanCAMomResiduals.push_back(mcTC.pValue - caTC.pValue);
      rootVariables.cleanCApTResiduals.push_back(mcTC.pTValue - caTC.pTValue);
      rootVariables.cleanCAThetaResiduals.push_back(thetaMC - thetaCA);
      rootVariables.cleanCAPhiResiduals.push_back(phiMC - phiCA);
      rootVariables.cleanCAMomResidualAngles.push_back(angle);
      rootVariables.cleanCApTResidualAngles.push_back(transverseAngle);

      rootVariables.cleanCASeedPositionResiduals.push_back((mcTC.seedHit - caTC.seedHit).Mag());

      rootVariables.cleanPXresiduals.push_back(px);
      rootVariables.cleanPYresiduals.push_back(py);
      rootVariables.cleanPZresiduals.push_back(pz);
    }
  } else {
    recoveryState = -1; // means still unknown
  }

  if (recoveryState > 0 and m_PARAMwriteToRoot == true) { // store caValues only if track has been sufficiently good reconstructed

    if (std::find(m_forRootCountFoundIDs.begin(), m_forRootCountFoundIDs.end(), caTC.finalAssignedID) == m_forRootCountFoundIDs.end()
        and             caTC.finalAssignedID != -1) {
      m_forRootCountFoundIDs.push_back(caTC.finalAssignedID);

      /// why are there values of the mcTC stored? we want to know the real data, not the guesses of the reconstructed data. Guesses of the reconstructed data will be stored in resiudals
      rootVariables.totalCAMomValues.push_back(mcTC.pValue);
      rootVariables.totalCApTValues.push_back(mcTC.pTValue);
      rootVariables.totalCAThetaValues.push_back(thetaMC);
      rootVariables.totalCAPhiValues.push_back(phiMC);
      rootVariables.cAreconstructedTrackLength.push_back(caTC.coordinates.size());
      rootVariables.totalCAVertex2IP3DValues.push_back(mcTC.vertex.Mag());
      rootVariables.totalCAVertex2IPXYValues.push_back(mcTC.vertex.Perp());
      rootVariables.totalCAVertex2IPZValues.push_back(mcTC.vertex.Z());

      rootVariables.totalCAMomResiduals.push_back(mcTC.pValue - caTC.pValue);
      rootVariables.totalCApTResiduals.push_back(mcTC.pTValue - caTC.pTValue);
      rootVariables.totalCAThetaResiduals.push_back(thetaMC - thetaCA);
      rootVariables.totalCAPhiResiduals.push_back(phiMC - phiCA);
      rootVariables.totalCAMomResidualAngles.push_back(angle);
      rootVariables.totalCApTResidualAngles.push_back(transverseAngle);

      rootVariables.totalCASeedPositionResiduals.push_back((mcTC.seedHit - caTC.seedHit).Mag());

      rootVariables.totalPXresiduals.push_back(px);
      rootVariables.totalPYresiduals.push_back(py);
      rootVariables.totalPZresiduals.push_back(pz);
      if (true) { /// DEBUGGING! py > 0.08
        B2DEBUG(10, " pyResidual is: " << py << ", pxGuess/pxReal/pyGuess/pyReal/pzGuess/pzReal: " << caDirection[0] << "/" <<
                mcDirection[0] << "/" << caDirection[1] << "/" << mcDirection[1] << "/" << mcDirection[2] << "/" << caDirection[2] <<
                ", same for position: " << caSeedHit[0] << "/" << mcSeedHit[0] << "/" << caSeedHit[1] << "/" << mcSeedHit[1] << "/" << caSeedHit[2]
                << "/" << mcSeedHit[2] << " guessed/real pdg: " << caTC.pdgCode << "/" << mcTC.pdgCode);
      }
    }
  }

  if (m_PARAMprintExtentialAnalysisData == true) {
    B2INFO("PRINTINFO: At event " << m_eventCounter <<
           ": mcType §" << tcType <<
           "§ having §" << mcTC.coordinates.size() <<
           "§ hits with thetaMC of (mc/ca)§" << setprecision(5) << thetaMC << "/" << thetaCA <<
           "§° got pT of (mc/ca)§" << setprecision(4) << mcTC.pTValue << "/" << setprecision(4) << caTC.pTValue <<
           /*"§ GeV/c, assigned caTC got pT of §" << setprecision(4) << caTC.pTValue <<*/
           "§ GeV/c, and probValue of §" << setprecision(6) << caTC.probValue <<
           "§. Their residual of seedHit was §" << setprecision(4) << (mcTC.seedHit - caTC.seedHit).Mag()
           << /*difference of mctc and catc innermost hits */
           "§. Their residual of pT was §" << setprecision(4) << mcTC.pTValue - caTC.pTValue
           << /*difference of estimated and real transverseMomentum */
           "§ GeV/c, their residual of angle was §" << setprecision(6) << angle <<
           "§ in grad, their residual of transverse angle was §" << setprecision(6) << transverseAngle <<
           "§ with PDGCode of mcTC: §" << mcTC.pdgCode <<
           "§ and PDGCode of caTC: §" << caTC.pdgCode <<
           "§"); // '§' will be used to filter
  }
  if (mcTC.pdgCode == 13 or mcTC.pdgCode == -13) {   // muons
    if (caTC.pdgCode == 13 or caTC.pdgCode == -13) {
      if (sign(caTC.pdgCode) not_eq sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    } else {
      if (sign(caTC.pdgCode) == sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    }

  } else {
    if (caTC.pdgCode == 13 or caTC.pdgCode == -13) {
      if (sign(caTC.pdgCode) == sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    } else {
      if (sign(caTC.pdgCode) not_eq sign(mcTC.pdgCode)) { m_wrongChargeSignCounter++; }
    }
  }

}


void TFAnalizerModule::printMC(bool type, VXDTrackCandidate& mcTC, RootVariables& rootVariables)
{
  // get information about nClusters per type of current TC:
  StoreArray<SVDCluster> svdClusters;
  int uClusters = 0, vClusters = 0;
  vector<double> uClusterEDep, vClusterEDep;
  for (int index : mcTC.svdClusterIDs) {
    if (svdClusters[index]->isUCluster()) {
      uClusters++;
      uClusterEDep.push_back(svdClusters[index]->getCharge());
      continue;
    }
    vClusters++;
    vClusterEDep.push_back(svdClusters[index]->getCharge());
  }

  string info = "FOUNDINFO";
  if (type == false) {
    info = "LOSTINFO";

    rootVariables.lostUClusters.push_back(uClusters);
    rootVariables.lostVClusters.push_back(vClusters);
    rootVariables.lostUClusterEDep.insert(rootVariables.lostUClusterEDep.end(), uClusterEDep.begin(), uClusterEDep.end());
    rootVariables.lostVClusterEDep.insert(rootVariables.lostVClusterEDep.end(), vClusterEDep.begin(), vClusterEDep.end());
  }

//   TVector3 zDir(0., 0., 1.); // vector parallel to z-axis
  TVector3 mcDirection = mcTC.direction;
  double theta = mcDirection.Theta() * 180.*TMath::InvPi(); //0 < theta < pi   ->   0 < theta < 180°
  double phi = mcDirection.Phi() * 180.*TMath::InvPi(); // -pi < phi < pi   ->   0 < phi < 360°
  if (phi < 0) { phi += 360; }

  // store mcValues in any case...
  if (m_PARAMwriteToRoot == true) {
    rootVariables.totalMCMomValues.push_back(mcTC.pValue);
    rootVariables.totalMCpTValues.push_back(mcTC.pTValue);
    rootVariables.totalMCThetaValues.push_back(theta);
    rootVariables.totalMCPhiValues.push_back(phi);
    rootVariables.mCreconstructedTrackLength.push_back(mcTC.coordinates.size());

    rootVariables.totalMCVertex2IP3DValues.push_back(mcTC.vertex.Mag());
    rootVariables.totalMCVertex2IPXYValues.push_back(mcTC.vertex.Perp());
    rootVariables.totalMCVertex2IPZValues.push_back(mcTC.vertex.Z());

    rootVariables.totalMcUClusters.push_back(uClusters);
    rootVariables.totalMcVClusters.push_back(vClusters);
    rootVariables.totalMcUClusterEDep.insert(rootVariables.totalMcUClusterEDep.end(), uClusterEDep.begin(), uClusterEDep.end());
    rootVariables.totalMcVClusterEDep.insert(rootVariables.totalMcVClusterEDep.end(), vClusterEDep.begin(), vClusterEDep.end());

  }

  if (m_PARAMprintExtentialAnalysisData == true) {
    B2INFO(info << ": At event " << m_eventCounter <<
           ": MC with ID " << mcTC.indexNumber << " having §" << mcTC.coordinates.size() <<
           "§ hits with theta of §" << setprecision(4) <<  theta <<
           "§ and phi of §" << setprecision(4) <<  phi <<
           "§° got pT of §" << setprecision(4) << mcTC.pTValue <<
           "§ GeV/c and vertex distance to origin: §" << setprecision(4) << mcTC.vertex.Mag() <<
           "§cm, transverseDistance: §" << setprecision(4) << mcTC.vertex.Perp() <<
           "§cm, zDistance: §" << setprecision(4) << mcTC.vertex.Z() <<
           "§, and pdg of: §" << setprecision(4) << mcTC.pdgCode <<
           "§"); // '§' will be used to filter
  }
}



void TFAnalizerModule::printCA(bool type, VXDTrackCandidate& caTC)
{
  string info = "FOUNDCATCINFO";
  if (type == false) { info = "LOSTCATCINFO"; }

//   TVector3 zDir(0., 0., 1.); // vector parallel to z-axis

  B2INFO(info << ": At event " << m_eventCounter <<
         ": CA with assigned ID " << caTC.finalAssignedID <<
         " having §" << caTC.coordinates.size() <<
         "§ hits with theta of §" << setprecision(4) << caTC.direction.Theta() * 180.*TMath::InvPi() <<
         "§° got pT of §" << setprecision(4) << caTC.pTValue <<
         "§ GeV/c, QI of §" << setprecision(4) << caTC.qualityIndex <<
         "§ and pdg of: " << caTC.pdgCode); // '§' will be used to filter
}



void TFAnalizerModule::extractHits(genfit::TrackCand* aTC,
                                   RelationIndex<PXDCluster, PXDTrueHit>& relationPXD,
                                   RelationIndex<SVDCluster, SVDTrueHit>& relationSVD,
                                   StoreArray<PXDCluster>& pxdClusters,
                                   StoreArray<SVDCluster>& svdClusters,
                                   StoreArray<VXDTFInfoBoard>& infoBoards,
                                   StoreArray<TrackCandidateTFInfo>& infosTCs,
                                   vector<VXDTrackCandidate>& tcVector,
                                   bool isMCTC,
                                   int index)
{
  TVector3 hitLocal, hitGlobal;
  VxdID aVxdID;
  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  B2DEBUG(10, "starting extractHits... isMCTC: " << isMCTC << ", index: " << index);
  int numOfHits = aTC->getNHits();
  B2DEBUG(10, " found " << numOfHits << " hits for TC " << index);
  if (int(aTC->getNHits()) == 0) {
    B2ERROR("TFAnalizerModule::extractHits - event " << m_eventCounter << ": GfTrackcand with isMCTC " << isMCTC <<
            " has no hits, neglecting tc...:");
    return;
  }
  vector<int> pxdHitIDsOfCurrentTC;
  vector<int> svdHitIDsOfCurrentTC;
  vector<TVector3> coordinates;
//  vector<PXDTrueHit*> pxdTrueHits;
  vector<const SVDTrueHit*> svdTrueHits;

  /*typedef std::map<int, int> map_type;
  typedef pair<unsigned int, vector<int> > pair4SvdHitMap;
  map< pair4SvdHitMap > svdHitMap;*/ // since svdClusters are only 1D and we can not assume, that two clusters of the same hit are found in pairs within the TC-hit-List, we have to sort them before using them TODO find out, why this value is not used!
  for (int hitIndex = 0; hitIndex not_eq numOfHits; ++hitIndex) {
    B2DEBUG(100, "----importing hit " << hitIndex << " from trackCandidate...");
    int detID = -1; // ID of detector
    int hitID = -1; // ID of Hit in StoreArray
    aTC->getHit(hitIndex, detID, hitID); // sets detId and hitId for given hitIndex
    B2DEBUG(100, "----got Hitinfo. detID: " << detID << ", hitID: " << hitID);
    if (detID == Const::PXD) { // pxd
      pxdHitIDsOfCurrentTC.push_back(hitID);
      hitLocal.SetXYZ(pxdClusters[hitID]->getU(), pxdClusters[hitID]->getV(), 0);
      B2DEBUG(100, "gotPXD, u: " << pxdClusters[hitID]->getU() << ", v: " << pxdClusters[hitID]->getV());
      aVxdID = pxdClusters[hitID]->getSensorID();
      VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
      hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
      coordinates.push_back(hitGlobal);
    }
    if (detID == Const::SVD) { // svd
      svdHitIDsOfCurrentTC.push_back(hitID);
      RelationIndex<SVDCluster, SVDTrueHit>::range_from relationRange = relationSVD.getElementsFrom(svdClusters[hitID]);
      for (const auto& relElement : relationRange) {
        const SVDTrueHit* aTrueHit = relElement.to;

        aVxdID = aTrueHit->getSensorID();

        hitLocal.SetXYZ(aTrueHit->getU(), aTrueHit->getV(), 0);
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
        coordinates.push_back(hitGlobal);
        B2DEBUG(100, "gotSVD, u: " << aTrueHit->getU() << ", v: " << aTrueHit->getV());

        svdTrueHits.push_back(aTrueHit);
      }
    }
  } /// getting Info for each hit. 1D clusters make the whole thing complicated since it has to be checked, whether two 1D hits form a 2D hit of the track or not WARNING: for each SVD-Cluster, there is a full hit added to the coordinates-list ->therefore hits are doubled

  TcInfoTuple tcInfo = boost::make_tuple(pxdHitIDsOfCurrentTC, svdHitIDsOfCurrentTC, coordinates, svdTrueHits);

  ///get Momentum information:
  TVectorD stateSeed = aTC->getStateSeed();
//  TVector3 dirSeed = aTC->getDirSeed();
//  double qOverP = aTC->getQoverPseed();
//  double pOverQ = 1./qOverP;
  TVector3 momentum, /*momentum_t,*/ vertex, seedHit; // = pOverQ*dirSeed;
  momentum[0] = stateSeed(3); momentum[1] = stateSeed(4); momentum[2] = stateSeed(5);
  vertex[0] = stateSeed(0); vertex[1] = stateSeed(1); vertex[2] = stateSeed(2);
  seedHit = vertex; // for CATC seedHit is innermost hit of TC, for MCTC it is the vertex or the innermost hit
  double pValue = momentum.Mag();
//   momentum_t = momentum;
//   momentum_t.SetZ(0.);
  double pT = momentum.Perp();
  int pdgCode = aTC->getPdgCode();
  B2DEBUG(10, " tc number " << index << " with isMCTC " << isMCTC << " has got initial pValue " << pValue << ", pdgCode " << pdgCode
          << " and " << int(aTC->getNHits()) << " hits");

  if (isMCTC == true) {   // want momentum vector of innermost hit, not of primary vertex
    bool gotNewMomentum = false;

    if (pT < m_PARAMminTMomentumFilter or pT > m_PARAMmaxTMomentumFilter) { return; } // do not store mcTrack in this case

    int detID = -1; // ID of detector
    int hitID = -1; // ID of Hit in StoreArray
    TVector3 tempMomentum; // momentum of innermost hit
    TVector3 tempSeedHit; // position of innermost hit
    aTC->getHit(0, detID, hitID); // 0 means innermost hit
    if (detID == Const::PXD) {  // means PXD
      vector< pair<double, const PXDTrueHit* > > tempPXDSeeds; // .first = momentumResidual (vertex - hit), .second pointer to hit
      RelationIndex<PXDCluster, PXDTrueHit>::range_from relationRange = relationPXD.getElementsFrom(pxdClusters[hitID]);
      for (const auto& relElement : relationRange) {
        // since more than one trueHit can be the cause of current hit, we have to find the real TrueHit. Identified by |momentum| (that trueHit with the closest total momentum compared to the primary vertex is accepted)
        const PXDTrueHit* aTrueHit = relElement.to;
        double momentumResidual = pValue - aTrueHit->getMomentum().Mag();
        if (momentumResidual < 0) { momentumResidual = -momentumResidual; }
        tempPXDSeeds.push_back(make_pair(momentumResidual, aTrueHit));
//         if (pValue - tempMomentum.Mag() < pValue * 0.1) { gotNewMomentum = true; break; } // if difference in Momentum is less than 10% of initial momentum, we accept current value as the real one
      }

      if (tempPXDSeeds.size() != 0) {
        std::sort(tempPXDSeeds.begin(),
                  tempPXDSeeds.end()); // sorts by first entry of pair -> entry with smallest difference between vertex and hit will be taken...
        tempMomentum = tempPXDSeeds.at(0).second->getMomentum();
        tempSeedHit.SetXYZ(tempPXDSeeds.at(0).second->getU(), tempPXDSeeds.at(0).second->getV(), tempPXDSeeds.at(0).second->getW());

        aVxdID = tempPXDSeeds.at(0).second->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        tempMomentum = aSensorInfo.vectorToGlobal(tempMomentum);
        tempSeedHit = aSensorInfo.pointToGlobal(tempSeedHit);
//        B2ERROR(" tempSeedHit stored...")
        gotNewMomentum = true;
      }
    } else if (detID == Const::SVD) {  // SVD
      vector< pair<double, const SVDTrueHit* > > tempSVDSeeds; // .first = momentumResidual (vertex - hit), .second pointer to hit
      RelationIndex<SVDCluster, SVDTrueHit>::range_from relationRange = relationSVD.getElementsFrom(svdClusters[hitID]);
      for (const auto& relElement : relationRange) {
        // since more than one trueHit can be the cause of current hit, we have to find the real TrueHit. Identified by |momentum|
        const SVDTrueHit* aTrueHit = relElement.to;
        double momentumResidual = pValue - aTrueHit->getMomentum().Mag();
        if (momentumResidual < 0) { momentumResidual = -momentumResidual; }
        tempSVDSeeds.push_back(make_pair(momentumResidual, aTrueHit));
      }

      if (tempSVDSeeds.size() != 0) {
        std::sort(tempSVDSeeds.begin(),
                  tempSVDSeeds.end()); // sorts by first entry of pair -> entry with smallest difference between vertex and hit will be taken...
        tempMomentum = tempSVDSeeds.at(0).second->getMomentum();
        tempSeedHit.SetXYZ(tempSVDSeeds.at(0).second->getU(), tempSVDSeeds.at(0).second->getV(), tempSVDSeeds.at(0).second->getW());

        aVxdID = tempSVDSeeds.at(0).second->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
        tempMomentum = aSensorInfo.vectorToGlobal(tempMomentum);
        tempSeedHit = aSensorInfo.pointToGlobal(tempSeedHit);
//        B2ERROR(" tempSeedHit stored...")
        gotNewMomentum = true;
      }
    } else { B2FATAL("TFAnalizer - this track candidate does not have any VXD-hits, can not analyze it"); }

    if (gotNewMomentum == true) {
      momentum = tempMomentum;
      seedHit = tempSeedHit;
      pValue = momentum.Mag();
      pT = momentum.Pt();
      B2DEBUG(10, "TFAnalizer event " << m_eventCounter << ": calculated new momentum (pT = " << pT << ")");
    }
  }

  VXDTrackCandidate newTC;
  newTC.pxdClusterIDs = boost::get<0>(tcInfo);
  newTC.svdClusterIDs = boost::get<1>(tcInfo);
  newTC.coordinates = boost::get<2>(tcInfo);
  newTC.direction = momentum;
  newTC.seedHit = seedHit;
  newTC.svdTrueHits = boost::get<3>(tcInfo);
  newTC.isMCtrackCandidate = isMCTC;
  newTC.indexNumber = index;
  newTC.pTValue = pT;
  newTC.pValue = pValue;
  newTC.pdgCode = pdgCode;

  if (isMCTC == true) {
    newTC.finalAssignedID = tcVector.size();
    newTC.qualityIndex = 1;
    newTC.vertex = vertex;
  } else {
    newTC.finalAssignedID = -1;
    newTC.qualityIndex = 0;

    /// read additional info:
    int gfIndex = aTC->getMcTrackId();

    // To be replaced
    newTC.probValue = infoBoards[gfIndex]->getProbValue();
    newTC.survivedFit = infoBoards[gfIndex]->isFitPossible();

    if (m_display > 0) {

      int currentTCID = ana_collector.getTcIDFromGfIndex(gfIndex);

      if (currentTCID != -1) {
        newTC.probValue = infosTCs[currentTCID]->getProbValue();
        newTC.survivedFit = infosTCs[currentTCID]->isFitPossible();

        B2DEBUG(10, "New probValue: " << newTC.probValue << ", Old probValue: " << infoBoards[gfIndex]->getProbValue());

        B2DEBUG(10, "New survivedFit: " << newTC.survivedFit << ", Old survivedFit: " << infoBoards[gfIndex]->isFitPossible());
      }

    }

  }
  tcVector.push_back(newTC);
  B2DEBUG(10, " end of extractHits. TC isMCTC: " << isMCTC << ", PDGCode: " << pdgCode << ", finalAssignedID: " <<
          newTC.finalAssignedID << ", indexNumber: " << newTC.indexNumber << ", pValue: " << pValue << ", pT: " << pT <<
          ", new tcVector.size(): " << tcVector.size());
}





void TFAnalizerModule::checkCompatibility(VXDTrackCandidate& mcTC, VXDTrackCandidate& caTC)
{
  double compatibilityValue;
  int goodHitsInCaTC = 0; // if hit of mcTC is found in caTC too -> ++;
  int badHitsInCaTC = 0; // if hit of caTC is not found in mcTC -> ++:
  B2DEBUG(100, "--checkCompatibility has started, comparing mcTC: " << mcTC.indexNumber << " with caTC: " << caTC.indexNumber);
  /** several approaches:
   * 1. caTC-based: how many hits of the caTC were good ones?
   *  A. check number of total hits of caTC
   *  B. concatenate hits of poth, caTC and mcTC, unique & sort them.
   *  C. compare number of total hits before and after doing unique. Relation between difference of before and after and the number of caTC-hits -> compatibilityValue caTC
   *  D. Difference between before and after unique and total number of caTC
   * 2. mcTC-Based:  Difference between before and after and the total number of Hits of MCTC
   *
   * if both caTC and mcTC were compatible, they get indexed to each other
   */

  for (int indexValueMC : mcTC.pxdClusterIDs) {   /// PXDHits
    int trueCtr = 0, falseCtr = 0;
    for (int indexValueCA : caTC.pxdClusterIDs) {
      if (indexValueCA == indexValueMC) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr != 0) { goodHitsInCaTC++; }
  } // check good hits for mcTC (PXD) counts each hit of the mctc which was found again in the catc
  B2DEBUG(10, "--after check good hits for mcTC(PXD), value is: " << goodHitsInCaTC);

  for (const SVDTrueHit* aTrueHitmc : mcTC.svdTrueHits) {   /// SVDHits
    int trueCtr = 0, falseCtr = 0;
    for (const SVDTrueHit* aTrueHitca : caTC.svdTrueHits) {
      if (aTrueHitca == aTrueHitmc) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr != 0) { goodHitsInCaTC++; }
  } // check good hits for mcTC (SVD) counts each hit of the mctc which was found again in the catc
  B2DEBUG(10, "--after check good hits for mcTC(SVD), value is: " << goodHitsInCaTC);

  for (int indexValueCA : caTC.pxdClusterIDs) {
    int trueCtr = 0, falseCtr = 0;
    for (int indexValueMC : mcTC.pxdClusterIDs) {
      if (indexValueCA == indexValueMC) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr == 0) { badHitsInCaTC++; }
  } // check bad hits for caTC (PXD) counts each hit of the catc which was NOT found in the mctc
  B2DEBUG(10, "--after check bad hits for caTC (PXD), value is: " << badHitsInCaTC);

  for (const SVDTrueHit* aTrueHitca : caTC.svdTrueHits) {   /// SVDHits
    int trueCtr = 0, falseCtr = 0;
    for (const SVDTrueHit* aTrueHitmc : mcTC.svdTrueHits) {
      if (aTrueHitca == aTrueHitmc) {
        trueCtr++;
      } else { falseCtr++; }
    }
    if (trueCtr == 0) { badHitsInCaTC++; }
  } // check bad hits for caTC (SVD) counts each hit of the catc which was NOT found in the mctc

  B2DEBUG(10, "--after check bad hits for caTC (SVD), value is: " << badHitsInCaTC);
  int totalMCHits = mcTC.pxdClusterIDs.size() + mcTC.svdTrueHits.size();
  compatibilityValue = double(goodHitsInCaTC) / double(totalMCHits);
  B2DEBUG(10, "calculated compatibilityValue: " << compatibilityValue << ", caTC.qualityIndex: " << caTC.qualityIndex);

  if (caTC.qualityIndex <
      compatibilityValue) {  // in this case, the current mcTC suits better for the caTC guess than for the last one.
    caTC.qualityIndex = compatibilityValue;
    caTC.finalAssignedID = mcTC.finalAssignedID;
    caTC.numOfCorrectlyAssignedHits = goodHitsInCaTC;
    caTC.numOfBadAssignedHits = badHitsInCaTC;
  }
  B2DEBUG(10, "after check, caTC.qualityIndex: " << caTC.qualityIndex << ", caTC.finalAssignedID: " << caTC.finalAssignedID);
  CompatibilityIndex caCindex = boost::make_tuple(mcTC.indexNumber, goodHitsInCaTC, badHitsInCaTC, totalMCHits - goodHitsInCaTC,
                                                  compatibilityValue);
  CompatibilityIndex mcCindex = boost::make_tuple(caTC.indexNumber, goodHitsInCaTC, totalMCHits - goodHitsInCaTC, badHitsInCaTC,
                                                  compatibilityValue);
  mcTC.compatiblePartners.push_back(mcCindex);
  caTC.compatiblePartners.push_back(caCindex);
}
