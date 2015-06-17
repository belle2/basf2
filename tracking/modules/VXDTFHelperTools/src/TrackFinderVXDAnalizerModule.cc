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
#include "tracking/modules/VXDTFHelperTools/TrackFinderVXDAnalizerModule.h"
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/gearbox/Const.h>

#include <tracking/spacePointCreation/PurityCalculatorTools.h>

#include <tracking/trackFindingVXD/analyzingTools/AnalizerTCInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/AnalyzingAlgorithmFactory.h>


#include <tracking/vectorTools/B2Vector3.h>
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
#include <TVector3.h>

//boost stuff
#include <boost/math/special_functions/sign.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;
using boost::tuple;
using boost::math::sign;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFinderVXDAnalizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFinderVXDAnalizerModule::TrackFinderVXDAnalizerModule() : Module()
{
  InitializeVariables();

  vector<string> rootFileNameVals;
  rootFileNameVals.push_back("TrackFinderVXDAnalizerResults");
  rootFileNameVals.push_back("RECREATE");

  vector<vector<string> > parametersToBeTracked = {
    {"Contaminated", "AnalyzingAlgorithmResidualP"},
    {"Clean", "AnalyzingAlgorithmResidualP"},
    {"Perfect", "AnalyzingAlgorithmResidualP"}
  };

  //Set module properties
  setDescription("analyzes quality of SpacePointTrackCands delivered by a test-TF compared to a reference TF");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("fileExportMcTracks", m_PARAMFileExportMcTracks, "export mc Trackfinder tracks into file", bool(false));
  addParam("fileExportTfTracks", m_PARAMFileExportTfTracks, "export vxd Trackfinder tracks into file", bool(false));
  addParam("referenceTCname", m_PARAMreferenceTCname, "the name of the storeArray container provided by the reference TF",
           string("mcTracks"));
  addParam("testTCname", m_PARAMtestTCname, "the name of the storeArray container provided by the TF to be evaluated", string(""));
  addParam("acceptedTCname", m_PARAMacceptedTCname, "special name for accepted/successfully reconstructed track candidates",
           string("acceptedVXDTFTracks"));
  addParam("lostTCname", m_PARAMlostTCname, "special name for lost track candidates", string("lostTracks"));
  addParam("purityThreshold", m_PARAMpurityThreshold,
           " chose value to filter TCs found by Test TF. TCs having purities lower than this value won't be marked as reconstructed",
           double(0.7));
  addParam("minNDFThreshold", m_PARAMminNDFThreshold,
           " defines how many measurements (numbers of degrees of freedom) the TC must have to be accepted as reconstructed, standard is 5",
           unsigned(5));
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

  addParam("parametersToBeTracked", m_PARAMparametersToBeTracked,
           "accepts a vector of pairs of entries, each pair determines a TC type (first entry) and the algorithm to be used (second entry)",
           parametersToBeTracked);
  addParam("origin", m_PARAMorigin, " only allowed size: 3. stores coordinates of the origin used", {0, 0, 0});
}


