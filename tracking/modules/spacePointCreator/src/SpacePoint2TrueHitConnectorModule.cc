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

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>

#include <algorithm>
// #include <map>
#include <unordered_map>
#include <boost/concept_check.hpp>

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

  addParam("storeSeperate", m_PARAMstoreSeparate, "Set to false if you do not want to create seperate StoreArrays for processed SpacePoints. (i.e. a relation from SpacePoint to TrueHit will be set in the passed StoreArray. NOTE: this StoreArray will contain SpacePoints with a relation to TrueHits and such without after this module). The Names of the output StoreArrays will be the names of the input StoreArrays with 'outputSuffix' (module parameter) appended to them", true);

  addParam("registerAll", m_PARAMregisterAll, "If set to true, the module simply registers a relation for all TrueHits that are related to a SpacePoint (resp. its Clusters). In this way the module can be used to find all related TrueHits and then the user can decide what to do with these TrueHits (otherwise this module does some decision making). NOTE that some of the information is lost in this way (e.g. how many of the Clusters of a SpacePoint have been related to a TrueHit)!", false);

  std::vector<std::string> defaultInList; // default list for input StoreArrays
  defaultInList.push_back(std::string(""));
  addParam("TrueHitNames", m_PARAMtrueHitNames, "Container names of TrueHits. NOTE: need one name per 'DetectorType' (i.e. unique entries in 'DetectorType)!", defaultInList);
  addParam("SpacePointNames", m_PARAMspacePointNames, "Container names of SpacePoints.", defaultInList);
  addParam("DetectorTypes", m_PARAMdetectorTypes, "detector types to determine which entries in 'TrueHitNames' and 'SpacePointNames' belong to which detector type. Entries have to be 'SVD' or 'PXD'. NOTE: if more 'SpacePointNames' than 'DetectorTypes' get passed, the last entry in 'DetectorTypes' is assumed to be valid for all remaining 'SpacePointNames'!");
  addParam("ClusterNames", m_PARAMclusterNames, "Container names of Clusters. NOTE: need one name per ('DetectorType' (i.e. unique entries in 'DetectorType')!", defaultInList);

  addParam("outputSuffix", m_PARAMoutputSuffix, "Suffix that will be appended to the container names if 'storeSeperate' is set to true", std::string("_relTH"));

  addParam("maxGlobalPosDiff", m_PARAMmaxGlobalDiff, "max difference of global position coordinates between TrueHit and SpacePoint (in each direction) in cm. NOTE: the default value is still subject to tuning and finding the appropriate value!", 0.05);
