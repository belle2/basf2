/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/GFTC2SPTCConverterModule.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <algorithm> // count, sort, etc...

#include <boost/tuple/tuple_comparison.hpp>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GFTC2SPTCConverter)

GFTC2SPTCConverterModule::GFTC2SPTCConverterModule() :
  Module()
{
  setDescription("Module for converting genfit::TrackCands (e.g. from TrackFinderMCTruth) to SpacePointTrackCands.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PXDClusters", m_PXDClusterName, "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name", string(""));
  addParam("genfitTCName", m_genfitTCName, "Name of container of genfit::TrackCands", string(""));
  addParam("SpacePointTCName", m_SPTCName,
           "Name of the container under which SpacePointTrackCands will be stored in the DataStore (NOTE: These SpaceTrackCands are not checked for curling behaviour, but are simply converted and stored!)",
           string(""));

  addParam("SingleClusterSVDSP", m_SingleClusterSVDSPName,
           "Single Cluster SVD SpacePoints collection name. NOTE: This StoreArray will be searched for SpacePoints only if 'useSingleClusterSP' is set to true!",
           string("SVDSpacePoints"));
  addParam("NoSingleClusterSVDSP", m_NoSingleClusterSVDSPName,
           "Non Single Cluster SVD SpacePoints collection name. This StoreArray will be searched for SpacePoints", string("SVDSpacePoints"));
  addParam("PXDClusterSP", m_PXDClusterSPName, "PXD Cluster SpacePoints collection name.", string("PXDSpacePoints"));

  addParam("minNDF", m_PARAMminNDF,
           "Minimum number of degrees of freedom a SpacePointTrackCand has to contain in order to get registered in the DataStore. If set to 0, any number is accepted",
           0);

  addParam("checkTrueHits", m_PARAMcheckTrueHits,
           "Set to true if you want TrueHits of Clusters forming a SpacePoint (e.g. SVD) to be checked for equality", false);
  addParam("useSingleClusterSP", m_PARAMuseSingleClusterSP,
           "Set to true if you want to use singleCluster SVD SpacePoints if no doubleCluster SVD SpacePoint can be found. NOTE: this gets overriden if 'skipCluster' is set to true!",
           true);
  addParam("checkNoSingleSVDSP", m_PARAMcheckNoSingleSVDSP,
           "Set to false if you want to disable the initial check for the StoreArray of Non Single Cluster SVD SpacePoints. NOTE: The module will still search for these SpacePoints first, so you have to make sure you are not registering SpacePoints under the StoreArray with the NoSingleClusterSVDSP name! (Disable the module that registers these SpacePoints)",
           true);
  addParam("skipCluster", m_PARAMskipCluster,
           "Set to true if you only want to skip the Clusters for which no appropriate SpacePoints can be found, instead of aborting the conversion of the whole GFTC when such a case occurs. NOTE: setting this to true automatically sets 'useSingleClusterSP' to false!",
           false);

  initializeCounters(); // NOTE: they get initialized in initialize again!!
}

// ------------------------------ INITIALIZE ---------------------------------------
void GFTC2SPTCConverterModule::initialize()
{
  B2INFO("GFTC2SPTCConverter -------------- initialize() ---------------------");
  // initialize Counters
  initializeCounters();

  // check if all required StoreArrays are here
  StoreArray<PXDCluster> PXDClusters(m_PXDClusterName); PXDClusters.isRequired(m_PXDClusterName);
  StoreArray<SVDCluster> SVDClusters(m_SVDClusterName); SVDClusters.isRequired(m_SVDClusterName);
  if (m_PARAMuseSingleClusterSP) {
    StoreArray<SpacePoint> SCSPs(m_SingleClusterSVDSPName);
    SCSPs.isRequired(m_SingleClusterSVDSPName);
  }
  if (m_PARAMcheckNoSingleSVDSP) {
    StoreArray<SpacePoint> nSCSPs(m_NoSingleClusterSVDSPName);
    nSCSPs.isRequired(m_NoSingleClusterSVDSPName);
  }
  StoreArray<SpacePoint> pxdSPs(m_PXDClusterSPName);
  pxdSPs.isRequired(m_PXDClusterSPName);

  StoreArray<genfit::TrackCand> gfTrackCand(m_genfitTCName);
  gfTrackCand.isRequired(m_genfitTCName);

  // registering StoreArray for SpacePointTrackCand
  StoreArray<SpacePointTrackCand> spTrackCand(m_SPTCName);
  spTrackCand.registerInDataStore(m_SPTCName, DataStore::c_ErrorIfAlreadyRegistered);

  // register Relation to genfit::TrackCand
  spTrackCand.registerRelationTo(gfTrackCand);

  // CAUTION: if the StoreArray of the TrueHits is named, this check fails!!!
  if (m_PARAMcheckTrueHits) {
    StoreArray<PXDTrueHit> PXDTrueHits; PXDTrueHits.isRequired();
    StoreArray<SVDTrueHit> SVDTrueHits; SVDTrueHits.isRequired();
  }

  if (m_PARAMminNDF < 0) {
    B2WARNING("'minNDF' is set to a value below 0. Resetting to 0!");
    m_PARAMminNDF = 0;
  }

}