void TrackFinderVXDAnalizerModule::initialize()
{
  m_referenceTCs.isRequired(m_PARAMreferenceTCname);
  m_testTCs.isRequired(m_PARAMtestTCname);
  m_acceptedTCs.registerInDataStore(m_PARAMacceptedTCname, DataStore::c_DontWriteOut);
  m_lostTCs.registerInDataStore(m_PARAMlostTCname, DataStore::c_DontWriteOut);


  if (m_PARAMorigin.size() != 3) {
    B2ERROR("TrackFinderVXDAnalizerModule::initialize() passed parameter 'origin' has wrong number of entries (allowed: 3) of " <<
            m_PARAMorigin.size() << " reset to standard (0, 0, 0)")
    m_PARAMorigin = {0, 0, 0};
  }

  // deal with root-related stuff
  if (m_PARAMwriteToRoot == false) { return; }

  if ((m_PARAMrootFileName.size()) != 2) {
    string output;
    for (string& entry : m_PARAMrootFileName) {
      output += "'" + entry + "' ";
    }
    B2FATAL("TrackFinderVXDAnalizer::initialize(), rootFileName is set wrong, although parameter 'writeToRoot' is enabled! Actual entries are: "
            <<
            output)
  }

  // deal with algorithms:
  m_rootParameterTracker.initialize(m_PARAMrootFileName[0] + ".root", m_PARAMrootFileName[1]);
  for (auto& parameterSet : m_PARAMparametersToBeTracked) {
    if (parameterSet.size() != 2) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'parametersToBeTracked' was mis-used! Please read the documentation!") }

    // store in internal bla, prepare branch
    m_rootParameterTracker.addParameters(parameterSet[0], parameterSet[1]);
  }


  AnalyzingAlgorithmBase<double, AnalizerTCInfo, TVector3>().setOrigin(TVector3(m_PARAMorigin[0], m_PARAMorigin[1],
      m_PARAMorigin[2]));

  AnalyzingAlgorithmBase<double, AnalizerTCInfo, TVector3> algo = AnalyzingAlgorithmFactory<double, AnalizerTCInfo, TVector3>
      (string("AnalyzingAlgorithmBase"));

  B2DEBUG(1, " TrackFinderVXDAnalizerModule::initialize(): algo is " << algo.getID())

  if (m_PARAMwriteToRoot == true) {
//     if ((m_PARAMrootFileName.size()) != 2) {
//       string output;
//       for(string& entry : m_PARAMrootFileName) {
//         output += "'" + entry + "' ";
//       }
//       B2FATAL("TFAnalizer::initialize: rootFileName is set wrong, although parameter 'writeToRoot' is enabled! Actual entries are: " <<
//               output)
//     }
//     m_PARAMrootFileName[0] += ".root";
//     m_rootFilePtr = new TFile(m_PARAMrootFileName[0].c_str(), m_PARAMrootFileName[1].c_str()); // alternative: UPDATE
//     m_treePtr = new TTree("m_treePtr", "aTree");
//
//     m_treePtr->Branch("TotalPXresiduals", &m_rootTotalPXresiduals);
//     m_treePtr->Branch("TotalPYresiduals", &m_rootTotalPYresiduals);
//     m_treePtr->Branch("TotalPZresiduals", &m_rootTotalPZresiduals);
//
//     m_treePtr->Branch("CleanPXresiduals", &m_rootCleanPXresiduals);
//     m_treePtr->Branch("CleanPYresiduals", &m_rootCleanPYresiduals);
//     m_treePtr->Branch("CleanPZresiduals", &m_rootCleanPZresiduals);
//
//     m_treePtr->Branch("CompletePXresiduals", &m_rootCompletePXresiduals);
//     m_treePtr->Branch("CompletePYresiduals", &m_rootCompletePYresiduals);
//     m_treePtr->Branch("CompletePZresiduals", &m_rootCompletePZresiduals);
//
//     m_treePtr->Branch("TotalMCMomValues", &m_rootTotalMCMomValues);
//     m_treePtr->Branch("TotalCAMomValues", &m_rootTotalCAMomValues);
//     m_treePtr->Branch("CleanCAMomValues", &m_rootCleanCAMomValues);
//     m_treePtr->Branch("CompleteCAMomValues", &m_rootCompleteCAMomValues);
//     m_treePtr->Branch("TotalMomValues", &m_rootTotalMomValues);
//
//     m_treePtr->Branch("TotalCAMomResiduals", &m_rootTotalCAMomResiduals);
//     m_treePtr->Branch("CleanCAMomResiduals", &m_rootCleanCAMomResiduals);
//     m_treePtr->Branch("CompleteCAMomResiduals", &m_rootCompleteCAMomResiduals);
//
//     m_treePtr->Branch("TotalMCpTValues", &m_rootTotalMCpTValues);
//     m_treePtr->Branch("TotalCApTValues", &m_rootTotalCApTValues);
//     m_treePtr->Branch("CleanCApTValues", &m_rootCleanCApTValues);
//     m_treePtr->Branch("CompleteCApTValues", &m_rootCompleteCApTValues);
//     m_treePtr->Branch("TotalpTValues", &m_rootTotalpTValues);
//
//     m_treePtr->Branch("TotalCApTResiduals", &m_rootTotalCApTResiduals);
//     m_treePtr->Branch("CleanCApTResiduals", &m_rootCleanCApTResiduals);
//     m_treePtr->Branch("CompleteCApTResiduals", &m_rootCompleteCApTResiduals);
//
//     m_treePtr->Branch("TotalMCThetaValues", &m_rootTotalMCThetaValues);
//     m_treePtr->Branch("TotalCAThetaValues", &m_rootTotalCAThetaValues);
//     m_treePtr->Branch("CleanCAThetaValues", &m_rootCleanCAThetaValues);
//     m_treePtr->Branch("CompleteCAThetaValues", &m_rootCompleteCAThetaValues);
//     m_treePtr->Branch("TotalThetaValues", &m_rootTotalThetaValues);
//
//     m_treePtr->Branch("TotalCAThetaResiduals", &m_rootTotalCAThetaResiduals);
//     m_treePtr->Branch("CleanCAThetaResiduals", &m_rootCleanCAThetaResiduals);
//     m_treePtr->Branch("CompleteCAThetaResiduals", &m_rootCompleteCAThetaResiduals);
//
//     m_treePtr->Branch("TotalMCPhiValues", &m_rootTotalMCPhiValues);
//     m_treePtr->Branch("TotalCAPhiValues", &m_rootTotalCAPhiValues);
//     m_treePtr->Branch("CleanCAPhiValues", &m_rootCleanCAPhiValues);
//     m_treePtr->Branch("CompleteCAPhiValues", &m_rootCompleteCAPhiValues);
//     m_treePtr->Branch("TotalPhiValues", &m_rootTotalPhiValues);
//
//     m_treePtr->Branch("TotalMCVertex2IP3DValues", &m_rootTotalMCVertex2IP3DValues);
//     m_treePtr->Branch("TotalCAVertex2IP3DValues", &m_rootTotalCAVertex2IP3DValues);
//     m_treePtr->Branch("CleanCAVertex2IP3DValues", &m_rootCleanCAVertex2IP3DValues);
//     m_treePtr->Branch("CompleteCAVertex2IP3DValues", &m_rootCompleteCAVertex2IP3DValues);
//     m_treePtr->Branch("TotalVertex2IP3DValues", &m_rootTotalVertex2IP3DValues);
//
//     m_treePtr->Branch("TotalMCVertex2IPXYValues", &m_rootTotalMCVertex2IPXYValues);
//     m_treePtr->Branch("TotalCAVertex2IPXYValues", &m_rootTotalCAVertex2IPXYValues);
//     m_treePtr->Branch("CleanCAVertex2IPXYValues", &m_rootCleanCAVertex2IPXYValues);
//     m_treePtr->Branch("CompleteCAVertex2IPXYValues", &m_rootCompleteCAVertex2IPXYValues);
//     m_treePtr->Branch("TotalVertex2IPXYValues", &m_rootTotalVertex2IPXYValues);
//
//     m_treePtr->Branch("TotalMCVertex2IPZValues", &m_rootTotalMCVertex2IPZValues);
//     m_treePtr->Branch("TotalCAVertex2IPZValues", &m_rootTotalCAVertex2IPZValues);
//     m_treePtr->Branch("CleanCAVertex2IPZValues", &m_rootCleanCAVertex2IPZValues);
//     m_treePtr->Branch("CompleteCAVertex2IPZValues", &m_rootCompleteCAVertex2IPZValues);
//     m_treePtr->Branch("TotalVertex2IPZValues", &m_rootTotalVertex2IPZValues);
//
//     m_treePtr->Branch("TotalCAThetaResiduals", &m_rootTotalCAThetaResiduals);
//     m_treePtr->Branch("CleanCAThetaResiduals", &m_rootCleanCAThetaResiduals);
//     m_treePtr->Branch("CompleteCAThetaResiduals", &m_rootCompleteCAThetaResiduals);
//
//     m_treePtr->Branch("TotalCAMomResidualsAngles", &m_rootTotalCAMomResidualsAngles);
//     m_treePtr->Branch("CleanCAMomResidualsAngles", &m_rootCleanCAMomResidualsAngles);
//     m_treePtr->Branch("CompleteCAMomResidualsAngles", &m_rootCompleteCAMomResidualsAngles);
//
//     m_treePtr->Branch("TotalCApTResidualsAngles", &m_rootTotalCApTResidualsAngles);
//     m_treePtr->Branch("CleanCApTResidualsAngles", &m_rootCleanCApTResidualsAngles);
//     m_treePtr->Branch("CompleteCApTResidualsAngles", &m_rootCompleteCApTResidualsAngles);
//
//     m_treePtr->Branch("TotalCASeedPositionResiduals", &m_rootTotalCASeedPositionResiduals);
//     m_treePtr->Branch("CleanCASeedPositionResiduals", &m_rootCleanCASeedPositionResiduals);
//     m_treePtr->Branch("CompleteCASeedPositionResiduals", &m_rootCompleteCASeedPositionResiduals);
//
//     m_treePtr->Branch("MCreconstructedTrackLength", &m_rootMCreconstructedTrackLength);
//     m_treePtr->Branch("CAreconstructedTrackLength", &m_rootCAreconstructedTrackLength);
//
//     m_treePtr->Branch("LostUClusters", &m_rootLostUClusters);
//     m_treePtr->Branch("LostVClusters", &m_rootLostVClusters);
//     m_treePtr->Branch("TotalMcUClusters", &m_rootTotalMcUClusters);
//     m_treePtr->Branch("TotalMcVClusters", &m_rootTotalMcVClusters);
//
//     m_treePtr->Branch("LostUClusterEDep", &m_rootLostUClusterEDep);
//     m_treePtr->Branch("LostVClusterEDep", &m_rootLostVClusterEDep);
//     m_treePtr->Branch("TotalMcUClusterEDep", &m_rootTotalMcUClusterEDep);
//     m_treePtr->Branch("TotalMcVClusterEDep", &m_rootTotalMcVClusterEDep);

  } else {
    m_rootFilePtr = NULL;
    m_treePtr = NULL;
  }
}


void TrackFinderVXDAnalizerModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(1, "################## entering TrackFinderVXDAnalizerModule - event " << m_eventCounter << " ######################");
  B2WARNING("################## entering TrackFinderVXDAnalizerModule - event " << m_eventCounter << " ######################");

//   RootVariables rootVariables; // storing all root related infos
//   m_forRootCountFoundIDs.clear(); // reset foundIDs-counter

  int nReferenceTCs = m_referenceTCs.getEntries();
  m_mcTrackCounter += nReferenceTCs;
  m_mcTrackVectorCounter += nReferenceTCs; // WARNING temporal solution!
  int nTestTCs = m_testTCs.getEntries();
  m_caTrackCounter += nTestTCs;


  /// collect all reference TCs
  unsigned nTC = 0;
  vector<AnalizerTCInfo> referenceTCVector;
  for (SpacePointTrackCand& aTC : m_referenceTCs) {
    vector<MCVXDPurityInfo > purities = createPurityInfos(aTC);

    MCVXDPurityInfo particleInfo = returnDominantParticleID(purities);
    B2DEBUG(25, "Calculating purities for reference TC " << nTC << ", name. Dominating iD " << particleInfo.getParticleID() <<
            " with purity " << particleInfo.getPurity().second << " was found among " << purities.size() << " assigned particles");

    AnalizerTCInfo referenceTC = AnalizerTCInfo::createTC(true, particleInfo, aTC);
    referenceTCVector.push_back(referenceTC);
    nTC++;
  }


  /// collect all test TCs
  nTC = 0;
  vector<AnalizerTCInfo> testTCVector;
  for (SpacePointTrackCand& aTC : m_testTCs) {
    vector<MCVXDPurityInfo > purities = createPurityInfos(aTC);

    MCVXDPurityInfo particleInfo = returnDominantParticleID(purities);
    B2DEBUG(25, "Calculating purities for test TC " << nTC << ", name. Dominating iD " << particleInfo.getParticleID() <<
            " with purity" << particleInfo.getPurity().second << " was found among " << purities.size() << " assigned particles");

    AnalizerTCInfo testTC = AnalizerTCInfo::createTC(false, particleInfo, aTC);
    testTCVector.push_back(testTC);
    nTC++;
  }


  /** for each test TC there will be a reference TC assigned to it.
   * This relation will be stored in a pair (.first == pointer to test TC, .second == pointer to reference TC.
   * if nothing found, a NULL pointer is stored at the according position).
   * in LostTCs for each unpaired Reference TC there will be attached a test TC (which will have below m_PARAMqiThreshold of purity) if found, or NULL if not.
   *
   * additionally all the relevant stuff will be counted.
   */
  vector<std::pair<AnalizerTCInfo*, AnalizerTCInfo*> > pairedTCs;
  for (AnalizerTCInfo& testTC : testTCVector) {
    int testID = testTC.assignedID.getParticleID();

    for (AnalizerTCInfo& referenceTC : referenceTCVector) {
      int refID = referenceTC.assignedID.getParticleID();
      B2DEBUG(50, "test TC with assigned ID " << testID << " was matched with refID " << refID)
      if (refID != testID) continue;

      testTC.tcType = AnalizerTCInfo::classifyTC(referenceTC, testTC, m_PARAMpurityThreshold, m_PARAMminNDFThreshold);

      B2DEBUG(50, "test TC with assigned ID " << testID <<
              " was classified for the corresponding with type for testTC/refTC: " << TCType::getTypeName(testTC.tcType) <<
              "/" << TCType::getTypeName(referenceTC.tcType) << " and will now paired up")

      referenceTC.pairUp(&testTC);

      pairedTCs.push_back({ &testTC, &referenceTC});
    }
  }

  /// find unpaired testTCs and mark them as Ghost
  AnalizerTCInfo::markUnused(testTCVector, TCType::Ghost);

  /// find unpaired referenceTCs and mark them as Lost
  AnalizerTCInfo::markUnused(referenceTCVector, TCType::Lost);


  /// fill counters:
  // good cases:
  unsigned int nPerfectTCs = 0, nCleanTCs = 0, nContaminatedTCs = 0, nFound = 0;
  // not so good cases:
  unsigned int nClonedTCs = 0, nSmallTCs = 0, nGhostTCs = 0, nLostTCs = 0;
  // bad cases: counts nTimes when the TC could not be identified/classified at all
  unsigned int nBadCases = 0;
  // hitCounters:
  unsigned int refPXDClusters = 0, refSVDClusters = 0, testPXDClusters = 0, testSVDClusters = 0;

  for (AnalizerTCInfo& aTC : testTCVector) {
    testPXDClusters += aTC.assignedID.getNPXDClustersTotal();
    testSVDClusters += aTC.assignedID.getNSVDUClustersTotal() + aTC.assignedID.getNSVDVClustersTotal();

    switch (aTC.tcType) {
      case TCType::Perfect: { nPerfectTCs++; nFound++; break; }
      case TCType::Clean: { nCleanTCs++; nFound++; break; }
      case TCType::Contaminated: { nContaminatedTCs++; nFound ++; break; }
      case TCType::Clone: { nClonedTCs++; break; }
      case TCType::SmallStump: { nSmallTCs++; break; }
      case TCType::Ghost: { nGhostTCs++; break; }
      case TCType::Lost: { nLostTCs++; break; }
      default: {
        nBadCases++;
        B2WARNING("TrackFinderVXDAnalizer::event(): test TC got type " << TCType::getTypeName(aTC.tcType) <<
                  " which is not counted for efficiency-calculation")
      }
    }
  }

  for (AnalizerTCInfo& aTC : referenceTCVector) {
    refPXDClusters += aTC.assignedID.getNPXDClustersTotal();
    refSVDClusters += aTC.assignedID.getNSVDUClustersTotal() + aTC.assignedID.getNSVDVClustersTotal();

    switch (aTC.tcType) {
      case TCType::Reference: { break; }
      case TCType::Lost: { nLostTCs++; break; }
      default: {
        nBadCases++;
        B2WARNING("TrackFinderVXDAnalizer::event(): reference TC got type " << TCType::getTypeName(aTC.tcType) <<
                  " which is not counted for efficiency-calculation")
      }
    }
  }


  /// providing some debug output
  B2DEBUG(1,
          "Event " << m_eventCounter <<
          ": the tested TrackFinder found total " << nFound <<
          " IDs (perfect/clean/contaminated/multipleFound/smallTCs/ghost: " << nPerfectTCs <<
          "/" << nCleanTCs <<
          "/" << nContaminatedTCs <<
          "/" << nClonedTCs <<
          "/" << nSmallTCs <<
          "/" << nGhostTCs <<
          ") within " << nTestTCs <<
          " TCs and lost " << nLostTCs <<
          " TCs. nBadCases: " << nBadCases <<
          "\n" <<
          "There are " << nReferenceTCs <<
          " referenceTCs, with mean of " << (float(refPXDClusters) / float(nReferenceTCs)) <<
          "/" << (float(refSVDClusters) / float(nReferenceTCs)) << " PXD/SVD clusters"
          "\n" <<
          "There are " << nTestTCs <<
          " testTCs, with mean of " << (float(testPXDClusters) / float(nTestTCs)) <<
          "/" << (float(testSVDClusters) / float(nTestTCs)) << " PXD/SVD clusters")

  m_totalRealHits += refPXDClusters + refSVDClusters;
  m_nCaPXDHits += testPXDClusters;
  m_nMcPXDHits += refPXDClusters;
  m_nCaSVDHits += testSVDClusters;
  m_nMcSVDHits += refSVDClusters;
  m_countReconstructedTCs += nFound;
  m_countedPerfectRecoveries += nPerfectTCs;
  m_countedCleanRecoveries += nCleanTCs;
  m_countedContaminatedRecoveries  += nContaminatedTCs;
  m_countedDoubleEntries += nClonedTCs;
  m_countedTCsTooShort  += nSmallTCs;
  m_countedGhosts  += nGhostTCs;

  B2INFO("TFAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter <<
         " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter <<
         " mcTrackCandidates where used for analysis because of cutoffs.")
  B2INFO("TFAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates, of those " << m_countAcceptedGFTCs <<
         " were stored in " << m_PARAMacceptedTCname << " and " << m_lostGFTCs << " lost TCs were stored in " << m_PARAMlostTCname <<
         " for further use, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter <<
         ", number of caTCs which produced a double entry: " << m_countedDoubleEntries)
  B2INFO("TFAnalizerModule:  totalCA|totalMC|ratio of pxdHits " << m_nCaPXDHits << "|" << m_nMcPXDHits << "|" << float(
           m_nCaPXDHits) / float(m_nMcPXDHits) <<
         ", svdHits " << m_nCaSVDHits << "|" << m_nMcSVDHits << "|" << float(m_nCaSVDHits) / float(m_nMcSVDHits) << " found by the two TFs")


  // TODO:
  /**
   * catch refTCs which are out of bounds (e.g. min/max-momentum-cuts):
   *  m_mcTrackVectorCounter
   * do the acceptedTCs-stuff:
   *  m_countAcceptedGFTCs
   *  m_PARAMacceptedTCname
   *  m_lostGFTCs
   *  m_PARAMlostTCname
   * do wrongCharargeSignCounter-stuff:
   *  m_wrongChargeSignCounter
   * do nRecoTCs (how?):
   *  m_countReconstructedTCs
   * */