//   addParam("maxLocalPosDiff", m_PARAMmaxLocalDiff, "max difference of local position coordinates between TrueHit and SpacePoint (in U & V direction) in cm. NOTE: the default value is still subject to tuning and finding the appropriate value!", 0.01);

  addParam("maxPosSigma", m_PARAMmaxPosSigma, "Define the maximum local position difference in units of PitchSize / sqrt(12)", 4.);
  // initialize all couters
  initializeCounters();
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
    B2FATAL("Passed " << nTHNames << " TrueHitNames and " << nDetTypes << " DetectorTypes but number of passed SpacePointArrays is " << m_nContainers)
  }
  if ((nTHNames != nDetTypes) || (nClNames != nTHNames)) {
    B2FATAL("Passed " << nTHNames << " TrueHitNames and " << nClNames << "ClusterNames but " << nDetTypes << " DetectorTypes!")
  }

  for (unsigned int i = 0; i < m_nContainers; ++i) {
    if (i < nDetTypes) { // handle the detector types
      std::string detType = m_PARAMdetectorTypes.at(i);
      if (detType.compare(std::string("SVD")) != 0 && detType.compare(std::string("PXD")) != 0) {
        B2FATAL("Found entry " << detType << " in DetectorTypes, but only 'PXD' and 'SVD' are allowed!")
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
  }

  if (m_PARAMstoreSeparate) {
    if (m_PARAMoutputSuffix.empty()) {
      B2WARNING("'outputSuffix' is empty and 'storeSeperate' is set to true. This would lead to StoreArrays with the same name. Resetting to 'outputSuffix' to '_relTH'!");
      m_PARAMoutputSuffix = "_relTH";
    }

    for (unsigned int i = 0; i < m_nContainers; ++i) {
      std::string name = m_inputSpacePoints.at(i).first.getName() + m_PARAMoutputSuffix;
      m_outputSpacePoints.push_back(StoreArray<SpacePoint>(name));
      m_outputSpacePoints.at(i).registerInDataStore(name, DataStore::c_DontWriteOut);

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
}

// ========================================================== EVENT ===============================================================
void SpacePoint2TrueHitConnectorModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCtr = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "SpacePoint2TrueHitConnector::event(). Processing event " << eventCtr << " -----------------------");


  // NEW LOOP! (leaving old untouched)
  for (m_iCont = 0; m_iCont < m_nContainers; ++m_iCont) {
    StoreArray<SpacePoint> spacePoints = m_inputSpacePoints.at(m_iCont).first;
    const int nSpacePoints = spacePoints.getEntries();
    e_detTypes detType = m_inputSpacePoints.at(m_iCont).second;
    std::string detTypeStr = detType == c_SVD ? "SVD" : "PXD";
    B2DEBUG(10, "Found " << nSpacePoints << " SpacePoints in Array " << m_inputSpacePoints.at(m_iCont).first.getName() << " for this event. detType: " << detTypeStr)

    m_SpacePointsCtr.at(m_iCont) += nSpacePoints;

    for (int iSP = 0; iSP < nSpacePoints; ++iSP) {
      SpacePoint* spacePoint = spacePoints[iSP];
      B2DEBUG(49, "Processing SpacePoint " << iSP << " from " << nSpacePoints)

      baseMapT trueHitMap = processSpacePoint<baseMapT>(spacePoint, detType);
      if (trueHitMap.empty()) continue; // next SpacePoint if something went wrong

      unsigned int nUniqueTHs = getUniqueSize(trueHitMap);
      B2DEBUG(50, "Found " << nUniqueTHs << " TrueHits (unique) related to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName())
      unsigned int iRels = nUniqueTHs > 4 ? 4 : nUniqueTHs - 1;
      m_nRelTrueHitsCtr.at(m_iCont).at(iRels)++;

      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 350, PACKAGENAME())) { // print the complete map if the debug level is set high enough
        std::string mapCont = printMap(trueHitMap);
        B2DEBUG(250, "The TrueHits and their weights for spacePoint " << spacePoint->getArrayIndex() << ": " + mapCont);
      }

      if (m_PARAMregisterAll) {
        registerAllRelations(spacePoint, trueHitMap, detType);
      } else { // find THE ONE TrueHit (to rule them all, one TrueHit to find them all ...)
        // COULDDO: wrap this up in a function
        if (detType == c_PXD) {
          pair<PXDTrueHit*, double> trueHitwWeight = getTHwithWeight<baseMapT, PXDTrueHit>(trueHitMap, m_PXDTrueHits, spacePoint);
          if (trueHitwWeight.first != NULL) {
            registerOneRelation(spacePoint, trueHitwWeight, detType);
            continue;
          }
        } else {
          pair<SVDTrueHit*, double> trueHitwWeight = getTHwithWeight<baseMapT, SVDTrueHit>(trueHitMap, m_SVDTrueHits, spacePoint);
          if (trueHitwWeight.first != NULL) {
            registerOneRelation<SVDTrueHit>(spacePoint, trueHitwWeight, detType);
            continue;
          }
        }
        // if NULL above -> probably ghost hit
        B2DEBUG(10, "Could not relate one TrueHit to SpacePoint " << spacePoint->getArrayIndex() << ". This SpacePoint is probably a ghosthit!")
        m_ghostHitCtr.at(m_iCont)++;
      }
    } // end loop SpacePoints
  } // end loop containers
}

// ================================================================ TERMINATE =======================================================================================
void SpacePoint2TrueHitConnectorModule::terminate()
{
  unsigned int sumSpacePoints = accumulate(m_SpacePointsCtr.begin(), m_SpacePointsCtr.end(), 0);
  unsigned int sumRelations = accumulate(m_regRelationsCtr.begin(), m_regRelationsCtr.end(), 0);

  B2RESULT("SpacePoint2TrueHitConnector: Got " << sumSpacePoints << " SpacePoints in " << m_nContainers << " containers and registered " << sumRelations << " relations to TrueHits")
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME())) {
    stringstream contSumm;
    contSumm << "Container-wise summary: \n";

    for (unsigned int iCont = 0; iCont < m_nContainers; ++iCont) {
      contSumm << "In Container " << iCont << " (container name: " << m_inputSpacePoints[iCont].first.getName() << ") " << m_SpacePointsCtr[iCont]  << " SpacePoints were contained. The number of related TrueHits to a SpacePoint are:\n";
      for (unsigned int i = 0; i < m_nRelTrueHitsCtr[iCont].size() - 1; ++i) { contSumm << i + 1 << " related TrueHits to a SpacePoint : " << m_nRelTrueHitsCtr[iCont].at(i) << "\n"; }
      contSumm << " more than 4 related TrueHits to a SpacePoint: " << m_nRelTrueHitsCtr[iCont].at(4) << "\n"; // WARNING hardcoded at the moment!!!
      contSumm << m_ghostHitCtr.at(iCont) << " were probably ghost hits in this container!\n";
    }
    B2DEBUG(1, contSumm.str())

    if (!m_PARAMregisterAll) {
      // TODO: do this containerwise
      stringstream furtherSummary;
      furtherSummary << "Summary for all containers:\n";
      furtherSummary << "possible/accepted relations for cases:\n";
      furtherSummary << m_all2WTHCtr << "/" << m_accAll2WTHCtr << " SP with THs (more than one) with all THs having two weights\n";
      furtherSummary << m_single2WTHCtr << "/" << m_accSingle2WTHCtr << " SP with THs (more than one) with only one TH having two weights\n";
      furtherSummary << m_nonSingle2WTHCtr << "/" << m_accNonSingle2WTHCtr << " SP with THs (more than one) with more than one but not all THs having two weights\n";
      furtherSummary << "In " << m_oneCluster2THCtr << " cases there was a SP with only one Cluster but more than one related TrueHits";
      B2DEBUG(2, furtherSummary.str())
    }
  }