// ------------------------------------- EVENT -------------------------------------------------------
void GFTC2SPTCConverterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "GFTC2SPTCConverter::event(). Processing event " << eventCounter << " --------");

  StoreArray<genfit::TrackCand> mcTrackCands(m_genfitTCName);
  StoreArray<SpacePointTrackCand> spacePointTrackCands(m_SPTCName); // output StoreArray

  StoreArray<PXDCluster> pxdClusters(m_PXDClusterName);
  StoreArray<SVDCluster> svdClusters(m_SVDClusterName);

  int nTCs = mcTrackCands.getEntries();

  B2DEBUG(15, "Found " << nTCs << " genfit::TrackCands in StoreArray " << mcTrackCands.getName());

  for (int iTC = 0; iTC < nTCs; ++iTC) {
    genfit::TrackCand* trackCand = mcTrackCands[iTC];
    m_genfitTCCtr += 1;

    B2DEBUG(10,
            "================================================================================\nNow processing genfit::TrackCand "
            << iTC << ".");
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 15, PACKAGENAME())) { trackCand->Print(); } // prints to stdout
    try {
      // get the converted SPTC
      std::pair<const SpacePointTrackCand, conversionStatus> spacePointTC = createSpacePointTC(trackCand, pxdClusters, svdClusters);
      // check if the SPTC contains enough SpacePoints
      if (spacePointTC.second == 0) {
        if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 50, PACKAGENAME())) spacePointTC.first.print(50);
        SpacePointTrackCand* newSPTC = spacePointTrackCands.appendNew(spacePointTC.first);
        m_SpacePointTCCtr++;
        newSPTC->addRelationTo(trackCand);
        B2DEBUG(10, "Added new SpacePointTrackCand to StoreArray " << spacePointTrackCands.getName());
      } else {
        B2DEBUG(10, "The conversion failed due to: " << spacePointTC.second);
        increaseFailCounter(spacePointTC.second);
      }
    } catch (std::runtime_error& anE) { // catch all Belle2 exceptions with this!
      B2ERROR("Caught exception in creation of SpacePointTrackCand: " << anE.what());
    } catch (...) { // catch the rest
      B2ERROR("Caught undefined exception in creation of SpacePointTrackCand!"); // COULDDO: rethrow exception as something is fishy if this happens
    }
  }
}

