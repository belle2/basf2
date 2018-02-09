/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePoint2TrueHitConnectorModule.h>
#include <framework/datastore/StoreObjPtr.h>
// #include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/Environment.h> // getNumberProcesses

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>
#include <cmath>

#include <algorithm>
// #include <map>
#include <unordered_map>
#include <tracking/spacePointCreation/MapHelperFunctions.h> // map helper stuff

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;


REG_MODULE(SpacePoint2TrueHitConnector) // register the modules

SpacePoint2TrueHitConnectorModule::SpacePoint2TrueHitConnectorModule() :
  Module()
{
  setDescription("Module that tries to find the appropriate TrueHit to each SpacePoint and to register a relation between them for making MC information for SpacePoints more easily accesible for Modules that need it. Module can also be used to filter out 'fishy' SpacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("storeSeperate", m_PARAMstoreSeparate,
           "Set to false if you do not want to create seperate StoreArrays for processed SpacePoints. (i.e. a relation from SpacePoint to TrueHit will be set in the passed StoreArray. NOTE: this StoreArray will contain SpacePoints with a relation to TrueHits and such without after this module). The Names of the output StoreArrays will be the names of the input StoreArrays with 'outputSuffix' (module parameter) appended to them",
           true);

  addParam("registerAll", m_PARAMregisterAll,
           "If set to true, the module simply registers a relation for all TrueHits that are related to a SpacePoint (resp. its Clusters). In this way the module can be used to find all related TrueHits and then the user can decide what to do with these TrueHits (otherwise this module does some decision making). Setting this to true means that all checks (e.g. 'minWeight', 'maxPosSigma', ...) are ommitted! NOTE that some of the information is lost in this way (e.g. how many of the Clusters of a SpacePoint have been related to a TrueHit)!",
           false);

  addParam("positionAnalysis", m_PARAMpositionAnalysis,
           "Analyze the positions of SpacePoints and corresponding TrueHits. NOTE: if enabled a root file gets created!", false);

  addParam("requirePrimary", m_PARAMrequirePrimary,
           "Set to true if only relations to TrueHits that are related to a primary particle should get registered.", false);

  addParam("requireProximity", m_PARAMrequireProximity,
           "Require that the TrueHit is close to the SpacePoint (in local coordinates). The meaning of 'close' can be defined with the parameters 'maxPosSigma' and 'maxGlobalPosDiff'.",
           true);

  std::vector<std::string> defaultInList; // default list for input StoreArrays
  defaultInList.push_back(std::string(""));
  addParam("TrueHitNames", m_PARAMtrueHitNames,
           "Container names of TrueHits. NOTE: need one name per 'DetectorType' (i.e. unique entries in 'DetectorType)!", defaultInList);
  addParam("SpacePointNames", m_PARAMspacePointNames, "Container names of SpacePoints.", defaultInList);
  addParam("DetectorTypes", m_PARAMdetectorTypes,
           "detector types to determine which entries in 'TrueHitNames' and 'SpacePointNames' belong to which detector type. Entries have to be 'SVD' or 'PXD'. NOTE: if more 'SpacePointNames' than 'DetectorTypes' get passed, the last entry in 'DetectorTypes' is assumed to be valid for all remaining 'SpacePointNames'!");
  addParam("ClusterNames", m_PARAMclusterNames,
           "Container names of Clusters. NOTE: need one name per 'DetectorType' (i.e. unique entries in 'DetectorType')!", defaultInList);

  std::vector<std::string> defaultRootFName = { "PositionAnalysis", "RECREATE" };
  addParam("rootFileName", m_PARAMrootFileName,
           "Filename and write-mode ('RECREATE' or 'UPDATE'). If given more than 2 strings this module will cause termination",
           defaultRootFName);

  addParam("outputSuffix", m_PARAMoutputSuffix,
           "Suffix that will be appended to the container names if 'storeSeperate' is set to true", std::string("_relTH"));

  addParam("maxGlobalPosDiff", m_PARAMmaxGlobalDiff,
           "max difference of global position coordinates between TrueHit and SpacePoint (in each direction) in cm.", 0.05);
//   addParam("maxLocalPosDiff", m_PARAMmaxLocalDiff, "max difference of local position coordinates between TrueHit and SpacePoint (in U & V direction) in cm. NOTE: the default value is still subject to tuning and finding the appropriate value!", 0.01);

  addParam("maxPosSigma", m_PARAMmaxPosSigma, "Define the maximum local position difference in units of PitchSize / sqrt(12).", 4.);
  addParam("minWeight", m_PARAMminWeight,
           "Define a minimal weight a relation between a Cluster and a TrueHit has to have for the TrueHit to be considered as possible candidate.",
           0.);

  // initialize all counters
  initializeCounters();
  m_rootFilePtr = NULL;
  m_treePtr = NULL;

  if (m_PARAMpositionAnalysis == true and Environment::Instance().getNumberProcesses() > 0) {
    B2WARNING(
      "SpacePoint2TrueHitConnector::initialize: parameter positionAnalysis (and therefore root-output) is enabled and basf2 is running in multi-threaded mode - this can cause nondeterministic behavior! "
      << "\n"
      << " you can suppress multi-threading for this module by writing:"
      << "\n"
      << "main.add_module('SpacePoint2TrueHitConnector').set_property_flags(0) "
      << "\n"
      << "into the steering file!");
  }
}

