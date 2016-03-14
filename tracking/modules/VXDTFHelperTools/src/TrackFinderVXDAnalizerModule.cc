/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//fw:
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

//tracking:
#include <tracking/modules/VXDTFHelperTools/TrackFinderVXDAnalizerModule.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/PurityCalculatorTools.h>
#include <tracking/trackFindingVXD/analyzingTools/AnalizerTCInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/AnalyzingAlgorithmFactory.h>
#include <tracking/trackFindingVXD/analyzingTools/AlgoritmType.h>
#include <tracking/vectorTools/B2Vector3.h>

//root-stuff
#include <TVector3.h>



using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;



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


  vector< vector< vector< string> > > trackedParametersDouble = {
    {{"Contaminated"}, {"AnalyzingAlgorithmResidualP", "AnalyzingAlgorithmResidualPosition", "AnalyzingAlgorithmValuePT"}},
    {{"Clean"}, {"AnalyzingAlgorithmResidualP", "AnalyzingAlgorithmResidualPosition", "AnalyzingAlgorithmValuePT"}},
    {{"Perfect"}, {"AnalyzingAlgorithmResidualP", "AnalyzingAlgorithmResidualPosition", "AnalyzingAlgorithmValuePT"}}
  };

  vector< vector< vector< string> > > trackedParametersInt = {
    {{"Clean"}, {"AnalyzingAlgorithmLostUClusters", "AnalyzingAlgorithmLostVClusters"}}
  };


  vector< vector< vector< string> > > trackedParametersVecDouble = {
    {{"Clean"}, {"AnalyzingAlgorithmLostUEDep", "AnalyzingAlgorithmLostVEDep"}}
  };

  //Set module properties
  setDescription("analyzes quality of SpacePointTrackCands delivered by a test-TF compared to a reference TF");
  setPropertyFlags(c_ParallelProcessingCertified);


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
//   addParam("printExtentialAnalysisData", m_PARAMprintExtentialAnalysisData, "set true, if you want to cout special Info to the shell",
  //            bool(false)); // TODO outdated, to be deleted...