// -------------------------------- TERMINATE --------------------------------------------------------
void GFTC2SPTCConverterModule::terminate()
{
  stringstream generalOutput;
  generalOutput << "GFTC2SPTCConverter::terminate(): got " << m_genfitTCCtr << " GFTCs and created " << m_SpacePointTCCtr <<
                " SPTCs. ";
  if (m_abortedLowNDFCtr) generalOutput << "For " << m_abortedLowNDFCtr << " SPTCs the NDF was below " << m_PARAMminNDF << "\n";
  // NEW output
  B2INFO(generalOutput.str());
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME())) {
    stringstream verboseOutput;
    verboseOutput << "counter variables: ";
    verboseOutput << "abortedNoSP: " << m_abortedNoSPCtr << ", abortedUnsuitableGFTC: " << m_abortedUnsuitableTCCtr <<
                  ", abortedNoValidSP: " << m_abortedNoValidSPCtr;
    if (m_PARAMcheckTrueHits) verboseOutput << ", abortedTrueHit: " << m_abortedTrueHitCtr;
    if (m_PARAMskipCluster) {
      verboseOutput << ", skippedCluster: " << m_skippedCluster << ", skippedPXDnoSP: " << m_skippedPXDnoSPCtr;
      verboseOutput << ", skippedSVDnoSP: " << m_skippedSVDnoSPCtr << ", skippedPXDnoTH: " << m_skippedPXDnoTHCtr << ", skippedSVDnoTH: "
                    << m_skippedSVDnoTHCtr;
      verboseOutput << ", skippedPXDunsuitable: " << m_skippedPXDunsuitableCtr << ", skippedSVDunsuitable: " << m_skippedSVDunsuitableCtr;
      verboseOutput << ", skippedPXDnoValidSP " << m_skippedPXDnoValidSPCtr << ", skippedSVDnoValidSP " << m_skippedSVDnoValidSPCtr;
    }
    if (m_nonSingleSPCtr) verboseOutput << ", nonSingleSP " << m_nonSingleSPCtr;
    verboseOutput << ", noTwoClusterSP: " << m_noTwoClusterSPCtr << ", singleClusterSVDSP: " << m_singleClusterSPCtr;
    B2DEBUG(1, verboseOutput.str());
  }
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 2, PACKAGENAME())) {
    stringstream explanation;
    explanation << "explanation of counter variables (key words only):\n";
    explanation << "NoSP -> Found no related SpacePoint to a Cluster\n";
    explanation << "Unsuitable -> Cluster combination of SpacePoint was not in consecutive order in GFTC\n";
    explanation << "NoValidSP -> Cluster combination of SpacePoint was not contained in GFTC\n";
    if (m_PARAMcheckTrueHits) explanation << "TrueHit/noTH -> found no related TrueHit to a SpacePoint\n";
    if (m_nonSingleSPCtr) explanation << "nonSingleSP -> more than one singleCluster SpacePoint related to a Cluster\n";
    explanation << "noTwoClusterSP -> found no two Cluster SpacePoint\n";
    explanation << "singleClusterSVDSP -> number of tries to add a singleCluster SpacePoint for latter cases\n";
    B2DEBUG(2, explanation.str());
  }
}