// ================================================================ INITIALIZE ====================================================
void SpacePoint2TrueHitConnectorModule::initialize()
{
  B2INFO("SpacePoint2TrueHitConnector -------------------------- initialize --------------------------------");

  m_nContainers = m_PARAMspacePointNames.size(); // get number of passed arrays
  unsigned int nTHNames = m_PARAMtrueHitNames.size();
  unsigned int nDetTypes = m_PARAMdetectorTypes.size();
  unsigned int nClNames = m_PARAMclusterNames.size();
  if (m_nContainers < nTHNames || m_nContainers < nDetTypes || m_nContainers < nClNames) {
    B2FATAL("Passed " << nTHNames << " TrueHitNames and " << nDetTypes << " DetectorTypes but number of passed SpacePointArrays is " <<
            m_nContainers);
  }
  if ((nTHNames != nDetTypes) || (nClNames != nTHNames)) {
    B2FATAL("Passed " << nTHNames << " TrueHitNames and " << nClNames << "ClusterNames but " << nDetTypes << " DetectorTypes!");
  }

  for (unsigned int i = 0; i < m_nContainers; ++i) {
    if (i < nDetTypes) { // handle the detector types
      std::string detType = m_PARAMdetectorTypes.at(i);
      if (detType.compare(std::string("SVD")) != 0 && detType.compare(std::string("PXD")) != 0) {
        B2FATAL("Found entry " << detType << " in DetectorTypes, but only 'PXD' and 'SVD' are allowed!");
      }
      if (detType.compare(std::string("SVD")) == 0) {
        m_detectorTypes.push_back(c_SVD);
        m_SVDTrueHits = StoreArray<SVDTrueHit>(m_PARAMtrueHitNames.at(i));
        m_SVDTrueHits.isRequired();
        m_SVDClusters = StoreArray<SVDCluster>(m_PARAMclusterNames.at(i));
        m_SVDClusters.isRequired();
      } else {
        m_detectorTypes.push_back(c_PXD);
        m_PXDTrueHits = StoreArray<PXDTrueHit>(m_PARAMtrueHitNames.at(i));
        m_PXDTrueHits.isRequired();
        m_PXDClusters = StoreArray<PXDCluster>(m_PARAMclusterNames.at(i));
        m_PXDClusters.isRequired();
      }
    } else { m_detectorTypes.push_back(m_detectorTypes.at(i - 1)); } // add the last entry again

    m_inputSpacePoints.push_back(make_pair(StoreArray<SpacePoint>(m_PARAMspacePointNames.at(i)), m_detectorTypes.at(i)));
    m_inputSpacePoints.at(i).first.isRequired();

    // add counters
    m_SpacePointsCtr.push_back(0);
    m_nRelTrueHitsCtr.push_back(array<unsigned int, 5>());
    m_noClusterCtr.push_back(0);
    m_noTrueHitCtr.push_back(0);
    m_regRelationsCtr.push_back(0);
    m_ghostHitCtr.push_back(0);
    m_rejectedRelsCtr.push_back(0);
  }

  if (m_PARAMstoreSeparate) {
    if (m_PARAMoutputSuffix.empty()) {
      B2WARNING("'outputSuffix' is empty and 'storeSeperate' is set to true. This would lead to StoreArrays with the same name. Resetting to 'outputSuffix' to '_relTH'!");
      m_PARAMoutputSuffix = "_relTH";
    }

    for (unsigned int i = 0; i < m_nContainers; ++i) {
      std::string name = m_inputSpacePoints.at(i).first.getName() + m_PARAMoutputSuffix;
      m_outputSpacePoints.push_back(StoreArray<SpacePoint>(name));
      m_outputSpacePoints.at(i).registerInDataStore(name, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);

      // register relations (detector dependent)
      if (m_inputSpacePoints.at(i).second == c_SVD) {
        m_outputSpacePoints.at(i).registerRelationTo(m_SVDTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
        m_outputSpacePoints.at(i).registerRelationTo(m_SVDClusters, DataStore::c_Event, DataStore::c_DontWriteOut);
      } else {
        m_outputSpacePoints.at(i).registerRelationTo(m_PXDTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
        m_outputSpacePoints.at(i).registerRelationTo(m_PXDClusters, DataStore::c_Event, DataStore::c_DontWriteOut);
      }
    }
  } else {
    for (unsigned int i = 0; i < m_nContainers; ++i) {
      if (m_inputSpacePoints.at(i).second == c_SVD) {
        m_inputSpacePoints.at(i).first.registerRelationTo(m_SVDTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
      } else {
        m_inputSpacePoints.at(i).first.registerRelationTo(m_PXDTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
      }
    }
  }

  m_maxGlobalDiff = m_PARAMmaxGlobalDiff * m_PARAMmaxGlobalDiff; // only comparing squared values in module!
//   m_PARAMmaxLocalDiff *= m_PARAMmaxLocalDiff;

  if (m_PARAMmaxPosSigma < 0) {
    B2WARNING("'maxPosSigma' is set to a value below 0: " << m_PARAMmaxPosSigma << "! Resetting to default (4)!");
    m_PARAMmaxPosSigma = 4.;
  }

  if (m_PARAMpositionAnalysis) { initializeRootFile(); }
}

// ========================================================== EVENT ===============================================================
void SpacePoint2TrueHitConnectorModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCtr = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "SpacePoint2TrueHitConnector::event(). Processing event " << eventCtr << " -----------------------");

  if (m_PARAMpositionAnalysis) m_rootVariables = RootVariables(); // clear the rootVariables for every event

  // loop over all containers
  for (m_iCont = 0; m_iCont < m_nContainers; ++m_iCont) {
    StoreArray<SpacePoint> spacePoints = m_inputSpacePoints.at(m_iCont).first;
    const int nSpacePoints = spacePoints.getEntries();
    e_detTypes detType = m_inputSpacePoints.at(m_iCont).second;
    std::string detTypeStr = detType == c_SVD ? "SVD" : "PXD";
    B2DEBUG(10, "Found " << nSpacePoints << " SpacePoints in Array " << m_inputSpacePoints.at(m_iCont).first.getName() <<
            " for this event. detType: " << detTypeStr);

    m_SpacePointsCtr.at(m_iCont) += nSpacePoints;

    for (int iSP = 0; iSP < nSpacePoints; ++iSP) {
      SpacePoint* spacePoint = spacePoints[iSP];
      B2DEBUG(49, "Processing SpacePoint " << iSP << " from " << nSpacePoints);

      baseMapT trueHitMap = processSpacePoint<baseMapT>(spacePoint, detType);
      if (trueHitMap.empty()) continue; // next SpacePoint if something went wrong

      unsigned int nUniqueTHs = getUniqueSize(trueHitMap);
      B2DEBUG(50, "Found " << nUniqueTHs << " TrueHits (unique) related to SpacePoint " << spacePoint->getArrayIndex() << " from Array "
              << spacePoint->getArrayName());
      unsigned int iRels = nUniqueTHs > 4 ? 4 : nUniqueTHs - 1;
      m_nRelTrueHitsCtr.at(m_iCont).at(iRels)++;

      // print the complete map if the debug level is set high enough
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 350, PACKAGENAME())) {
        std::string mapCont = printMap(trueHitMap);
        B2DEBUG(250, "The TrueHits and their weights for spacePoint " << spacePoint->getArrayIndex() << ": " + mapCont);
      }

      int thIndex = -1; // declare negative and only assign a value if ONE relation gets registered! (see doc of positionAnalysis)
      if (m_PARAMregisterAll) {
        registerAllRelations(spacePoint, trueHitMap, detType);
      } else { // find THE ONE TrueHit (to rule them all, one TrueHit to find them all ...)
        // COULDDO: wrap this up in a function
        pair<VXDTrueHit*, double> trueHitwWeight = { NULL, 0.0 };

        if (detType == c_PXD) trueHitwWeight = getTHwithWeight<baseMapT, PXDTrueHit>(trueHitMap, m_PXDTrueHits, spacePoint, c_PXD);
        else trueHitwWeight = getTHwithWeight<baseMapT, SVDTrueHit>(trueHitMap, m_SVDTrueHits, spacePoint, c_SVD);

        if (trueHitwWeight.first != NULL) {
          registerOneRelation(spacePoint, trueHitwWeight, detType);
          thIndex = trueHitwWeight.first->getArrayIndex();
        } else {
          B2DEBUG(10, "Could not relate one TrueHit to SpacePoint " << spacePoint->getArrayIndex() << ".");
          m_rejectedRelsCtr.at(m_iCont)++;
        }
      }

      if (m_PARAMpositionAnalysis) { positionAnalysis(spacePoint, trueHitMap, thIndex, detType); }
    } // end loop SpacePoints
  } // end loop containers
  if (m_PARAMpositionAnalysis) { m_treePtr->Fill(); }
}