//   B2INFO("total number of weights: " << m_totWeightsCtr << " of which " << m_negWeightCtr << " were negative")
//   B2INFO("Number of SpacePoints that contained a Cluster to which no related TrueHit could be found " << m_OLDnoTrueHitCtr << ". Number of SpacePoints that contained no Cluster " << m_OLDnoClusterCtr);
//   B2INFO("m_moreThan2Weights = " << m_moreThan2Weights);

  // Write one varible to root NOT NEEDED AT THE MOMENT
//   TFile* aFile = new TFile("simple.root", "recreate");
//   TTree* aTree = new TTree("tree", "aTree");
//   aTree->Branch("weightThing", &m_weightDiffsByAvg);
//   aTree->Fill();
//   aFile->cd();
//   aTree->Write();
//   aFile->Close();
}

// ====================================================== PROCESS SPACEPOINT ======================================================
template<typename MapType>
MapType SpacePoint2TrueHitConnectorModule::processSpacePoint(Belle2::SpacePoint* spacePoint, e_detTypes detType)
{
  B2DEBUG(50, "Processing SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName())
  MapType trueHitMap;
  try {
    if (detType == c_PXD) {
      trueHitMap = getRelatedTrueHits<MapType, PXDCluster, PXDTrueHit>(spacePoint, m_PXDClusters.getName(), m_PXDTrueHits.getName());
    } else {
      trueHitMap = getRelatedTrueHits<MapType, SVDCluster, SVDTrueHit>(spacePoint, m_SVDClusters.getName(), m_SVDTrueHits.getName());
    }
  } catch (std::runtime_error& anE) { // should catch all Belle2 exceptions in try-block
    B2WARNING("Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what());
    increaseExceptionCounter(anE);
  } catch (...) { // catch the rest
    B2ERROR("Caught undefined exception while trying to relate SpacePoints and TrueHtis")
    throw; // throw further (maybe it is caught somewhere else)
  }

  B2DEBUG(499, "trueHitMap.size() before return in processSpacePoint: " << trueHitMap.size())
  return trueHitMap;
}

/////////////////////////////////////////////// GET RELATED TRUE HITS /////////////////////////////////////////////////////////////////////////////
template<typename MapType, typename ClusterType, typename TrueHitType>
MapType SpacePoint2TrueHitConnectorModule::getRelatedTrueHits(Belle2::SpacePoint* spacePoint, std::string clusterName, std::string trueHitName)
{
  MapType trueHitsMap; // map to be filled with indices (keys) and weights (values)

  RelationVector<ClusterType> spacePointClusters = spacePoint->getRelationsTo<ClusterType>(clusterName);
  if (spacePointClusters.size() == 0) {
    B2DEBUG(1, "Found no related Cluster for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayIndex())
    throw NoClusterToSpacePoint();
  }
  B2DEBUG(75, "Found " << spacePointClusters.size() << " related Clusters to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName())

  // loop over all Clusters, get all TrueHits from them and add the information to the map
  for (const ClusterType & cluster : spacePointClusters) {
    RelationVector<TrueHitType> clusterTrueHits = cluster.template getRelationsTo<TrueHitType>(trueHitName); // Note searching all StoreArrys here -> wanted? TODO: check
    if (clusterTrueHits.size() == 0) {
      B2DEBUG(3, "Found no related TrueHit for Cluster " << cluster.getArrayIndex() << " contained by SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
      throw NoTrueHitToCluster();
    }
    B2DEBUG(80, "Found " << clusterTrueHits.size() << " related TrueHits to Cluster " << cluster.getArrayIndex() << " from Array " << cluster.getArrayName())

    for (unsigned int i = 0; i < clusterTrueHits.size(); ++i) { // 'TrueHit' loop
      trueHitsMap.insert(std::pair<int, float>(clusterTrueHits[i]->getArrayIndex(), clusterTrueHits.weight(i)));
    }
  }

  return trueHitsMap;
}

// ================================================================= INITIALIZE COUNTERS ==============================================================================================
void SpacePoint2TrueHitConnectorModule::initializeCounters()
{
  // NEW
  for (unsigned int i = 0; i < m_SpacePointsCtr.size(); ++i) {
    m_SpacePointsCtr.at(i) = 0;
    m_regRelationsCtr.at(i) = 0;
    m_noClusterCtr.at(i) = 0;
    m_ghostHitCtr.at(i) = 0;
    m_noTrueHitCtr.at(i) = 0;

    for (unsigned int j = 0; j < m_nRelTrueHitsCtr.at(i).size(); ++j) { m_nRelTrueHitsCtr.at(i).at(j) = 0; }
  }

  m_negWeightCtr = 0;
  m_totWeightsCtr = 0;
  m_moreThan2Weights = 0;

  m_single2WTHCtr = 0;
  m_nonSingle2WTHCtr = 0;
  m_all2WTHCtr = 0;
  m_accSingle2WTHCtr = 0;
  m_accNonSingle2WTHCtr = 0;
  m_accAll2WTHCtr = 0;

  m_oneCluster2THCtr = 0;
}

// ================================================ INCREASE EXCEPTION COUNTER ====================================================
void SpacePoint2TrueHitConnectorModule::increaseExceptionCounter(std::runtime_error& exception)
{
  const char* message = exception.what();
  // NOTE comparing the messages of the exceptions to determine which exception got caught
  if (strcmp(message, NoClusterToSpacePoint().what()) == 0) {
    m_noClusterCtr.at(m_iCont)++;
  } else if (strcmp(message, NoTrueHitToCluster().what()) == 0) {
    m_noTrueHitCtr.at(m_iCont)++;
  } else {
    B2ERROR("Caught an unexpected runtime_error: " << exception.what()) // only the upper to should be caught!
  }
}

// ===================================================== REGISTER ALL RELATIONS ===================================================
template<typename MapType>
void SpacePoint2TrueHitConnectorModule::registerAllRelations(Belle2::SpacePoint* spacePoint, MapType trueHitMap, e_detTypes detType)
{
  B2DEBUG(50, "Registering all possible relations for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". storeSeparate is set to " << m_PARAMstoreSeparate)
  SpacePoint* newSP = spacePoint; // declaring pointer here, getting new pointer if storeSeparate ist true

  if (m_PARAMstoreSeparate) { // if storing in separate Array, re-register the relations to the Clusters first
    newSP = m_outputSpacePoints.at(m_iCont).appendNew(*spacePoint);
    B2DEBUG(50, "Added new SpacePoint to Array " << m_outputSpacePoints[m_iCont].getName() << ".")
    if (detType == c_PXD) reRegisterClusterRelations<PXDCluster>(spacePoint, newSP, m_PXDClusters.getName());
    else reRegisterClusterRelations<SVDCluster>(spacePoint, newSP, m_SVDClusters.getName());
  }

  // register the Relations to the TrueHits
  std::vector<int> uniqueTHinds = getUniqueKeys(trueHitMap); // get the unique indices of all related TrueHits and loop over them
  for (int aInd : uniqueTHinds) {
    std::vector<double> weights = getValuesToKey(trueHitMap, aInd);
    double sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0); // sum all weights to the index -> sum of weights is used as weight for the relation between TrueHit and SpacePoint
    if (detType == c_PXD) registerTrueHitRelation<PXDTrueHit>(newSP, aInd, sumOfWeights, m_PXDTrueHits);
    else registerTrueHitRelation<SVDTrueHit>(newSP, aInd, sumOfWeights, m_SVDTrueHits);
  }
}

// =============================================== REGISTER ONE RELATION ==========================================================
template<typename TrueHitType>
void SpacePoint2TrueHitConnectorModule::registerOneRelation(Belle2::SpacePoint* spacePoint, std::pair<TrueHitType*, double> trueHitwWeight, e_detTypes detType)
{
  TrueHitType* trueHit = trueHitwWeight.first;
  B2DEBUG(50, "Registering relation between to TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName())
  SpacePoint* newSP = spacePoint; // declaring pointer here, getting new pointer if storeSeparate ist true

  if (m_PARAMstoreSeparate) { // if storing in separate Array, re-register the relations to the Clusters first
    newSP = m_outputSpacePoints.at(m_iCont).appendNew(*spacePoint);
    B2DEBUG(50, "Added new SpacePoint to Array " << m_outputSpacePoints[m_iCont].getName() << ".")
    if (detType == c_PXD) reRegisterClusterRelations<PXDCluster>(spacePoint, newSP, m_PXDClusters.getName());
    else reRegisterClusterRelations<SVDCluster>(spacePoint, newSP, m_SVDClusters.getName());
  }

  newSP->addRelationTo(trueHit, trueHitwWeight.second);
  m_regRelationsCtr.at(m_iCont)++;
  B2DEBUG(50, "Added Relation to TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName() << " for SpacePoint " << spacePoint->getArrayIndex() << " (weight = " << trueHitwWeight.second << ")")
}

// ========================================================= REREGISTER CLUSTER RELATIONS =========================================
template<typename ClusterType>
void SpacePoint2TrueHitConnectorModule::reRegisterClusterRelations(Belle2::SpacePoint* origSpacePoint, Belle2::SpacePoint* newSpacePoint, std::string clusterName)
{
  B2DEBUG(100, "Registering the Relations to Clusters of SpacePoint " << origSpacePoint->getArrayIndex() << " in Array " << origSpacePoint->getArrayName() << " for SpacePoint " << newSpacePoint->getArrayIndex() << " in Array " << newSpacePoint->getArrayIndex())

  vector<pair<ClusterType*, double> > clustersAndWeights = getRelatedClusters<ClusterType>(origSpacePoint, clusterName);
  for (auto aCluster : clustersAndWeights) {
    newSpacePoint->addRelationTo(aCluster.first, aCluster.second);
    B2DEBUG(100, "Registered Relation to Cluster " << aCluster.first->getArrayIndex() << " with weight " << aCluster.second)
  }
}

// =========================================================== GET RELATED CLUSTERS ===============================================
template<typename ClusterType>
std::vector<std::pair<ClusterType*, double> > SpacePoint2TrueHitConnectorModule::getRelatedClusters(Belle2::SpacePoint* spacePoint, std::string clusterName)
{
  vector<pair<ClusterType*, double> > indsAndWeights;
  RelationVector<ClusterType> relClusters = spacePoint->getRelationsTo<ClusterType>(clusterName);

  for (unsigned int iCl = 0; iCl < relClusters.size(); ++iCl) {
    indsAndWeights.push_back(make_pair(relClusters[iCl], relClusters.weight(iCl)));
  }

  if (indsAndWeights.empty()) { B2ERROR("No Clusters related to SpacePoint " << spacePoint->getArrayIndex() << "!"); } // safety measure, should not / cannot happen (checked before)

  return indsAndWeights;
}

// ====================================================== REGISTER TRUEHIT RELATIONS ==============================================
template<typename TrueHitType>
void SpacePoint2TrueHitConnectorModule::registerTrueHitRelation(Belle2::SpacePoint* spacePoint, int index, double weight, Belle2::StoreArray<TrueHitType> trueHits)
{
  TrueHitType* trueHit = trueHits[index];
  spacePoint->addRelationTo(trueHit, weight);
  m_regRelationsCtr.at(m_iCont)++; // increase counter of registered relations for this container
  B2DEBUG(50, "Added Relation to TrueHit " << index << " from Array " << trueHits.getName() << " for SpacePoint " << spacePoint->getArrayIndex() << " (weight = " << weight << ")")
}

// ======================================================================== PRINT MAP ====================================================================================================
// COULDDO: rewrite with getUniqueKeys(aMap) and getValuesToKey(aMap, key)
template <typename MapType>
std::string SpacePoint2TrueHitConnectorModule::printMap(const MapType& aMap)
{
  if (aMap.empty()) return std::string("passed map is empty!");

  typedef typename MapType::const_iterator mapIter; // typedef for less typing effort

  stringstream mapContent;
  mapContent << "printMap(): content of map:\n";
  typename MapType::key_type lastKey = aMap.begin()->first; // last key that has not been processed (is the first key in the map before processing)
  for (mapIter it = aMap.begin(); it != aMap.end(); ++it) { // loop over all entries

    if (it->first != lastKey) { // if the key has changed get all entries to the
      std::pair<mapIter, mapIter > keyRange = aMap.equal_range(lastKey);

      mapContent << "key: " << lastKey << " => value(s): ";
      for (mapIter kit = keyRange.first; kit != keyRange.second; ++kit) { mapContent << " " << kit->second; }
      mapContent << "\n";

      lastKey = it->first; // assign key from this loop as last non-processed key
    }
  }
  // now process the last non-processed key (COULDDO: check if this can somehow be put inside the loop)
  std::pair<mapIter, mapIter> keyRange = aMap.equal_range(lastKey);
  mapContent << "key: " << lastKey << " => value(s): ";
  for (mapIter kit = keyRange.first; kit != keyRange.second; ++kit) { mapContent << " " << kit->second; }
  mapContent << "\n";

  return mapContent.str();
}

// ============================================================= GET UNIQUE KEYS ======================================================================================================
template <typename MapType>
std::vector<typename MapType::key_type> SpacePoint2TrueHitConnectorModule::getUniqueKeys(const MapType& aMap)
{
  std::vector<typename MapType::key_type> allKeys; // collect all keys of the map -> then sort & unique (+resize)
  if (aMap.empty()) { return allKeys; }

  typedef typename MapType::const_iterator mapIter;
  for (mapIter it = aMap.begin(); it != aMap.end(); ++it) { allKeys.push_back(it->first); }
  std::sort(allKeys.begin(), allKeys.end());
  auto newEnd = std::unique(allKeys.begin(), allKeys.end());
  allKeys.resize(std::distance(allKeys.begin(), newEnd));

  return allKeys;
}

// =============================================================== GET VALUES PER KEY ===================================================================================================
template <typename MapType>
std::vector<std::pair<typename MapType::key_type, unsigned int> > SpacePoint2TrueHitConnectorModule::getNValuesPerKey(const MapType& aMap)
{
  typedef typename MapType::key_type keyT;
  std::vector<std::pair<keyT, unsigned int> > valuesPerKey;
  if (aMap.empty()) return valuesPerKey; // return empty vector if map is empty
  typedef typename MapType::const_iterator mapIter;

  std::vector<keyT> uniqueKeys = getUniqueKeys<MapType>(aMap);

  for (keyT key : uniqueKeys) {
    std::pair<mapIter, mapIter> keyRange = aMap.equal_range(key);
    valuesPerKey.push_back(std::make_pair(key, std::distance(keyRange.first, keyRange.second)));
  }
  return valuesPerKey;
}

// =================================================================== GET VALUES TO KEY ==================================================================================================
template <typename MapType>
std::vector<typename MapType::mapped_type> SpacePoint2TrueHitConnectorModule::getValuesToKey(const MapType& aMap, typename MapType::key_type aKey)
{
  typedef typename MapType::const_iterator mapIter;

  std::vector<typename MapType::mapped_type> values;
  if (aMap.empty()) return values;

  std::pair<mapIter, mapIter> keyRange = aMap.equal_range(aKey);
  for (mapIter it = keyRange.first; it != keyRange.second; ++it) { values.push_back(it->second); }

  return values;
}

// ========================================================================= GET TH WITH WEIGHT ===========================================================================================
template<typename MapType, typename TrueHitType>
std::pair<TrueHitType*, double> SpacePoint2TrueHitConnectorModule::getTHwithWeight(const MapType& aMap, Belle2::StoreArray<TrueHitType> trueHits, Belle2::SpacePoint* spacePoint)
{
  std::pair<TrueHitType*, double> THwithWeight(NULL, 0.0); // default return value
  std::vector<std::pair<int, unsigned int> > indNweights = getNValuesPerKey(aMap);

  // very verbose output only to have a look on why these TrueHits could be in the same SpacePoint
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 999, PACKAGENAME())) {
    std::pair<double, double> spacePointLocal = SpacePoint::convertNormalizedToLocalCoordinates(std::make_pair(spacePoint->getNormalizedLocalU(), spacePoint->getNormalizedLocalV()), spacePoint->getVxdID());
    std::pair<bool, bool> assignedLocal = spacePoint->getIfClustersAssigned();

    B2DEBUG(999, "SpacePoint " << spacePoint->getArrayIndex() << " U: " << spacePointLocal.first << " V: " << spacePointLocal.second << " assigned: " << assignedLocal.first << ", " << assignedLocal.second)

    std::vector<int> uniqueKeys = getUniqueKeys(aMap);
    for (int key : uniqueKeys) {
      TrueHitType* trueHit = trueHits[key];
      MCParticle* mcParticle = trueHit->template getRelatedFrom<MCParticle>("ALL"); // NOTE: assuming here that there is only one MCParticle to each TrueHit

      int mcPartId = mcParticle->getArrayIndex();
      bool primary = mcParticle->hasStatus(MCParticle::c_PrimaryParticle);
      int pdgCode = mcParticle->getPDG();

      B2DEBUG(999, "TrueHit " << key << " U: " << trueHit->getU() << ", V: " << trueHit->getV() << " mc Particle Id: " << mcPartId << ", primary " << primary << ", pdg: " << pdgCode);
    }
  }

  // 1) check if there is only one TrueHit
  if (indNweights.size() == 1) { // if there is only one TrueHit, sum up the weights in the map, and return
    B2DEBUG(999, "There is only one TrueHit for spacePoint " << spacePoint->getArrayIndex() << ". It has " << indNweights.at(0).second << " associated weights")
    std::vector<typename MapType::mapped_type> weights = getValuesToKey(aMap, indNweights.at(0).first);
    double sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0);

    TrueHitType* trueHit = trueHits[indNweights.at(0).first];
    if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, sumOfWeights); } // only return TrueHit if it is compatible
  }

  // 2) check if there are TrueHits with 2 weights
  std::vector<int> indsTwoWeights; // vector to collect all the indices, with two associated weights!
  for (auto aPair : indNweights) { if (aPair.second == 2) indsTwoWeights.push_back(aPair.first); }
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 999, PACKAGENAME())) {
    stringstream output;
    for (int anInd : indsTwoWeights) { output << anInd << " "; }
    B2DEBUG(999, "Indices with two associated weights: " << output.str())
  }

  // count the TrueHits that have more than one weight (should be two)
  int nTwoWeights = indsTwoWeights.size();
  B2DEBUG(999, "number of entries in map with two weights: " << nTwoWeights)

  if (nTwoWeights == 1) { // if there is only one TrueHit with two weights, sum up the weights and return it
    m_single2WTHCtr++; // at this point has to be another TrueHit without 2 weights -> increase counter
    int key = indsTwoWeights.at(0);
    B2DEBUG(999, "key of only entry in map with two weights: " << key << " (for SpacePoint " << spacePoint->getArrayIndex() << ")")
    std::vector<typename MapType::mapped_type> weights = getValuesToKey(aMap, key);
    double sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0);

    TrueHitType* trueHit = trueHits[key];
    if (compatibleCombination(spacePoint, trueHit)) { // only return TrueHit if it is compatible
      m_accSingle2WTHCtr++; // if accepted increase counter
      return std::make_pair(trueHit, sumOfWeights);
    }
  }

  if (nTwoWeights > 1) { // only do the following steps if there are more than one candidates
    bool allTwoWeights = uint(nTwoWeights) == indNweights.size(); // to increase the right counter variable. If the size of all keys and the size of keys with two weights is the same -> all TrueHits have two weights
    if (allTwoWeights) m_all2WTHCtr++;
    else m_nonSingle2WTHCtr++;
    // collect the sum of the weights of the TrueHits with 2 weights
    std::vector<std::pair<int, double> > weightSums;
    for (int anInd : indsTwoWeights) {
      std::vector<typename MapType::mapped_type> weights = getValuesToKey(aMap, anInd);
      B2DEBUG(999, "Getting the sum of weights for TrueHit " << anInd << " that has two attatched weights.")
      weightSums.push_back(std::make_pair(anInd, std::accumulate(weights.begin(), weights.end(), 0.0)));
    }

    if (!weightSums.empty()) {
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1999, PACKAGENAME())) { // only for developing purposes
        stringstream output;
        output << "content of weightSums before sorting:\n";
        for (auto aPair : weightSums) { output << "index " << aPair.first << " sum of weights: " << aPair.second << "\n"; }
        B2DEBUG(999, output.str())
      }

      std::sort(weightSums.begin(), weightSums.end(), [](const std::pair<int, double>& lP, const std::pair<int, double>& rP) {return lP.second > rP.second; }); // lambda function such that the highest value is first after sort!

      for (unsigned int iTH = 0; iTH < weightSums.size(); ++iTH) { // loop over all TrueHits with two weights (starting from the one with the biggest weight sum, the first TrueHit that is compatible (position) gets returned
        TrueHitType* trueHit = trueHits[weightSums.at(iTH).first];
        if (compatibleCombination(spacePoint, trueHit)) { // only return TrueHit if it is compatible
          if (allTwoWeights) m_accAll2WTHCtr++;
          else m_accNonSingle2WTHCtr++; // increase the appropriate counter variables

          return std::make_pair(trueHit, weightSums.at(iTH).second);
        } else { B2DEBUG(999, "TrueHit " << weightSums.at(iTH).first << " is not compatible!") }
      }
    } else { B2DEBUG(999, "There is no TrueHit with two weights for SpacePoint " << spacePoint->getArrayIndex()) } // very verbose debug output
  }

  // 3) get the TrueHit with the biggest weight only if the SpacePoint has only one related Cluster
  unsigned int nClusters = 0;
  if (spacePoint->getType() == VXD::SensorInfoBase::PXD) { nClusters = spacePoint->getRelationsTo<PXDCluster>("ALL").size(); } // NOTE: searching in all
  else { nClusters = spacePoint->getRelationsTo<SVDCluster>("ALL").size(); }

  B2DEBUG(999, "Number of related Clusters for SpacePoint " << nClusters)
  if (nClusters == 1) {
    m_oneCluster2THCtr++; // at this point only possible way
    std::vector<std::pair<int, double> > weightSums;
    for (auto aPair : indNweights) {
      std::vector<typename MapType::mapped_type> weights = getValuesToKey(aMap, aPair.first);
      weightSums.push_back(std::make_pair(aPair.first, weights.at(0)));   // WARNING: only assuming here that there will be no more than one weight!!!
    }

    if (!weightSums.empty()) {
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1999, PACKAGENAME())) { // only for developing purposes
        stringstream output;
        output << "content of weightSums before sorting:\n";
        for (auto aPair : weightSums) { output << "index " << aPair.first << " sum of weights: " << aPair.second << "\n"; }
        B2DEBUG(999, output.str())
      }

      std::sort(weightSums.begin(), weightSums.end(), [](const std::pair<int, double>& lP, const std::pair<int, double>& rP) {return lP.second > rP.second; }); // lambda function such that the highest value is first after sort!
      B2DEBUG(999, "Index of the TrueHit with the highest weight: " << weightSums.at(0).first << ", weight for TrueHit: " << weightSums.at(0).second)

      TrueHitType* trueHit = trueHits[weightSums.at(0).first];
      if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, weightSums.at(0).second); } // only return TrueHit if it is compatible
    }
  }
  return THwithWeight;
}