// ================================================================= CREATE SPACEOINT TRACKCAND =========================================================================================
std::pair<const Belle2::SpacePointTrackCand, GFTC2SPTCConverterModule::conversionStatus>
GFTC2SPTCConverterModule::createSpacePointTC(const genfit::TrackCand* genfitTC, const StoreArray<PXDCluster>& pxdClusters,
                                             const StoreArray<SVDCluster>& svdClusters)
{
  m_NDF = 0; // reset for every trackCand
  std::vector<HitInfo<SpacePoint> > tcSpacePoints;
  conversionStatus convStatus = c_noFail; // part of return

  int nHits = genfitTC->getNHits();
  B2DEBUG(15, "genfit::TrackCand contains " << nHits << " hits");

  // for easier handling fill a taggedPair (typedef) to distinguish whether a hit has already been used or if is still open for procession
  std::vector<flaggedPair<int> > fHitIDs;
  for (int i = 0; i < nHits; ++i) {
    auto aHit = genfitTC->getHit(i);
    flaggedPair<int> aPair(false, aHit->getDetId(), aHit->getHitId());
    fHitIDs.push_back(aPair);
  }

  bool usedSingleCluster = false;
  // now loop over all hits and add them appropriately
  for (int iTCHit = 0; iTCHit < nHits; ++iTCHit) {
    genfit::TrackCandHit* aTCHit = genfitTC->getHit(iTCHit);
    double sortingParam = aTCHit->getSortingParameter();

    B2DEBUG(20, "Processing TrackCandHit " << iTCHit << " of " << nHits);
    if (fHitIDs[iTCHit].get<0>()) { // check if this hit has already been used, if not process
      B2DEBUG(60, "This hit has already been added to the SpacePointTrackCand via a SpacePoint and will not be processed again");
    } else {
      std::pair<SpacePoint*, conversionStatus> aSpacePoint = processTrackCandHit(aTCHit, pxdClusters, svdClusters, fHitIDs, iTCHit);

      // if there is more than one single Cluster SpacePoint related to a Cluster, add one
      if (aSpacePoint.first != NULL && (aSpacePoint.second >= 0 || aSpacePoint.second == c_nonSingleSP)) {
        if (aSpacePoint.second == c_singleClusterSP) usedSingleCluster = true;
        tcSpacePoints.push_back({sortingParam, aSpacePoint.first});
        B2DEBUG(60, "Added SpacePoint " << aSpacePoint.first->getArrayIndex() << " from Array " << aSpacePoint.first->getArrayName() <<
                " to tcSpacePoints");
        m_NDF += getNDF(aSpacePoint.first);
      } else {
        convStatus = aSpacePoint.second;
        B2DEBUG(60, "There was an error during conversion: for Hit " << iTCHit << ": " << convStatus);
        if (!m_PARAMskipCluster) {
          B2DEBUG(10,
                  "There was an error during conversion for a GFTC. 'skipCluster' is set to false, hence this trackCand will not be converted!");

          // return empty SPTC if there is a conversion error and splitCurlers is set to false
          return std::make_pair(SpacePointTrackCand(), convStatus);
        }
      }
    }
  }

  B2DEBUG(20, "NDF for this SpacePointTrackCand: " << m_NDF);
  if (m_NDF < m_PARAMminNDF) {
    B2DEBUG(10, "The created SpacePointTrackCand has not enough NDF: NDF is " << m_NDF << " but 'minNDF' is set to " << m_PARAMminNDF);
    return std::make_pair(SpacePointTrackCand(), c_lowNDF);
  }

  // check if all hits have been used
  bool usedAllHits = checkUsedAllHits(fHitIDs);
  if (!usedAllHits && !m_PARAMskipCluster) {
    B2WARNING("There is at least one TrackCandHit that has not been marked as used although 'skipCluster' is set to false"); // write warning here, because if this happens something has gone wrong
    throw UnusedHits();
  }

  // create a vector of SpacePoint* and one with sorting Parameters to add to the SpacePointTrackCand
  std::vector<const SpacePoint*> spacePoints;
  std::vector<double> sortingParams;
  for (const HitInfo<SpacePoint> aSP : tcSpacePoints) {
    spacePoints.push_back(aSP.second);
    sortingParams.push_back(aSP.first);
  }

  SpacePointTrackCand spacePointTC = SpacePointTrackCand(spacePoints, genfitTC->getPdgCode(), genfitTC->getChargeSeed(),
                                                         genfitTC->getMcTrackId());
  spacePointTC.set6DSeed(genfitTC->getStateSeed());
  spacePointTC.setCovSeed(genfitTC->getCovSeed());
  spacePointTC.setSortingParameters(sortingParams);

  if (m_PARAMcheckTrueHits) { spacePointTC.addRefereeStatus(SpacePointTrackCand::c_checkedTrueHits); }
  if (!usedAllHits) { spacePointTC.addRefereeStatus(SpacePointTrackCand::c_omittedClusters); }
  if (usedSingleCluster) { spacePointTC.addRefereeStatus(SpacePointTrackCand::c_singleClustersSPs); }

  return std::make_pair(spacePointTC, c_noFail);
}

// ============================================================================== PROCESS TRACKCANDHIT ===============================================================================================
std::pair<Belle2::SpacePoint*, GFTC2SPTCConverterModule::conversionStatus>
GFTC2SPTCConverterModule::processTrackCandHit(genfit::TrackCandHit* hit, const StoreArray<PXDCluster>& pxdClusters,
                                              const StoreArray<SVDCluster>& svdClusters,
                                              std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit)
{
  int detID = hit->getDetId();
  int hitID = hit->getHitId();
  int planeID = hit->getPlaneId(); // not used at the moment (except for debug output)
  B2DEBUG(60, "Processing TrackCandHit " << iHit << " with detID: " << detID << ", hitID: " << hitID << ", planeID: " << planeID);

  std::pair<SpacePoint*, conversionStatus> returnSP = { NULL, c_noFail }; // default return, optimistically assuming no fail

  if (detID == Const::PXD) {
    const PXDCluster* aCluster = pxdClusters[hitID];
    returnSP = getSpacePoint<PXDCluster, PXDTrueHit>(aCluster, flaggedHitIDs, iHit, true, m_PXDClusterSPName);
    if (m_PARAMskipCluster) { increaseSkippedCounter(returnSP.second, aCluster); } // have to do this here at the moment -> COULDDO; change increaseSkippedCounter, such that it can be used without a Cluster
    if (returnSP.second == c_singleClusterSP) returnSP.second = c_noFail; // getSpacePoint returns singleClusterSP for PXDs!
  } else if (detID == Const::SVD) {
    const SVDCluster* aCluster = svdClusters[hitID];
    returnSP = getSpacePoint<SVDCluster, SVDTrueHit>(aCluster, flaggedHitIDs, iHit, false, m_NoSingleClusterSVDSPName);
    if (m_PARAMskipCluster) { increaseSkippedCounter(returnSP.second, aCluster); }
  } else {
    throw SpacePointTrackCand::UnsupportedDetType();
  }
  return returnSP;
}