// ================================================================ TERMINATE ======================================================
void SpacePoint2TrueHitConnectorModule::terminate()
{
  unsigned int sumSpacePoints = accumulate(m_SpacePointsCtr.begin(), m_SpacePointsCtr.end(), 0);
  unsigned int sumRelations = accumulate(m_regRelationsCtr.begin(), m_regRelationsCtr.end(), 0);

  B2RESULT("SpacePoint2TrueHitConnector: Got " << sumSpacePoints << " SpacePoints in " << m_nContainers <<
           " containers and registered " << sumRelations << " relations to TrueHits");
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME())) {
    stringstream contSumm;
    contSumm << "Container-wise summary: \n";

    for (unsigned int iCont = 0; iCont < m_nContainers; ++iCont) {
      contSumm << "In Container " << iCont << " (container name: " << m_inputSpacePoints[iCont].first.getName() << ") " <<
               m_SpacePointsCtr[iCont]  << " SpacePoints were contained. " << m_regRelationsCtr[iCont] <<
               " relations were registered.\nNumber of related TrueHits to a SpacePoint are:\n";
      for (unsigned int i = 0; i < m_nRelTrueHitsCtr[iCont].size() - 1; ++i) { contSumm << i + 1 << " related TrueHits to a SpacePoint : " << m_nRelTrueHitsCtr[iCont].at(i) << "\n"; }
      contSumm << " more than 4 related TrueHits to a SpacePoint: " << m_nRelTrueHitsCtr[iCont].at(4) << "\n"; // WARNING: hardcoded
      contSumm << m_rejectedRelsCtr.at(iCont) << " SpacePoints did not get a relation, " << m_ghostHitCtr.at(
                 iCont) << " were probably ghost hits in this container!\n";
      contSumm << m_noTrueHitCtr[iCont] << " SpacePoints had no relation to a TrueHit at all.\n";
    }
    B2DEBUG(1, contSumm.str());

    if (!m_PARAMregisterAll) {
      // TODO: do this containerwise
      stringstream furtherSummary;
      furtherSummary << "Ommited Relations because of weight < " << m_PARAMminWeight << ": " << m_weightTooSmallCtr << "\n";
      furtherSummary << "Rejected Relations because of non primary particle: " << m_rejectedNoPrimaryCtr;
//       furtherSummary << "Summary for all containers:\n";
//       furtherSummary << "possible/accepted relations for cases:\n";
//       furtherSummary << m_all2WTHCtr << "/" << m_accAll2WTHCtr << " SP with THs (more than one) with all THs having two weights\n";
//       furtherSummary << m_single2WTHCtr << "/" << m_accSingle2WTHCtr << " SP with THs (more than one) with only one TH having two weights\n";
//       furtherSummary << m_nonSingle2WTHCtr << "/" << m_accNonSingle2WTHCtr << " SP with THs (more than one) with more than one but not all THs having two weights\n";
//       furtherSummary << "In " << m_oneCluster2THCtr << " cases there was a SP with only one Cluster but more than one related TrueHits";
      B2DEBUG(2, furtherSummary.str());
    }
  }

  if (m_PARAMpositionAnalysis) { closeRootFile(); }

//   B2INFO("total number of weights: " << m_totWeightsCtr << " of which " << m_negWeightCtr << " were negative")
//   B2INFO("m_moreThan2Weights = " << m_moreThan2Weights);
}