//   addParam("minTMomentumFilter", m_PARAMminTMomentumFilter,
//            "to narrow down the relevant mcTracks, this minFilter can be set to filter tracks having lower transverse momentum in GeV/c than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges - WARNING for some cases, this is a typical source for strange results!",
//            double(0.)); // TODO: shall they be re-implemented?
//   addParam("maxTMomentumFilter", m_PARAMmaxTMomentumFilter,
//            "to narrow down the relevant mcTracks, this maxFilter can be set to filter tracks having higher transverse momentum in GeV/c than this threshold. Relevant for checking efficiency of TFs with certain transverse momentum ranges - WARNING for some cases, this is a typical source for strange results!",
  //            double(500.)); // TODO: shall they be re-implemented?
  addParam("writeToRoot", m_PARAMwriteToRoot,
           " if true, analysis data is stored to root file with file name chosen by 'rootFileName'", bool(true));
  addParam("rootFileName", m_PARAMrootFileName,
           " only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot'=true  ",
           rootFileNameVals);

  addParam("trackedParametersDouble", m_PARAMtrackedParametersDouble,
           "set here all parameters to be tracked which use an algorithm storing one double per tc. Accepts a vector of vector of vector of strings of entries. Sample usage in steering file: param('trackedParametersDouble', [ [ ['Perfect'], ['AnalyzingAlgorithmValuePX', 'AnalyzingAlgorithmResidualP'] ] ]) first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType",
           trackedParametersDouble);

  addParam("trackedParametersInt", m_PARAMtrackedParametersInt,
           "set here all parameters to be tracked which use an algorithm storing one int per tc. Accepts a vector of vector of vector of strings of entries. Sample usage in steering file: param('trackedParametersDouble', [ [ ['Contaminated'], ['AnalyzingAlgorithmLostUClusters', 'AnalyzingAlgorithmLostVClusters'] ] ]) first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType",
           trackedParametersInt);

  addParam("trackedParametersVecDouble", m_PARAMtrackedParametersVecDouble,
           "set here all parameters to be tracked which use an algorithm storing one vector< double> per tc. Accepts a vector of vector of vector of strings of entries. Sample usage in steering file: param('trackedParametersDouble', [ [ ['Clean'], ['AnalyzingAlgorithmLostUEDep', 'AnalyzingAlgorithmLostVEDep'] ] ]) first innermost vector storest the TCType to be tracked, second the algorithms which shall be applied on that tcType",
           trackedParametersVecDouble);


  addParam("origin", m_PARAMorigin, " only allowed size: 3. stores coordinates of the origin used", {0, 0, 0});
  addParam("useMCDataForValues", m_PARAMuseMCDataForValues,
           "if true, for testTC the values of attached refTC will be stored instead of own values. - why are there values of the refTC stored? we want to know the real data, not the guesses of the reconstructed data. Deviations of reference values to guesses of the reconstructed data will be stored in resiudals anyway."
           , bool(true));

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
            << output)
  }

  // deal with algorithms:
  m_rootParameterTracker.initialize(m_PARAMrootFileName[0] + ".root", m_PARAMrootFileName[1]);
  // typedef for increased readability:
  using AlgorithmDouble = AnalyzingAlgorithmBase<double, AnalizerTCInfo, TVector3>;
  using AlgorithmInt = AnalyzingAlgorithmBase<int, AnalizerTCInfo, TVector3>;
  using AlgorithmVecDouble = AnalyzingAlgorithmBase<vector<double>, AnalizerTCInfo, TVector3>;


  // prepare all algorithms which store a double per tc:
  for (auto& parameterPackage : m_PARAMtrackedParametersDouble) {
    // check if parameterPackage has two entries: first is TCType, second is vector of algorithms
    if (parameterPackage.size() != 2) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersDouble' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    // check if vector for tcType is really only one entry
    if (parameterPackage.front().size() != 1) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersDouble' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    std::string tcTypeName = parameterPackage.front()[0];
    if (TCType::isValidName(tcTypeName) == false)
    { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersDouble' was mis-used! Please read the documentation! (invalid tcType: " << tcTypeName << ")") }

    // for each algorithm, store a branch:
    for (auto& algorithm : parameterPackage.back()) {
      if (AlgoritmType::isValidName(algorithm) == false)
      { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersDouble' was mis-used! Please read the documentation! (invalid algorithmType: " << algorithm << ")") }
      m_rootParameterTracker.addParameters4DoubleAlgorithms(tcTypeName, algorithm);
    }
  }
  AlgorithmDouble::setOrigin(TVector3(m_PARAMorigin[0], m_PARAMorigin[1], m_PARAMorigin[2]));
  AlgorithmDouble::setWillRefTCdataBeUsed4TestTCs(m_PARAMuseMCDataForValues);

  // prepare all algorithms which store an int per tc:
  for (auto& parameterPackage : m_PARAMtrackedParametersInt) {
    // check if parameterPackage has two entries: first is TCType, second is vector of algorithms
    if (parameterPackage.size() != 2) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersInt' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    // check if vector for tcType is really only one entry
    if (parameterPackage.front().size() != 1) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersInt' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    std::string tcTypeName = parameterPackage.front()[0];
    if (TCType::isValidName(tcTypeName) == false)
    { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersInt' was mis-used! Please read the documentation! (invalid tcType: " << tcTypeName << ")") }

    // for each algorithm, store a branch:
    for (auto& algorithm : parameterPackage.back()) {
      if (AlgoritmType::isValidName(algorithm) == false)
      { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersInt' was mis-used! Please read the documentation! (invalid algorithmType: " << algorithm << ")") }
      m_rootParameterTracker.addParameters4IntAlgorithms(tcTypeName, algorithm);
    }
  }
  AlgorithmInt::setOrigin(TVector3(m_PARAMorigin[0], m_PARAMorigin[1], m_PARAMorigin[2]));
  AlgorithmInt::setWillRefTCdataBeUsed4TestTCs(m_PARAMuseMCDataForValues);

  // prepare all algorithms which store a vector< double> per tc:
  for (auto& parameterPackage : m_PARAMtrackedParametersVecDouble) {
    // check if parameterPackage has two entries: first is TCType, second is vector of algorithms
    if (parameterPackage.size() != 2) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersVecDouble' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    // check if vector for tcType is really only one entry
    if (parameterPackage.front().size() != 1) { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersVecDouble' was mis-used! Please read the documentation! (wrong number of added parameters)") }

    std::string tcTypeName = parameterPackage.front()[0];
    if (TCType::isValidName(tcTypeName) == false)
    { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersVecDouble' was mis-used! Please read the documentation! (invalid tcType: " << tcTypeName << ")") }

    // for each algorithm, store a branch:
    for (auto& algorithm : parameterPackage.back()) {
      if (AlgoritmType::isValidName(algorithm) == false)
      { B2FATAL("TrackFinderVXDAnalizer::initialize(), parameter 'trackedParametersVecDouble' was mis-used! Please read the documentation! (invalid algorithmType: " << algorithm << ")") }
      m_rootParameterTracker.addParameters4VecDoubleAlgorithms(tcTypeName, algorithm);
    }
  }
  AlgorithmVecDouble::setOrigin(TVector3(m_PARAMorigin[0], m_PARAMorigin[1], m_PARAMorigin[2]));
  AlgorithmVecDouble::setWillRefTCdataBeUsed4TestTCs(m_PARAMuseMCDataForValues);
}


void TrackFinderVXDAnalizerModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "################## entering TrackFinderVXDAnalizerModule - event " << m_eventCounter << " ######################");

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
  B2DEBUG(10,
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

  B2DEBUG(11, "TFAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter <<
          " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter <<
          " mcTrackCandidates where used for analysis because of cutoffs.")
  B2DEBUG(11, "TFAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates, of those " << m_countAcceptedGFTCs <<
          " were stored in " << m_PARAMacceptedTCname << " and " << m_lostGFTCs << " lost TCs were stored in " << m_PARAMlostTCname <<
          " for further use, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter <<
          ", number of caTCs which produced a double entry: " << m_countedDoubleEntries)
  B2DEBUG(11, "TFAnalizerModule:  totalCA|totalMC|ratio of pxdHits " << m_nCaPXDHits << "|" << m_nMcPXDHits << "|" << float(
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

  m_rootParameterTracker.collectData(testTCVector);
  m_rootParameterTracker.collectData(referenceTCVector);

  m_rootParameterTracker.fillRoot();

}


void TrackFinderVXDAnalizerModule::endRun()
{
  B2INFO("------------- >>>TrackFinderVXDAnalizerModule::endRun<<< -------------")
  B2DEBUG(1,
          "TrackFinderVXDAnalizerModule-explanation: \n perfect recovery means: all hits of mc-TC found again and clean TC. \n clean recovery means: no foreign hits within TC. \n ghost means: QI was below threshold or mcTC was found more than once (e.g. because of curlers) \n found more than once means: that there was more than one TC which was assigned to the same mcTC but each of them were good enough for themselves to be classified as reconstructed")

  B2INFO("TrackFinderVXDAnalizerModule: After " << m_eventCounter + 1 << " events there was a total number of " << m_mcTrackCounter <<
         " mcTrackCandidates and " << m_totalRealHits << " realHits. Of these TCs, " << m_mcTrackVectorCounter <<
         " mcTrackCandidates where used for analysis because of cutoffs.")
  B2INFO("TrackFinderVXDAnalizerModule: There were " << m_caTrackCounter << " caTrackCandidates, of those " << m_countAcceptedGFTCs <<
         " were stored in " << m_PARAMacceptedTCname << " and " << m_lostGFTCs << " lost TCs were stored in " << m_PARAMlostTCname <<
         " for further use, number of times where charge was guessed wrong: " << m_wrongChargeSignCounter <<
         ", number of caTCs which produced a double entry: " << m_countedDoubleEntries)
  B2INFO("TrackFinderVXDAnalizerModule:  totalCA|totalMC|ratio of pxdHits " << m_nCaPXDHits << "|" << m_nMcPXDHits << "|" << float(
           m_nCaPXDHits) / float(m_nMcPXDHits) <<
         ", svdHits " << m_nCaSVDHits << "|" << m_nMcSVDHits << "|" << float(m_nCaSVDHits) / float(m_nMcSVDHits) <<
         " found by the two TFs (ghost hits not removed, therefore only useful if ghost-rate is low)")

  B2INFO("TrackFinderVXDAnalizerModule: the VXDTF found:\n" <<
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
}


void TrackFinderVXDAnalizerModule::terminate()
{

  if (m_PARAMwriteToRoot == false) { return; }
  m_rootParameterTracker.terminate();
}