// ============================================================================ GET SPACEPOINT =======================================================================================================
template<typename ClusterType, typename TrueHitType>
std::pair<Belle2::SpacePoint*, GFTC2SPTCConverterModule::conversionStatus>
GFTC2SPTCConverterModule::getSpacePoint(const ClusterType* cluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit,
                                        bool singleCluster, std::string arrayName)
{
  std::pair<SpacePoint*, conversionStatus> spacePoint = {NULL, c_noFail}; // default return. be optimistic and assume that there are no problems!

  B2DEBUG(70, "Trying to find a related SpacePoint in StoreArray " << arrayName << " for Cluster " << cluster->getArrayIndex() <<
          " from Array " << cluster->getArrayName());
  RelationVector<SpacePoint> spacePoints = cluster->template getRelationsFrom<SpacePoint>(arrayName);
  B2DEBUG(80, "Found " << spacePoints.size() << " related SpacePoints for Cluster " << cluster->getArrayIndex() << " from Array " <<
          cluster->getArrayName()); // NOTE: .size == 0 handled later!

  if (singleCluster) { // if it is a singleCluster SpacePoint process different, then if it is a double Cluster SpacePoint
    if (spacePoints.size() == 0) {
      B2DEBUG(80, "Found no related (single Cluster) SpacePoint!");
      spacePoint.second = c_foundNoSpacePoint;
      return spacePoint;
    }
    if (spacePoints.size() > 1) {
      B2ERROR("More than one single Cluster SpacePoint related to a Cluster! Returning only the first in RelationVector!");
      spacePoint.second = c_nonSingleSP; // WARNING: returning first SpacePoint in RelationVector this way
      m_nonSingleSPCtr++;
    }
    spacePoint.first = spacePoints[0];
    spacePoint.second = c_singleClusterSP;
    markHitAsUsed(flaggedHitIDs, iHit); // mark hit as used
  } else {
    // try to get the appropriate double Cluster SVD SpacePoint
    spacePoint = findAppropriateSpacePoint<ClusterType>(spacePoints, flaggedHitIDs);
    if (spacePoint.first == NULL) {
      B2DEBUG(80, "Did not find an appropriate double Cluster SpacePoint for Cluster " << cluster->getArrayIndex() << " from Array " <<
              cluster->getArrayName() << ". Reason for failure: " << spacePoint.second);
      m_noTwoClusterSPCtr++;
      if (m_PARAMuseSingleClusterSP) {
        B2DEBUG(80, "Trying to get a single Cluster SpacePoint now!");
        m_singleClusterSPCtr++;
        // get single Cluster SVD PacePoint if desired. WARNING: hardcoded to SVD here at the moment!
        return getSpacePoint<ClusterType, TrueHitType>(cluster, flaggedHitIDs, iHit, true, m_SingleClusterSVDSPName);
      }
    }
  }

  if (m_PARAMcheckTrueHits && spacePoint.first != NULL) { // only do the TrueHit check if there is actually something to check
    if (!foundRelatedTrueHit<TrueHitType>(spacePoint.first)) { spacePoint.second = c_foundNoTrueHit; }
  }

  return spacePoint;
}