// ====================================================== PROCESS SPACEPOINT ======================================================
template<typename MapType>
MapType SpacePoint2TrueHitConnectorModule::processSpacePoint(Belle2::SpacePoint* spacePoint, e_detTypes detType)
{
  B2DEBUG(50, "Processing SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
  MapType trueHitMap;
  try {
    if (detType == c_PXD) {
      trueHitMap = getRelatedTrueHits<MapType, PXDCluster, PXDTrueHit>(spacePoint, m_PXDClusters.getName(), m_PXDTrueHits.getName());
    } else {
      trueHitMap = getRelatedTrueHits<MapType, SVDCluster, SVDTrueHit>(spacePoint, m_SVDClusters.getName(), m_SVDTrueHits.getName());
    }
  } catch (NoClusterToSpacePoint& anE) {
    B2WARNING("Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what());
    m_noClusterCtr.at(m_iCont)++;
  } catch (...) { // catch the rest
    B2ERROR("Caught undefined exception while trying to relate SpacePoints and TrueHits");
    throw; // throw further (maybe it is caught somewhere else)
  }

  B2DEBUG(499, "trueHitMap.size() before return in processSpacePoint: " << trueHitMap.size());
  return trueHitMap;
}

/////////////////////////////////////////////// GET RELATED TRUE HITS /////////////////////////////////////////////////////////////
template<typename MapType, typename ClusterType, typename TrueHitType>
MapType SpacePoint2TrueHitConnectorModule::getRelatedTrueHits(Belle2::SpacePoint* spacePoint, std::string clusterName,
    std::string trueHitName)
{
  MapType trueHitsMap; // map to be filled with indices (keys) and weights (values)

  RelationVector<ClusterType> spacePointClusters = spacePoint->getRelationsTo<ClusterType>(clusterName);
  if (spacePointClusters.size() == 0) {
    B2DEBUG(1, "Found no related Cluster for SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
            spacePoint->getArrayIndex());
    throw NoClusterToSpacePoint();
  }
  B2DEBUG(75, "Found " << spacePointClusters.size() << " related Clusters to SpacePoint " << spacePoint->getArrayIndex() <<
          " from Array " << spacePoint->getArrayName());

  // loop over all Clusters, get all TrueHits from them and add the information to the map
  short noTrueHits = 0;
  for (size_t iCl = 0; iCl < spacePointClusters.size(); ++iCl) {

    const ClusterType* cluster = spacePointClusters[iCl];
    RelationVector<TrueHitType> clusterTrueHits = cluster->template getRelationsTo<TrueHitType>(trueHitName);
    if (clusterTrueHits.size() == 0) {
      B2DEBUG(3, "Found no related TrueHit for Cluster " << cluster->getArrayIndex() << " contained by SpacePoint " <<
              spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
      noTrueHits++;
    } else {
      B2DEBUG(80, "Found " << clusterTrueHits.size() << " related TrueHits to Cluster " << cluster->getArrayIndex() << " from Array " <<
              cluster->getArrayName());

      for (unsigned int i = 0; i < clusterTrueHits.size(); ++i) { // 'TrueHit' loop
        if ((clusterTrueHits.weight(i) < m_PARAMminWeight) && !m_PARAMregisterAll) {
          m_weightTooSmallCtr++;
          continue; // not take into account relations with too little weight
        }
        int index = clusterTrueHits[i]->getArrayIndex();
        if (trueHitsMap.find(index) == trueHitsMap.end()) {  // create entry only if it is not already in the map
          trueHitsMap[index] = TrueHitInfo(index);
          B2DEBUG(499, "Added new TrueHitInfo to map. Index " << index);
        }
        if (spacePointClusters.weight(iCl) > 0) {
          trueHitsMap[index].setUWeight(clusterTrueHits.weight(i));
          B2DEBUG(499, "Added UCluster to TrueHitInfo with index " << index);
        } else {
          trueHitsMap[index].setVWeight(clusterTrueHits.weight(i));
          B2DEBUG(499, "Added VCluster to TrueHitInfo with index " << index);
        }
      }
    }
  }
  // check if there was no Cluster with a related TrueHit -> then throw
  if (noTrueHits == spacePoint->getNClustersAssigned()) {
    m_noTrueHitCtr[m_iCont]++; // increase counter for current container
  }
  return trueHitsMap;
}

// ================================================================= INITIALIZE COUNTERS ==========================================
void SpacePoint2TrueHitConnectorModule::initializeCounters()
{
  // NEW
  for (unsigned int i = 0; i < m_SpacePointsCtr.size(); ++i) {
    m_SpacePointsCtr.at(i) = 0;
    m_regRelationsCtr.at(i) = 0;
    m_noClusterCtr.at(i) = 0;
    m_ghostHitCtr.at(i) = 0;
    m_noTrueHitCtr.at(i) = 0;
    m_rejectedRelsCtr.at(i) = 0;

    for (unsigned int j = 0; j < m_nRelTrueHitsCtr.at(i).size(); ++j) { m_nRelTrueHitsCtr.at(i).at(j) = 0; }
  }

  m_weightTooSmallCtr = 0;
  m_rejectedNoPrimaryCtr = 0;

  // initialize the following here because of cppcheck complaining (not initialized in constructor)
  m_nContainers = 0;
  m_maxGlobalDiff = 0.;
  m_iCont = 0;

//   m_negWeightCtr = 0;
//   m_totWeightsCtr = 0;
//   m_moreThan2Weights = 0;
//
//   m_single2WTHCtr = 0;
//   m_nonSingle2WTHCtr = 0;
//   m_all2WTHCtr = 0;
//   m_accSingle2WTHCtr = 0;
//   m_accNonSingle2WTHCtr = 0;
//   m_accAll2WTHCtr = 0;
//
//   m_oneCluster2THCtr = 0;
//

}

// ===================================================== REGISTER ALL RELATIONS ===================================================
template<typename MapType>
void SpacePoint2TrueHitConnectorModule::registerAllRelations(Belle2::SpacePoint* spacePoint, MapType trueHitMap, e_detTypes detType)
{
  B2DEBUG(50, "Registering all possible relations for SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
          spacePoint->getArrayName() << ". storeSeparate is set to " << m_PARAMstoreSeparate);
  SpacePoint* newSP = spacePoint; // declaring pointer here, getting new pointer if storeSeparate is true

  if (m_PARAMstoreSeparate) { // if storing in separate Array, re-register the relations to the Clusters first
    newSP = m_outputSpacePoints.at(m_iCont).appendNew(*spacePoint);
    B2DEBUG(50, "Added new SpacePoint to Array " << m_outputSpacePoints[m_iCont].getName() << ".");
    if (detType == c_PXD) reRegisterClusterRelations<PXDCluster>(spacePoint, newSP, m_PXDClusters.getName());
    else reRegisterClusterRelations<SVDCluster>(spacePoint, newSP, m_SVDClusters.getName());
  }

  std::vector<TrueHitInfo> trueHitInfos = getAllValues(trueHitMap);
  // sort by the number of related Clusters first and than by weight (descending order)
  std::sort(trueHitInfos.begin(), trueHitInfos.end());

  for (const TrueHitInfo& info : trueHitInfos) {
    if (detType == c_PXD) registerTrueHitRelation<PXDTrueHit>(newSP, info.m_Id, 1, m_PXDTrueHits); // only one Cluster for PXDs!
    else { // for SVD relation weight is depending on which Cluster is related to TrueHit!
      double weight = calculateRelationWeight(info, spacePoint);
      registerTrueHitRelation<SVDTrueHit>(newSP, info.m_Id, weight, m_SVDTrueHits);
    }
  }
}

// =============================================== REGISTER ONE RELATION ==========================================================
template<typename TrueHitType>
void SpacePoint2TrueHitConnectorModule::registerOneRelation(Belle2::SpacePoint* spacePoint,
                                                            std::pair<TrueHitType*, double> trueHitwWeight, e_detTypes detType)
{
  TrueHitType* trueHit = trueHitwWeight.first;
  B2DEBUG(50, "Registering relation to TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName());
  SpacePoint* newSP = spacePoint; // declaring pointer here, getting new pointer if storeSeparate ist true

  if (m_PARAMstoreSeparate) { // if storing in separate Array, re-register the relations to the Clusters first
    newSP = m_outputSpacePoints.at(m_iCont).appendNew(*spacePoint);
    B2DEBUG(50, "Added new SpacePoint to Array " << m_outputSpacePoints[m_iCont].getName() << ".");
    if (detType == c_PXD) reRegisterClusterRelations<PXDCluster>(spacePoint, newSP, m_PXDClusters.getName());
    else reRegisterClusterRelations<SVDCluster>(spacePoint, newSP, m_SVDClusters.getName());
  }

  newSP->addRelationTo(trueHit, trueHitwWeight.second);
  m_regRelationsCtr.at(m_iCont)++;
  B2DEBUG(50, "Added Relation to TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName() <<
          " for SpacePoint " << spacePoint->getArrayIndex() << " (weight = " << trueHitwWeight.second << ")");
}

// ========================================================= REREGISTER CLUSTER RELATIONS =========================================
template<typename ClusterType>
void SpacePoint2TrueHitConnectorModule::reRegisterClusterRelations(Belle2::SpacePoint* origSpacePoint,
    Belle2::SpacePoint* newSpacePoint, std::string clusterName)
{
  B2DEBUG(100, "Registering the Relations to Clusters of SpacePoint " << origSpacePoint->getArrayIndex() << " in Array " <<
          origSpacePoint->getArrayName() << " for SpacePoint " << newSpacePoint->getArrayIndex() << " in Array " <<
          newSpacePoint->getArrayName());

  vector<pair<ClusterType*, double> > clustersAndWeights = getRelatedClusters<ClusterType>(origSpacePoint, clusterName);
  for (auto aCluster : clustersAndWeights) {
    newSpacePoint->addRelationTo(aCluster.first, aCluster.second);
    B2DEBUG(100, "Registered Relation to Cluster " << aCluster.first->getArrayIndex() << " with weight " << aCluster.second);
  }
}

// =========================================================== GET RELATED CLUSTERS ===============================================
template<typename ClusterType>
std::vector<std::pair<ClusterType*, double> > SpacePoint2TrueHitConnectorModule::getRelatedClusters(Belle2::SpacePoint* spacePoint,
    std::string clusterName)
{
  vector<pair<ClusterType*, double> > indsAndWeights;
  RelationVector<ClusterType> relClusters = spacePoint->getRelationsTo<ClusterType>(clusterName);

  for (unsigned int iCl = 0; iCl < relClusters.size(); ++iCl) {
    indsAndWeights.push_back(make_pair(relClusters[iCl], relClusters.weight(iCl)));
  }

  // safety measure, should not / cannot happen (checked before)
  if (indsAndWeights.empty()) { B2ERROR("No Clusters related to SpacePoint " << spacePoint->getArrayIndex() << "!"); }

  return indsAndWeights;
}

// ====================================================== REGISTER TRUEHIT RELATIONS ==============================================
template<typename TrueHitType>
void SpacePoint2TrueHitConnectorModule::registerTrueHitRelation(Belle2::SpacePoint* spacePoint, int index, double weight,
    Belle2::StoreArray<TrueHitType> trueHits)
{
  TrueHitType* trueHit = trueHits[index];
  spacePoint->addRelationTo(trueHit, weight);
  m_regRelationsCtr.at(m_iCont)++; // increase counter of registered relations for this container
  B2DEBUG(50, "Added Relation to TrueHit " << index << " from Array " << trueHits.getName() << " for SpacePoint " <<
          spacePoint->getArrayIndex() << " (weight = " << weight << ")");
}

// ====================================================== POSITION ANALYSIS =======================================================
// TODO: debug output
template<typename MapType>
void SpacePoint2TrueHitConnectorModule::positionAnalysis(Belle2::SpacePoint* spacePoint, const MapType& trueHitMap,
                                                         const int& index, e_detTypes detType)
{
  B2DEBUG(250, "Doing position analysis for SpacePoint "  << spacePoint->getArrayIndex() << " from Array " <<
          spacePoint->getArrayName());
  simpleBitfield<unsigned short int> relationStatus = simpleBitfield<unsigned short int>();

  // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
  pair<unsigned short int, unsigned short int> clusterSizes = getClusterSizes(spacePoint, detType);
  pair<double, double> positionError = getLocalError(spacePoint);
  // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)

  // do some checks and set the relationStatus
  pair<bool, bool> setUV = spacePoint->getIfClustersAssigned();
  if (setUV.first) relationStatus.addStatus(c_SpacePointU);
  if (setUV.second) relationStatus.addStatus(c_SpacePointV);

  const vector<TrueHitInfo> trueHitInfos = getAllValues(trueHitMap);
  unsigned int nRelations = trueHitInfos.size();
  if (nRelations != 1) relationStatus.addStatus(c_nonUniqueRelation);
  else relationStatus.addStatus(c_clearHit); // TODO: check if this is according to the definition of cleanHit

  B2DEBUG(999, "SpacePoint has assigned U: " << setUV.first << ", V: " << setUV.second << ". Possible TrueHits: " << nRelations);

  pair<double, double> spLocalPos = getLocalPos(spacePoint);
  unsigned short int vxdId = spacePoint->getVxdID();
  short nClusters = spacePoint->getNClustersAssigned();

  // loop over all TrueHitInfos and check if it is a ghostHit, and if a noise hit is contained in the SpacePoint
  bool onceU = false, onceV = false; // check if both Clusters are at least used once
  bool twoClusters = false;
  for (const TrueHitInfo& info : trueHitInfos) {
    onceU = onceU || info.m_U; // if U is set for one of the possible TrueHits, onceU is true after this loop
    onceV = onceV || info.m_V;
    twoClusters = twoClusters || (info.getNClusters() == 2); // if one of the TrueHits has two Clusters in the SpacePoint -> true
  }
  bool allSet = onceU && onceV;

  if (nClusters > 1) { // at this stage noise Clusters and ghostHits can only appear if there are two Clusters in the SpacePoint
    if (!allSet) relationStatus.addStatus(c_noiseCluster);
    if (allSet && !twoClusters) relationStatus.addStatus(c_ghostHit);
  }

  // get the status that has been set until now as from now on it can differ for every relation
  unsigned short int overAllStatus = relationStatus.getStatus();

  for (const TrueHitInfo& info : trueHitInfos) {
    relationStatus.setStatus(overAllStatus); // reset status
    VXDTrueHit* trueHit;
    if (detType == c_SVD) { trueHit = m_SVDTrueHits[info.m_Id]; }
    else { trueHit = m_PXDTrueHits[info.m_Id]; }
    pair<TVector3, TVector3> trueHitPos = getTrueHitPositions(trueHit);

    double weightU = info.m_wU;
    double weightV = info.m_wV;

    MCParticle* mcParticle = trueHit->getRelatedFrom<MCParticle>("ALL");
    if (mcParticle != NULL) {
      if (mcParticle->hasStatus(MCParticle::c_PrimaryParticle)) relationStatus.addStatus(c_primaryParticle);
    }

    if (info.m_Id == index) relationStatus.addStatus(c_registeredRelation);

    m_rootVariables.SpacePointULocal.push_back(spLocalPos.first);
    m_rootVariables.SpacePointVLocal.push_back(spLocalPos.second);
    m_rootVariables.SpacePointXGlobal.push_back(spacePoint->X());
    m_rootVariables.SpacePointYGlobal.push_back(spacePoint->Y());
    m_rootVariables.SpacePointZGlobal.push_back(spacePoint->Z());

    m_rootVariables.TrueHitULocal.push_back(trueHitPos.first.X());
    m_rootVariables.TrueHitVLocal.push_back(trueHitPos.first.Y());
    m_rootVariables.TrueHitXGlobal.push_back(trueHitPos.second.X());
    m_rootVariables.TrueHitYGlobal.push_back(trueHitPos.second.Y());
    m_rootVariables.TrueHitZGlobal.push_back(trueHitPos.second.Z());

    m_rootVariables.NRelations.push_back(nRelations);
    m_rootVariables.RelationStatus.push_back(relationStatus.getStatus());
    m_rootVariables.WeightU.push_back(weightU);
    m_rootVariables.WeightV.push_back(weightV);
    m_rootVariables.HitVxdID.push_back(vxdId);

    // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
    m_rootVariables.ClusterSizeU.push_back(clusterSizes.first);
    m_rootVariables.ClusterSizeV.push_back(clusterSizes.second);
    m_rootVariables.SpacePointErrorU.push_back(positionError.first);
    m_rootVariables.SpacePointErrorV.push_back(positionError.second);
    m_rootVariables.SpacePointErrorX.push_back(spacePoint->getPositionError().X());
    m_rootVariables.SpacePointErrorY.push_back(spacePoint->getPositionError().Y());
    m_rootVariables.SpacePointErrorZ.push_back(spacePoint->getPositionError().Z());
    // TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)

    B2DEBUG(999, "Branch contents of this entry:\nSPLocalU: " << spLocalPos.first << ". SPLocalV: " << spLocalPos.second << "\n" << \
            "SPGlobalX: " << spacePoint->X() << ", SPGlobalY: " << spacePoint->Y() << ", SPGlobalZ " << spacePoint->Z() << "\n" << \
            "THLocalU: " << trueHitPos.first.X() << ", THLocalV: " << trueHitPos.first.Y() << "\n" << \
            "THGlobalX: " << trueHitPos.second.X() << ", THGlobalY: " << trueHitPos.second.Y() << ", THGlobalZ: " << trueHitPos.second.Z() <<
            "\n" << \
            "weight1: " << weightU << ", weight2: " << weightV << ", VxdID: " << vxdId << ", nRelations: " << nRelations << ", relStatus: " <<
            relationStatus.getStatus());
  }
}

// ====================================================== GET TH WITH WEIGHT ======================================================
template<typename MapType, typename TrueHitType>
std::pair<TrueHitType*, double>
SpacePoint2TrueHitConnectorModule::getTHwithWeight(const MapType& aMap, Belle2::StoreArray<TrueHitType> trueHits,
                                                   Belle2::SpacePoint* spacePoint, e_detTypes detType)
{
  vector<TrueHitInfo> trueHitInfos = getAllValues(aMap);
  std::pair<TrueHitType*, double> THwithWeight(NULL, 0.0); // default return value
  // return NULL pointer and zero weight if there is no TrueHit (safety measure that should not actually be needed!)
  if (trueHitInfos.empty()) return THwithWeight;
  std::sort(trueHitInfos.begin(), trueHitInfos.end()); // sort to have best candidates at beginning

  short nClusters = spacePoint->getNClustersAssigned();
  size_t nRelations = trueHitInfos.size(); // get the number of possible relations

  B2DEBUG(50, "Trying to select one TrueHit for SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
          spacePoint->getArrayName() << ". SpacePoint has " << nClusters << " Clusters.");
  B2DEBUG(150, "There are " << nRelations << " possible candidates.");

  // very verbose output only to have a look on why these TrueHits could be in the same SpacePoint, COULDDO: wrap up in function
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 999, PACKAGENAME())) {
    stringstream output;
    output << "The candidates are: ";
    for (auto& info : trueHitInfos) { output << info; }
    B2DEBUG(999, output.str());

    std::pair<double, double> spacePointLocal = getLocalPos(spacePoint);
    std::pair<bool, bool> assignedLocal = spacePoint->getIfClustersAssigned();

    B2DEBUG(999, "SpacePoint " << spacePoint->getArrayIndex() << " U: " << spacePointLocal.first << " V: " << spacePointLocal.second <<
            " assigned: " << assignedLocal.first << ", " << assignedLocal.second);

    std::vector<int> uniqueKeys = getUniqueKeys(aMap);
    for (int key : uniqueKeys) {
      TrueHitType* trueHit = trueHits[key];
      // NOTE: assuming here that there is only one MCParticle to each TrueHit
      MCParticle* mcParticle = trueHit->template getRelatedFrom<MCParticle>("ALL");

      int mcPartId = -1, pdgCode = 0;
      bool primary = false;

      if (mcParticle != NULL) {
        mcPartId = mcParticle->getArrayIndex();
        primary = mcParticle->hasStatus(MCParticle::c_PrimaryParticle);
        pdgCode = mcParticle->getPDG();
      }

      B2DEBUG(999, "TrueHit " << key << " U: " << trueHit->getU() << ", V: " << trueHit->getV() << " mc Particle Id: " << mcPartId <<
              ", primary " << primary << ", pdg: " << pdgCode);
    }
  }

  // loop over all TrueHitInfos and check if the number of related Clusters is equal to the number of the number of the assigned
  // Clusters in the SpacePoint -> if so, check for compatibility and return (the first that is compatible)
  bool ghostHit = false;
  for (const TrueHitInfo& info : trueHitInfos) {
    B2DEBUG(499, "Now checking trueHit: " << info);
    if (nClusters == info.getNClusters()) {
      TrueHitType* trueHit = trueHits[info.m_Id];
      if (compatibleCombination(spacePoint, trueHit)) {
        double weight = (detType == c_PXD ? 1 : calculateRelationWeight(info, spacePoint)); // if PXD weight is always 1!
        return make_pair(trueHit, weight);
      }
    } else {
      B2DEBUG(499, "The number of related Clusters ( = " << nClusters << ") and the number of associated weights ( = " <<
              info.getNClusters() << ") do not match! This indicates a ghost hit");
      ghostHit = true;
    }
  }
  if (ghostHit) m_ghostHitCtr[m_iCont]++;

  // B2DEBUG(4999, "just before return in getTHwithWeight")
  return THwithWeight;
}

// ============================================== CHECK IF SPACEPOINT AND TRUEHIT ARE COMPATIBLE ==================================
template <typename TrueHitType>
bool SpacePoint2TrueHitConnectorModule::compatibleCombination(Belle2::SpacePoint* spacePoint, TrueHitType* trueHit)
{
  B2DEBUG(150, "Checking TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName() << " and SpacePoint " <<
          spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " for compatibility");

  // check primary first
  MCParticle* mcPart = trueHit->template getRelatedFrom<MCParticle>("ALL");
  bool primaryPart = false; // assume secondary or background for safety
  if (mcPart != NULL) primaryPart = mcPart->hasStatus(MCParticle::c_PrimaryParticle);
  if (m_PARAMrequirePrimary && !primaryPart) {
    B2DEBUG(150, "TrueHit is not related to a primary particle but 'requirePrimary' is set to true!");
    m_rejectedNoPrimaryCtr++;
    return false;
  }

  // CAUTION: if further tests are added, make sure that they do not get 'over-ruled' by this one (i.e. put before this if)
  if (!m_PARAMrequireProximity) {
    B2DEBUG(999, "Not checking positions because 'requireProximity' is set to false");
    return true;
  }
  const VxdID spacePointVxdId = spacePoint->getVxdID();
  const VxdID trueHitVxdId = trueHit->getSensorID();

  B2DEBUG(999, "Comparing the VxdIDs, SpacePoint: " << spacePointVxdId << ", TrueHit: " << trueHitVxdId);
  if (spacePointVxdId != trueHitVxdId) {
    B2DEBUG(150, "SpacePoint and TrueHit do not have the same VxdID. spacePoint: " << spacePointVxdId << ", trueHit: " << trueHitVxdId);
    return false;
  }

  VXD::SensorInfoBase SensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(
                                         trueHitVxdId); // only have to get one, since VxdIds are already the same at this point!
  double maxUres = SensorInfoBase.getUPitch(trueHit->getV()) / sqrt(12.) * m_PARAMmaxPosSigma;
  double maxVres = SensorInfoBase.getVPitch(trueHit->getV()) / sqrt(12.) * m_PARAMmaxPosSigma;

  B2DEBUG(999, "maximum residual in U: " << maxUres << ", in V: " << maxVres);

  const TVector3 trueHitLocalPos = TVector3(trueHit->getU(), trueHit->getV(), 0);
  const TVector3 trueHitGlobalPos = SensorInfoBase.pointToGlobal(trueHitLocalPos);

  std::pair<double, double> spacePointLocal = getLocalPos(spacePoint);
  // compare only those values of the local coordinates that have been set
  std::pair<bool, bool> setCoordinates = spacePoint->getIfClustersAssigned();

  if (setCoordinates.first) {
    B2DEBUG(999, "Comparing the U-coordinates, SpacePoint: " << spacePointLocal.first << ", TrueHit: " << trueHitLocalPos.X() <<
            " -> diff: " << spacePointLocal.first - trueHitLocalPos.X());
    if (pow(spacePointLocal.first - trueHitLocalPos.X(), 2) > maxUres * maxUres) {
      B2DEBUG(150, "The local position difference in U direction is " << spacePointLocal.first - trueHitLocalPos.X() <<
              " but maximum local position difference is set to: " << maxUres);
      return false;
    }
  }
  if (setCoordinates.second) {
    B2DEBUG(999, "Comparing the V-coordinates, SpacePoint: " << spacePointLocal.second << ", TrueHit: " << trueHitLocalPos.Y() <<
            " -> diff: " << spacePointLocal.second - trueHitLocalPos.Y());
    if (pow(spacePointLocal.second - trueHitLocalPos.Y(), 2) > maxVres * maxVres) {
      B2DEBUG(150, "The local position difference in V direction is " << spacePointLocal.second - trueHitLocalPos.Y() <<
              " but maximum local position difference is set to: " << maxVres);
      return false;
    }
  }

  // only if both local coordinates of a SpacePoint are set, compare also the global positions!
  if (setCoordinates.first && setCoordinates.second) {
    B2DEBUG(999, "Comparing the global positions, SpacePoint: (" << spacePoint->X() << "," << spacePoint->Y() << "," << spacePoint->Z()
            << "), TrueHit:  (" << trueHitGlobalPos.X() << "," << trueHitGlobalPos.Y() << "," << trueHitGlobalPos.Z() << ")");
    if (pow(spacePoint->X() - trueHitGlobalPos.X(), 2) > m_maxGlobalDiff
        || pow(spacePoint->Y() - trueHitGlobalPos.Y(), 2) > m_maxGlobalDiff ||
        pow(spacePoint->Z() - trueHitGlobalPos.Z(), 2) > m_maxGlobalDiff) {
      B2DEBUG(150, "The position differences are for X: " << spacePoint->X() - trueHitGlobalPos.X() << ", Y: " << spacePoint->Y() -
              trueHitGlobalPos.Y() << " Z: " << spacePoint->Z() - trueHitGlobalPos.Z() << " but the maximum position difference is set to: " <<
              sqrt(m_PARAMmaxGlobalDiff));
      return false;
    }
  } else {
    B2DEBUG(5, "For SpacePoint " << spacePoint->getArrayIndex() <<
            " one of the local coordinates was not assigned. The global positions and the un-assigned local coordinate were not compared!");
  }

  return true;
}

// ===================================================== GET LOCAL SPACEPOINT COORDINATES =========================================
std::pair<double, double> SpacePoint2TrueHitConnectorModule::getLocalPos(Belle2::SpacePoint* spacePoint)
{
  // get the normalized local coordinates from SpacePoint and convert them to local coordinates (have to do so because at the slanted parts the local U-position is dependant on the local V-position)
  // NOTE: second way is to convert the global position of the SpacePoint to local position via the sensorInfoBase (yields same results)
  double normU = spacePoint->getNormalizedLocalU();
  double normV = spacePoint->getNormalizedLocalV();
  return spacePoint->convertNormalizedToLocalCoordinates(std::make_pair(normU, normV), spacePoint->getVxdID());
}

// =============================================================== GET TRUEHIT POSITIONS ==========================================
template<typename TrueHitType>
std::pair<TVector3, TVector3> SpacePoint2TrueHitConnectorModule::getTrueHitPositions(TrueHitType* trueHit)
{
//   TrueHitType* trueHit = trueHits[index];
  const TVector3 localPos = TVector3(trueHit->getU(), trueHit->getV(), 0);

  const VxdID trueHitVxdId = trueHit->getSensorID();
  VXD::SensorInfoBase SensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(trueHitVxdId);
  const TVector3 globalPos = SensorInfoBase.pointToGlobal(localPos);

  return make_pair(localPos, globalPos);
}

// ================================================================= INITIALIZE ROOT FILE =========================================
void SpacePoint2TrueHitConnectorModule::initializeRootFile()
{
  if (m_PARAMrootFileName.size() != 2 || (m_PARAMrootFileName[1] != "UPDATE" && m_PARAMrootFileName[1] != "RECREATE")) {
    string output;
    for (string entry : m_PARAMrootFileName) { output += "'" + entry + "' "; }
    B2FATAL("CurlingTrackCandSplitter::initialize() : rootFileName is set wrong: entries are: " << output);
  }

  string fileName = m_PARAMrootFileName[0] + ".root";
  m_rootFilePtr = new TFile(fileName.c_str(), m_PARAMrootFileName[1].c_str());
  m_treePtr = new TTree("PosAnaTree", "Position Analysis");

  // link the variables
  m_treePtr->Branch("SPLocalU", &m_rootVariables.SpacePointULocal);
  m_treePtr->Branch("SPLocalV", &m_rootVariables.SpacePointVLocal);
  m_treePtr->Branch("SPGlobalX", &m_rootVariables.SpacePointXGlobal);
  m_treePtr->Branch("SPGlobalY", &m_rootVariables.SpacePointYGlobal);
  m_treePtr->Branch("SPGlobalZ", &m_rootVariables.SpacePointZGlobal);

  m_treePtr->Branch("THLocalU", &m_rootVariables.TrueHitULocal);
  m_treePtr->Branch("THLocalV", &m_rootVariables.TrueHitVLocal);
  m_treePtr->Branch("THGlobalX", &m_rootVariables.TrueHitXGlobal);
  m_treePtr->Branch("THGlobalY", &m_rootVariables.TrueHitYGlobal);
  m_treePtr->Branch("THGlobalZ", &m_rootVariables.TrueHitZGlobal);

  m_treePtr->Branch("WeightU", &m_rootVariables.WeightU);
  m_treePtr->Branch("WeightV", &m_rootVariables.WeightV);

  m_treePtr->Branch("relStatus", &m_rootVariables.RelationStatus);
  m_treePtr->Branch("VxdID", &m_rootVariables.HitVxdID);
  m_treePtr->Branch("nRelations", &m_rootVariables.NRelations);

  m_treePtr->Branch("clusterSizeU", &m_rootVariables.ClusterSizeU);
  m_treePtr->Branch("clusterSizeV", &m_rootVariables.ClusterSizeV);
  m_treePtr->Branch("SPErrorU", &m_rootVariables.SpacePointErrorU);
  m_treePtr->Branch("SPErrorV", &m_rootVariables.SpacePointErrorV);
  m_treePtr->Branch("SPErrorX", &m_rootVariables.SpacePointErrorX);
  m_treePtr->Branch("SPErrorY", &m_rootVariables.SpacePointErrorY);
  m_treePtr->Branch("SPErrorZ", &m_rootVariables.SpacePointErrorZ);
}

// ========================================================= CLOSE ROOT FILE ======================================================
void SpacePoint2TrueHitConnectorModule::closeRootFile()
{
  if (m_treePtr != NULL && m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
//     m_treePtr->Write(); // using TTree::Write() instead, which calls this any way
    m_rootFilePtr->Write();
    m_rootFilePtr->Close();
  }
}

// TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)
std::pair<unsigned short int, unsigned short int> SpacePoint2TrueHitConnectorModule::getClusterSizes(Belle2::SpacePoint* spacePoint,
    e_detTypes detType)
{
  std::pair<unsigned short int, unsigned short int> clusterSizes = { 0, 0 };
  if (detType == c_PXD) {
    vector<pair<PXDCluster*, double> > relClusters = getRelatedClusters<PXDCluster>(spacePoint, m_PXDClusters.getName());
    return make_pair(relClusters[0].first->getUSize(), relClusters[0].first->getVSize());
  } else {
    vector<pair<SVDCluster*, double> > relClusters = getRelatedClusters<SVDCluster>(spacePoint, m_SVDClusters.getName());
    for (auto cluster : relClusters) {
      if (cluster.first->isUCluster()) clusterSizes.first = cluster.first->getSize();
      else clusterSizes.second = cluster.first->getSize();
    }
  }
  return clusterSizes;
}

std::pair<double, double> SpacePoint2TrueHitConnectorModule::getLocalError(Belle2::SpacePoint* spacePoint)
{
  auto detType = spacePoint->getType();
  pair<double, double> errors = { -1., -1. };
  if (detType == VXD::SensorInfoBase::PXD) {
    vector<pair<PXDCluster*, double> > relClusters = getRelatedClusters<PXDCluster>(spacePoint, m_PXDClusters.getName());
    errors.first = relClusters[0].first->getUSigma();
    errors.second = relClusters[0].first->getVSigma();
  } else if (detType == VXD::SensorInfoBase::SVD) {
    vector<pair<SVDCluster*, double> > relClusters = getRelatedClusters<SVDCluster>(spacePoint, m_SVDClusters.getName());
    for (auto cluster : relClusters) {
      if (cluster.first->isUCluster()) { errors.first = cluster.first->getPositionSigma(); }
      else { errors.second = cluster.first->getPositionSigma(); }
    }
  } else {
    B2ERROR("Detector type not known in SpacePoint2TrueHitConnector::getLocalError() !");
  }

  return errors;
}
// TODO TODO TODO TODO TODO TODO TODO: remove if not needed, only for tessting at the moment (i.e. do not commit)

// ==================================================== CALCULATE RELATION WEIGHT ==================================================
double SpacePoint2TrueHitConnectorModule::calculateRelationWeight(const TrueHitInfo& trueHitInfo, Belle2::SpacePoint* spacePoint)
{
  bool isUAssigned = spacePoint->getIfClustersAssigned().first; // get if the Cluster in the SpacePoint is a U-Cluster
  // get if two Clusters are related to the Truehit (always false for single Cluster SPs
  bool bothClusters = trueHitInfo.m_U && trueHitInfo.m_V;
  // calculate the additional weight that identifies which Clusters are related to this TrueHit
  // 0 -> both Clusters have Relation to TrueHit, 10 -> only U-Cluster, 20 -> only V-Cluster has Relation to TrueHit
  short addWeight = 20 - (bothClusters ? 20 : isUAssigned ? 10 : 0);

  return addWeight + (bothClusters ? 2 : 1);
}