// ================================================================ CHECK IF SPACEPOINT AND TRUEHIT ARE COMPATIBLE ===============================================================================
template <typename TrueHitType>
bool SpacePoint2TrueHitConnectorModule::compatibleCombination(Belle2::SpacePoint* spacePoint, TrueHitType* trueHit)
{
  B2DEBUG(150, "Checking TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName() << " and SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " for compatibility")

  const VxdID spacePointVxdId = spacePoint->getVxdID();
  const VxdID trueHitVxdId = trueHit->getSensorID();

  B2DEBUG(999, "Comparing the VxdIDs, SpacePoint: " << spacePointVxdId << ", TrueHit: " << trueHitVxdId)
  if (spacePointVxdId != trueHitVxdId) {
    B2DEBUG(150, "SpacePoint and TrueHit do not have the same VxdID. spacePoint: " << spacePointVxdId << ", trueHit: " << trueHitVxdId);
    return false;
  }

  VXD::SensorInfoBase SensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(trueHitVxdId); // only have to get one, since VxdIds are already the same at this point!
  double maxUres = SensorInfoBase.getUPitch(trueHit->getV()) / sqrt(12.) * m_PARAMmaxPosSigma;
  double maxVres = SensorInfoBase.getVPitch(trueHit->getV()) / sqrt(12.) * m_PARAMmaxPosSigma;

  B2DEBUG(999, "maximum residual in U: " << maxUres << ", in V: " << maxVres);

  const TVector3 trueHitLocalPos = TVector3(trueHit->getU(), trueHit->getV(), 0);
  const TVector3 trueHitGlobalPos = SensorInfoBase.pointToGlobal(trueHitLocalPos);

  std::pair<double, double> spacePointLocal = getLocalPos(spacePoint);
  // compare only those values of the local coordinates that have been set
  std::pair<bool, bool> setCoordinates = spacePoint->getIfClustersAssigned();

  if (setCoordinates.first) {
    B2DEBUG(999, "Comparing the U-coordinates, SpacePoint: " << spacePointLocal.first << ", TrueHit: " << trueHitLocalPos.X())
    if (pow(spacePointLocal.first - trueHitLocalPos.X(), 2) > maxUres * maxUres) {
      B2DEBUG(150, "The local position difference in U direction is " << spacePointLocal.first - trueHitLocalPos.X() << " but maximum local position difference is set to: " << maxUres)
      return false;
    }
  }
  if (setCoordinates.second) {
    B2DEBUG(999, "Comparing the V-coordinates, SpacePoint: " << spacePointLocal.second << ", TrueHit: " << trueHitLocalPos.Y())
    if (pow(spacePointLocal.second - trueHitLocalPos.Y(), 2) > maxVres * maxVres) {
      B2DEBUG(150, "The local position difference in V direction is " << spacePointLocal.second - trueHitLocalPos.Y() << " but maximum local position difference is set to: " << maxVres)
      return false;
    }
  }

  // only if both local coordinates of a SpacePoint are set, compare also the global positions!
  if (setCoordinates.first && setCoordinates.second) {
    B2DEBUG(999, "Comparing the global positions, SpacePoint: (" << spacePoint->X() << "," << spacePoint->Y() << "," << spacePoint->Z() << "), TrueHit:  (" << trueHitGlobalPos.X() << "," << trueHitGlobalPos.Y() << "," << trueHitGlobalPos.Z() << ")")
    if (pow(spacePoint->X() - trueHitGlobalPos.X(), 2) > m_maxGlobalDiff || pow(spacePoint->Y() - trueHitGlobalPos.Y(), 2) > m_maxGlobalDiff ||
        pow(spacePoint->Z() - trueHitGlobalPos.Z(), 2) > m_maxGlobalDiff) {
      B2DEBUG(150, "The position differences are for X: " << spacePoint->X() - trueHitGlobalPos.X() << ", Y: " << spacePoint->Y() - trueHitGlobalPos.Y() << " Z: " << spacePoint->Z() - trueHitGlobalPos.Z() << " but the maximum position difference is set to: " << sqrt(m_PARAMmaxGlobalDiff))
      return false;
    }
  } else {
    B2DEBUG(5, "For SpacePoint " << spacePoint->getArrayIndex() << " one of the local coordinates was not assigned. The global positions and the un-assigned local coordinate were not compared!")
  }

  return true;
}

// ============================================================================ GET LOCAL SPACEPOINT COORDINATES ============================================================================
std::pair<double, double> SpacePoint2TrueHitConnectorModule::getLocalPos(Belle2::SpacePoint* spacePoint)
{
  // get the normalized local coordinates from SpacePoint and convert them to local coordinates (have to do so because at the slanted parts the local U-position is dependant on the local V-position)
  double normU = spacePoint->getNormalizedLocalU();
  double normV = spacePoint->getNormalizedLocalV();
  return SpacePoint::convertNormalizedToLocalCoordinates(std::make_pair(normU, normV), spacePoint->getVxdID());
}