// ============================================================= FIND APPROPRIATE SPACEPOINT ===============================================================================
template<typename ClusterType>
std::pair<Belle2::SpacePoint*, GFTC2SPTCConverterModule::conversionStatus>
GFTC2SPTCConverterModule::findAppropriateSpacePoint(const Belle2::RelationVector<Belle2::SpacePoint>& spacePoints,
                                                    std::vector<flaggedPair<int> >& flaggedHitIDs)
{
  std::pair<SpacePoint*, conversionStatus> returnSP = { NULL, c_noFail }; // default return, be optimistc and assume that there are no problems
  B2DEBUG(100, "Trying to find an appropriate SpacePoint from RelationVector with " << spacePoints.size() << " entries!");
  if (spacePoints.size() == 0) {
    B2DEBUG(80, "There are no spacePoints to choose of!");
    returnSP.second = c_foundNoSpacePoint;
    return returnSP;
  }

  // WARNING: TEL cluster will do something wrong here!
  int detID = spacePoints[0]->getType() == VXD::SensorInfoBase::SVD ? Const::SVD : Const::PXD;

  // .first: Cluster Combination exists in the TrackCand, .second: Cluster Combination exists and has not yet been used
  std::vector<std::pair<bool, bool> >  existAndValidSP;
  // .first: index of SpacePoint in RelationVector, .second: Index of Cluster in the genfit::TrackCand
  std::vector<std::pair<int, int> > clusterPositions;

  // loop over all SpacePoints to look if their Cluster combination is vali (or existing but used, etc...)
  for (unsigned int iSP = 0; iSP < spacePoints.size(); ++iSP) {
    const SpacePoint* aSP = spacePoints[iSP];
    B2DEBUG(100, "Processing SpacePoint " << iSP + 1 << " of " << spacePoints.size() << " with Index " << aSP->getArrayIndex() <<
            " in StoreArray " << aSP->getArrayName());

    bool bothValid = true; // assume true, change to false if the second Cluster cannot be found in the genfit::TrackCand
    bool foundBoth = true; // assume true, change to false if the second Cluster can be found, but is already used

    std::vector<int> clusterInds = getClusterIndices<SVDCluster>(aSP, m_SVDClusterName);
    for (int index : clusterInds) {
      // get the valid and existing position for cluster. TODO: rename variable!
      std::pair<int, int> existAndValidClPos = checkExistAndValid(index, detID, flaggedHitIDs);
      if (existAndValidClPos.first < 0) { // check if cluster is valid and/or exists
        bothValid = false;
        if (existAndValidClPos.second < 0) foundBoth = false;
      }

      // push_back the index in the relationVector and the found valid position
      clusterPositions.push_back({iSP, existAndValidClPos.first});
      B2DEBUG(999, "clusterInd: " << index << " checkExistAndValid.first: " << existAndValidClPos.first << ", .second: " <<
              existAndValidClPos.second << " bothValid/foundBoth: " << bothValid << "/" << foundBoth);
    }
    // push_back the determined values
    existAndValidSP.push_back({foundBoth, bothValid});
    B2DEBUG(100, "Cluster combination of SpacePoint " << aSP->getArrayIndex() << " is contained in genfit::TrackCand: " << foundBoth <<
            ". SpacePoint is valid: " << bothValid);
  }

  int relVecPosition = getAppropriateSpacePointIndex(existAndValidSP, clusterPositions);
  if (relVecPosition < 0) {
    returnSP.second = getFailEnum(relVecPosition);
    return returnSP;
  }

  B2DEBUG(100, "SpacePoint " << spacePoints[relVecPosition]->getArrayIndex() <<
          " is the appropriate SpacePoint of all checked SpacePoints! The positions inside the GFTC are: " << clusterPositions.at(
            relVecPosition * 2).second << " and " << clusterPositions.at(relVecPosition * 2 + 1).second);
  markHitAsUsed(flaggedHitIDs, clusterPositions.at(relVecPosition * 2).second);
  markHitAsUsed(flaggedHitIDs, clusterPositions.at(relVecPosition * 2 + 1).second);
  returnSP.first = spacePoints[relVecPosition];

  return returnSP;
}