//   int acceptedTCs = m_acceptedTCs.getEntries();
//   m_countAcceptedGFTCs += acceptedTCs;
//   m_lostGFTCs += nLostTCs;
//   B2DEBUG(1, " of " << nTestTCs << " TCs produced by the tested TrackFinder, " << acceptedTCs <<
//           " were recognized safely and stored into the container of accepted TCs, " << nLostTCs <<
//           " were lost and their MCTF-TCs were stored in lostTCs")
//   for (unsigned int ID : foundIDs) {
//     B2DEBUG(1, " - ID " << ID << " recovered")
//   }


  if (m_PARAMwriteToRoot == false) { return; }
  /// now deal with all the root-cases:
  // TODO:

  m_rootParameterTracker.prepareRoot(testTCVector);
  m_rootParameterTracker.prepareRoot(referenceTCVector);

  m_rootParameterTracker.fillRoot();




  /// for each vector<std::pair<TrackCandidate*, TrackCandidate*> >, fill the according Root-blabla and the accepted/lost TCs

//   m_rootTotalPXresiduals = rootVariables.totalPXresiduals;
//   m_rootTotalPYresiduals = rootVariables.totalPYresiduals;
//   m_rootTotalPZresiduals = rootVariables.totalPZresiduals;
//
//   m_rootCleanPXresiduals = rootVariables.cleanPXresiduals;
//   m_rootCleanPYresiduals = rootVariables.cleanPYresiduals;
//   m_rootCleanPZresiduals = rootVariables.cleanPZresiduals;
//
//   m_rootCompletePXresiduals = rootVariables.completePXresiduals;
//   m_rootCompletePYresiduals = rootVariables.completePYresiduals;
//   m_rootCompletePZresiduals = rootVariables.completePZresiduals;
//
//   m_rootTotalMCMomValues = rootVariables.totalMCMomValues;
//   m_rootTotalCAMomValues = rootVariables.totalCAMomValues;
//   m_rootCleanCAMomValues = rootVariables.cleanCAMomValues;
//   m_rootCompleteCAMomValues = rootVariables.completeCAMomValues;
//   m_rootTotalMomValues = rootVariables.totalMomValues;
//
//   m_rootTotalCAMomResiduals = rootVariables.totalCAMomResiduals;
//   m_rootCleanCAMomResiduals = rootVariables.cleanCAMomResiduals;
//   m_rootCompleteCAMomResiduals = rootVariables.completeCAMomResiduals;
//
//   m_rootTotalMCpTValues = rootVariables.totalMCpTValues;
//   m_rootTotalCApTValues = rootVariables.totalCApTValues;
//   m_rootCleanCApTValues = rootVariables.cleanCApTValues;
//   m_rootCompleteCApTValues = rootVariables.completeCApTValues;
//   m_rootTotalpTValues = rootVariables.totalpTValues;
//
//   m_rootTotalCApTResiduals = rootVariables.totalCApTResiduals;
//   m_rootCleanCApTResiduals = rootVariables.cleanCApTResiduals;
//   m_rootCompleteCApTResiduals = rootVariables.completeCApTResiduals;
//
//   m_rootTotalMCThetaValues = rootVariables.totalMCThetaValues;
//   m_rootTotalCAThetaValues = rootVariables.totalCAThetaValues;
//   m_rootCleanCAThetaValues = rootVariables.cleanCAThetaValues;
//   m_rootCompleteCAThetaValues = rootVariables.completeCAThetaValues;
//   m_rootTotalThetaValues = rootVariables.totalThetaValues;
//
//   m_rootTotalCAThetaResiduals = rootVariables.totalCAThetaResiduals;
//   m_rootCleanCAThetaResiduals = rootVariables.cleanCAThetaResiduals;
//   m_rootCompleteCAThetaResiduals = rootVariables.completeCAThetaResiduals;
//
//   m_rootTotalMCPhiValues = rootVariables.totalMCPhiValues;
//   m_rootTotalCAPhiValues = rootVariables.totalCAPhiValues;
//   m_rootCleanCAPhiValues = rootVariables.cleanCAPhiValues;
//   m_rootCompleteCAPhiValues = rootVariables.completeCAPhiValues;
//   m_rootTotalPhiValues = rootVariables.totalPhiValues;
//
//   m_rootTotalCAPhiResiduals = rootVariables.totalCAPhiResiduals;
//   m_rootCleanCAPhiResiduals = rootVariables.cleanCAPhiResiduals;
//   m_rootCompleteCAPhiResiduals = rootVariables.completeCAPhiResiduals;
//
//   m_rootTotalMCVertex2IP3DValues = rootVariables.totalMCVertex2IP3DValues;
//   m_rootTotalCAVertex2IP3DValues = rootVariables.totalCAVertex2IP3DValues;
//   m_rootCleanCAVertex2IP3DValues = rootVariables.cleanCAVertex2IP3DValues;
//   m_rootCompleteCAVertex2IP3DValues = rootVariables.completeCAVertex2IP3DValues;
//   m_rootTotalVertex2IP3DValues = rootVariables.totalVertex2IP3DValues;
//
//   m_rootTotalMCVertex2IPXYValues = rootVariables.totalMCVertex2IPXYValues;
//   m_rootTotalCAVertex2IPXYValues = rootVariables.totalCAVertex2IPXYValues;
//   m_rootCleanCAVertex2IPXYValues = rootVariables.cleanCAVertex2IPXYValues;
//   m_rootCompleteCAVertex2IPXYValues = rootVariables.completeCAVertex2IPXYValues;
//   m_rootTotalVertex2IPXYValues = rootVariables.totalVertex2IPXYValues;
//
//   m_rootTotalMCVertex2IPZValues = rootVariables.totalMCVertex2IPZValues;
//   m_rootTotalCAVertex2IPZValues = rootVariables.totalCAVertex2IPZValues;
//   m_rootCleanCAVertex2IPZValues = rootVariables.cleanCAVertex2IPZValues;
//   m_rootCompleteCAVertex2IPZValues = rootVariables.completeCAVertex2IPZValues;
//   m_rootTotalVertex2IPZValues = rootVariables.totalVertex2IPZValues;
//
//   m_rootTotalCAMomResidualsAngles = rootVariables.totalCAMomResidualAngles;
//   m_rootCleanCAMomResidualsAngles = rootVariables.cleanCAMomResidualAngles;
//   m_rootCompleteCAMomResidualsAngles = rootVariables.completeCAMomResidualAngles;
//
//   m_rootTotalCApTResidualsAngles = rootVariables.totalCApTResidualAngles;
//   m_rootCleanCApTResidualsAngles = rootVariables.cleanCApTResidualAngles;
//   m_rootCompleteCApTResidualsAngles = rootVariables.completeCApTResidualAngles;
//
//   m_rootTotalCASeedPositionResiduals = rootVariables.totalCASeedPositionResiduals;
//   m_rootCleanCASeedPositionResiduals = rootVariables.cleanCASeedPositionResiduals;
//   m_rootCompleteCASeedPositionResiduals = rootVariables.completeCASeedPositionResiduals;
//
//   m_rootMCreconstructedTrackLength = rootVariables.mCreconstructedTrackLength;
//   m_rootCAreconstructedTrackLength = rootVariables.cAreconstructedTrackLength;
//
//   m_rootLostUClusters = rootVariables.lostUClusters;
//   m_rootLostVClusters = rootVariables.lostVClusters;
//
//   m_rootTotalMcUClusters = rootVariables.totalMcUClusters;
//   m_rootTotalMcVClusters = rootVariables.totalMcVClusters;
//
//   m_rootLostUClusterEDep = rootVariables.lostUClusterEDep;
//   m_rootLostVClusterEDep = rootVariables.lostVClusterEDep;
//
//   m_rootTotalMcUClusterEDep = rootVariables.totalMcUClusterEDep;
//   m_rootTotalMcVClusterEDep = rootVariables.totalMcVClusterEDep;

//     m_treePtr->Fill();

//   B2DEBUG(10, "-------------------------------------------------------------------------------")
}


void TrackFinderVXDAnalizerModule::endRun()
{
  B2INFO("------------- >>>TrackFinderVXDAnalizerModule::endRun<<< -------------")
  B2DEBUG(1,
          "TFAnalizerModule-explanation: \n perfect recovery means: all hits of mc-TC found again and clean TC. \n clean recovery means: no foreign hits within TC. \n ghost means: QI was below threshold or mcTC was found more than once (e.g. because of curlers) \n found more than once means: that there was more than one TC which was assigned to the same mcTC but each of them were good enough for themselves to be classified as reconstructed")

  B2INFO("TFAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter <<
         " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter <<
         " mcTrackCandidates where used for analysis because of cutoffs.")
  B2INFO("TFAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates, of those " << m_countAcceptedGFTCs <<
         " were stored in " << m_PARAMacceptedTCname << " and " << m_lostGFTCs << " lost TCs were stored in " << m_PARAMlostTCname <<
         " for further use, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter <<
         ", number of caTCs which produced a double entry: " << m_countedDoubleEntries)
  B2INFO("TFAnalizerModule:  totalCA|totalMC|ratio of pxdHits " << m_nCaPXDHits << "|" << m_nMcPXDHits << "|" << float(
           m_nCaPXDHits) / float(m_nMcPXDHits) <<
         ", svdHits " << m_nCaSVDHits << "|" << m_nMcSVDHits << "|" << float(m_nCaSVDHits) / float(m_nMcSVDHits) << " found by the two TFs")

  B2INFO("TFAnalizerModule: the VXDTF found:\n" <<
         "Absolute numbers: total/perfect/clean/contaminated/clone/tooShort/ghost: " << m_countReconstructedTCs <<
         "/" << m_countedPerfectRecoveries <<
         "/" << m_countedCleanRecoveries <<
         "/" << m_countedContaminatedRecoveries <<
         "/" << m_countedDoubleEntries <<
         "/" << m_countedTCsTooShort <<
         "/" << m_countedGhosts <<
         " efficiency : total/perfect/clean/contaminated/clone/tooShort/ghost: " << double(100 * m_countReconstructedTCs) / double(
           m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedPerfectRecoveries) / double(m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedCleanRecoveries) / double(m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedContaminatedRecoveries) / double(m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedDoubleEntries) / double(m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedTCsTooShort) / double(m_mcTrackVectorCounter) <<
         "%/" << double(100 * m_countedGhosts) / double(m_mcTrackVectorCounter) <<
         "%")

  /// old:
//   B2INFO("TFAnalizerModule: the VXDTF found (total/perfect/clean/ghost)" << m_countReconstructedTCs << "/" <<
//          m_countedPerfectRecoveries << "/" << m_countedCleanRecoveries << "/" << (m_caTrackCounter - m_countReconstructedTCs) <<
//          " TCs -> efficiency(total/perfect/clean/ghost): " << double(100 * m_countReconstructedTCs) / double(
//            m_mcTrackVectorCounter) << "%/" << double(100 * m_countedPerfectRecoveries) / double(m_mcTrackVectorCounter) << "%/" << double(
//            100 * m_countedCleanRecoveries) / double(m_mcTrackVectorCounter) << "%/" << double(100 * (m_caTrackCounter -
//                m_countReconstructedTCs)) / double(m_countReconstructedTCs) << "%")

  amOasch();
}


void TrackFinderVXDAnalizerModule::terminate()
{

//   amOasch();
//
//   if (m_PARAMwriteToRoot == false) { return; }
//   m_rootParameterTracker.terminate();
//   if (m_treePtr != NULL) {
//     m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
//     m_treePtr->Write();
//     m_rootFilePtr->Close();
//   }
}


void TrackFinderVXDAnalizerModule::amOasch() { B2WARNING("bin amOasch!"); }