// ========================================================================= GET CLUSTER INDICES ===========================================================================
template<typename ClusterType>
std::vector<int> GFTC2SPTCConverterModule::getClusterIndices(const Belle2::SpacePoint* spacePoint, std::string storeArrayName)
{
  std::vector<int> clusterInds;
  RelationVector<ClusterType> relClusters = spacePoint->getRelationsTo<ClusterType>(storeArrayName);
  B2ASSERT("Too many clusters. There are " << relClusters.size() << " clusters.", relClusters.size() < 3);

  stringstream clusterStream;
  for (const ClusterType& aCluster : relClusters) {
    clusterInds.push_back(aCluster.getArrayIndex());
    clusterStream << aCluster.getArrayIndex() << " ";
  }

  B2DEBUG(499, "getClusterIndices(SpacePoint " << spacePoint->getArrayIndex() << "," << spacePoint->getArrayName() <<
          "): clusters are: " << clusterStream.str());

  return clusterInds;
}

// ========================================================================== CHECK EXIST AND VALID =============================================================================
std::pair<int, int> GFTC2SPTCConverterModule::checkExistAndValid(int clusterInd,  int detID,
    std::vector<flaggedPair<int> >& flaggedHitIDs)
{
  B2DEBUG(499, "Now checking if Cluster " << clusterInd << " is valid");
  std::pair<int, int> positions = { -1, -1}; // return Vector

  // flaggedPair for finding valid IDs: hit is in genfit::TrackCand and has not yet been used by another SpacePoint
  flaggedPair<int> validID(false, detID, clusterInd);
  flaggedPair<int> existingID(true, detID, clusterInd); // flaggedPair for finding existing but used fHitIDs

  // find the positions of these two pairs in flaggedHitIDs
  // position in "normal array indexing" (i.e. starting at 0, ending at vector.size() -1 )
  unsigned int validPos =  std::find(flaggedHitIDs.begin(), flaggedHitIDs.end(), validID) - flaggedHitIDs.begin();
  unsigned int existingPos = std::find(flaggedHitIDs.begin(), flaggedHitIDs.end(), existingID) - flaggedHitIDs.begin();

  B2DEBUG(100, "validID = (" << validID.get<1>() << "," << validID.get<2>() << "), found at position " << validPos <<
          ", existingID found at position " << existingPos << " of " << flaggedHitIDs.size());

  // check if these positions are still in the TrackCand
  if (validPos < flaggedHitIDs.size()) { positions.first = validPos; }
  if (existingPos < flaggedHitIDs.size()) { positions.second = existingPos; }

  B2DEBUG(999, "Return values, .first: " << positions.first << ", .second: " << positions.second);
  return positions;
}

// ============================================================================ GET NDF =============================================================================================
int GFTC2SPTCConverterModule::getNDF(Belle2::SpacePoint* spacePoint)
{
  if (spacePoint == NULL) {
    B2ERROR("Got NULL pointer to determine the NDF of!");
    return 0;
  }
  std::pair<bool, bool> assignedHits = spacePoint->getIfClustersAssigned();
  // if both are assigned -> NDF of SpacePoint is 2
  if (assignedHits.first && assignedHits.second) return 2;
  // if ONLY one is assigned -> NDF of SpacePoint is 1 (note the use of the bitwise XOR operator)
  if (assignedHits.first ^ assignedHits.second) return 1;
  return 0;
}

// =========================================================================== GET APPROPRIATE SPACEPOINT INDEX =====================================================================
int GFTC2SPTCConverterModule::getAppropriateSpacePointIndex(const std::vector<std::pair<bool, bool> >& existAndValidSPs,
                                                            const std::vector<std::pair<int, int> >& clusterPositions)
{
  // get the number of existing but used and the number of valid SpacePoints
  int nExistingButUsedSP = std::count(existAndValidSPs.begin(), existAndValidSPs.end(), std::make_pair(true, false));
  int nValidSP = std::count(existAndValidSPs.begin(), existAndValidSPs.end(), std::make_pair(true, true));

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 999, PACKAGENAME())) { // some very verbose output
    stringstream output;
    output << "content of passed vector of pairs (comma separated): ";
    for (auto entry : existAndValidSPs) { output << entry.first << "/" << entry.second << ", "; }
    B2DEBUG(999, output.str() << "nValidSP: " << nValidSP << " nExistingButUsedSP: " << nExistingButUsedSP);
  }

  // if there is no valid SpacePoint, but a SpacePoint with an existing but used cluster, throw, because conversion cannot be done properly then
  if (nValidSP < 1 && nExistingButUsedSP > 0) {
    B2DEBUG(80,
            "There are only Cluster Combinations where one of the Clusters is already used by another SpacePoint! This genfit::TrackCand cannot be converted properly to a SpacePointTrackCand!");
    return c_unsuitableGFTC;
  } else if (nValidSP < 1 && nExistingButUsedSP < 1) {
    B2DEBUG(120, "Found no valid SpacePoint and no SpacePoint with existing but used Clusters/Hits!");
    return c_noValidSP;
  }  // if there is at least one valid SpacePoint, check the position difference and then decide if the SP can be used!
  else if (nValidSP > 0) {
    // 1) index of SpacePoint in RelationVector, 2) squared position difference, 3) & 4) are positions inside genfit::TrackCand (valid positions)
    std::vector<std::pair<int, int> > positionInfos;

    for (unsigned int iSP = 0; iSP < existAndValidSPs.size(); ++iSP) {
      if (!existAndValidSPs.at(iSP).second) continue; // if not valid continue with next SpacePoint
      // sign doesnot matter, comparing squared values later only!
      int posDiff = clusterPositions.at(iSP * 2).second - clusterPositions.at(iSP * 2 + 1).second;

      B2DEBUG(200, "Difference of positions of Clusters for entry " << iSP << " is " << posDiff);
      positionInfos.push_back(std::make_pair(iSP, posDiff * posDiff));
    }

    // sort to find the smallest difference
    sort(positionInfos.begin(), positionInfos.end(), [](const pair<int, int>& lTuple, const pair<int, int>& rTuple) { return lTuple.second < rTuple.second; });

    if (positionInfos.at(0).second != 1) {
      B2DEBUG(80, "The shortest squared distance between two Clusters is " << positionInfos.at(0).second <<
              "! This would lead to wrong ordered TrackCandHits.");
      return c_unsuitableGFTC;
    }

    B2DEBUG(150, "SpacePoint with index " << positionInfos.at(0).first <<
            " is the valid SpacePoint with two Clusters in consecutive order from all valid SpacePoints.");
    return positionInfos.at(0).first;
  }

  return c_noValidSP; // default return is negative
}

// =============================================================================== CHECK USED ALL HITS ==========================================================================
bool GFTC2SPTCConverterModule::checkUsedAllHits(std::vector<flaggedPair<int> >& flaggedHitIDs)
{
  bool usedAll = true; // defining variable here so that all hits are checked (debug output)
  for (unsigned int i = 0; i < flaggedHitIDs.size(); ++i) {
    flaggedPair<int> fPair = flaggedHitIDs[i];
    B2DEBUG(200, "Hit " << i << " with (detID,hitID): (" << fPair.get<1>() << "," << fPair.get<2>() << ") has been used: " <<
            fPair.get<0>());
    if (!fPair.get<0>()) {
//       return false;
      usedAll = false;
    }
  }
//   return true;
  return usedAll;
}

// ================================================== FOUND RELATED TRUEHIT =================================================================
template <typename TrueHitType>
bool GFTC2SPTCConverterModule::foundRelatedTrueHit(const Belle2::SpacePoint* spacePoint, unsigned int allowedRelations)
{
  RelationVector<TrueHitType> relTrueHits = spacePoint->getRelationsTo<TrueHitType>("ALL"); // WARNING: searching in all relations
  if (relTrueHits.size() == 0) {
    B2DEBUG(100, "Found no TrueHit to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
    return false;
  }
  B2DEBUG(100, "Found " << relTrueHits.size() << " related TrueHits for SpacePoint " << spacePoint->getArrayIndex() << " from Array "
          << spacePoint->getArrayName());
  return (relTrueHits.size() <= allowedRelations);
}



void GFTC2SPTCConverterModule::markHitAsUsed(std::vector<flaggedPair<int> >& flaggedHitIDs, int hitToMark)
{
  flaggedHitIDs[hitToMark].get<0>() = true;
  flaggedPair<int> fPair = flaggedHitIDs[hitToMark];
  B2DEBUG(150, "Marked Hit " << hitToMark << " as used. (detID,hitID) of this hit is (" << fPair.get<1>() << "," << fPair.get<2>() <<
          ")");
}
