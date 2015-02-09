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

#include <string>
#include <vector>

#include <algorithm>
// #include <map>
#include <unordered_map>
#include <boost/concept_check.hpp>

#include <TFile.h>
#include <TTree.h>

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
  addParam("TrueHitNames", m_PARAMtrueHitNames, "Container names of TrueHits.", defaultInList);
  addParam("SpacePointNames", m_PARAMspacePointNames, "Container names of SpacePoints.", defaultInList);
  addParam("DetectorTypes", m_PARAMdetectorTypes, "detector types to determine which entries in 'TrueHitNames' and 'SpacePointNames' belong to which detector type. Entries have to be 'SVD' or 'PXD'!");

  addParam("outputSuffix", m_PARAMoutputSuffix, "Suffix that will be appended to the container names if 'storeSeperate' is set to true", std::string("_relTH"));

  // initialize all couters
  initializeCounters();
}



void SpacePoint2TrueHitConnectorModule::initialize()
{
  B2INFO("SpacePoint2TrueHitConnector -------------------------- initialize --------------------------------");

  // check the input and get some information from it
  for (std::string entry : m_PARAMdetectorTypes) {
    if (entry.compare(std::string("SVD")) != 0  && entry.compare(std::string("PXD")) != 0) {
      B2FATAL("Found entry " << entry << " in DetectorTypes, but detectorTypes has to be either 'PXD' or 'SVD'!")
    }
    if (entry.compare(std::string("SVD")) == 0) { m_detectorTypes.push_back(c_SVD); } // to avoid having to do the string comparison every time store the detector type in a vector
    else { m_detectorTypes.push_back(c_PXD); }
  }
  if (m_PARAMdetectorTypes.size() != m_PARAMspacePointNames.size()) {
    B2FATAL("The number of entries in 'DetectorTypes' and 'SpacePointNames' do not match!") // for the moment this is fatal -> COULDDO: solve this without stopping the whole process
  }
  if (m_PARAMdetectorTypes.size() != m_PARAMtrueHitNames.size()) {
    B2FATAL("The number of entries in 'DetectorTypes' and 'TrueHitNames' do not match!") // for the moment this is fatal -> COULDDO: solve this without stopping the whole process
  }

  m_nContainers = m_PARAMdetectorTypes.size(); // get the number of total entries
  if (m_PARAMoutputSuffix.empty()) {
    B2WARNING("'outputSuffix' is empty and 'storeSeperate' is set to true. This would lead to StoreArrays with the same name. Resetting to 'outputSuffix' to '_relTH'!");
    m_PARAMoutputSuffix = "_relTH";
  }

  // put the StoreArrays in the according vectors
  for (unsigned int i = 0; i < m_nContainers; ++i) {
    if (m_detectorTypes.at(i) == c_PXD) {
      m_inputSpacePoints.push_back(std::make_pair(StoreArray<SpacePoint>(m_PARAMspacePointNames.at(i)), c_PXD)); // NEW
      m_PXDTrueHits.push_back(StoreArray<PXDTrueHit>(m_PARAMtrueHitNames.at(i))); // NEEDED
    } else {
      m_inputSpacePoints.push_back(std::make_pair(StoreArray<SpacePoint>(m_PARAMspacePointNames.at(i)), c_SVD)); // NEW
      m_SVDTrueHits.push_back(StoreArray<SVDTrueHit>(m_PARAMtrueHitNames.at(i))); // NEEDED
    }
  }

  // NEW: check the StoreArrays (hopefully a little bit more general)
  int iPXD = 0, iSVD = 0; // need seperate counters for the vectors of StoreArrays of TrueHits (cannot put them together into one StoreArray!)
  for (unsigned int i = 0; i < m_nContainers; ++i) {
    m_inputSpacePoints.at(i).first.isRequired();
    e_detTypes detType = m_inputSpacePoints.at(i).second; // get the detector type

    if (detType == c_PXD) { // checking the TrueHit StoreArrays is detector dependent
      m_PXDTrueHits.at(iPXD).isRequired();
      iPXD++;
    } else {
      m_SVDTrueHits.at(iSVD).isRequired();
      iSVD++;
    }

    if (m_PARAMstoreSeparate) { // if desired create new StoreArrays and relations
      std::string name = m_inputSpacePoints.at(i).first.getName() + m_PARAMoutputSuffix;
      m_outputSpacePoints.push_back(StoreArray<SpacePoint>(name));
      m_outputSpacePoints.at(i).registerInDataStore(name, DataStore::c_DontWriteOut);

      // relation registering detector dependent again! (counters - 1, because they are already increased above)
      if (detType == c_PXD) { m_outputSpacePoints.at(i).registerRelationTo(m_PXDTrueHits.at(iPXD - 1), DataStore::c_Event, DataStore::c_DontWriteOut); }
      else { m_outputSpacePoints.at(i).registerRelationTo(m_SVDTrueHits.at(iSVD - 1), DataStore::c_Event, DataStore::c_DontWriteOut); }
    } else { // register relations for existing StoreArrays
      if (detType == c_PXD) { m_inputSpacePoints.at(i).first.registerRelationTo(m_PXDTrueHits.at(iPXD - 1), DataStore::c_Event, DataStore::c_DontWriteOut); }
      else { m_inputSpacePoints.at(i).first.registerRelationTo(m_SVDTrueHits.at(iSVD - 1), DataStore::c_Event, DataStore::c_DontWriteOut); }
    }
  }

  // get the number of PXD and SVDStoreArrays (only needed for summary output in terminate)
  m_nPXDarrays = m_PXDTrueHits.size();
  m_nSVDarrays = m_SVDTrueHits.size();
}

void SpacePoint2TrueHitConnectorModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCtr = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "SpacePoint2TrueHitConnector::event(). Processing event " << eventCtr << " -----------------------");

  // NEW WAY (hopefully more general than old way)
  int iPXD = -1, iSVD = -1; // need again seperate counters for accessing the PXD and SVD TrueHits vectors correctly. initialized to -1, because they are increased by one before accessing related stuff for the first time!
  for (unsigned int iCont = 0; iCont < m_nContainers; ++iCont) {
    StoreArray<SpacePoint> spacePoints = m_inputSpacePoints.at(iCont).first;
    const int nSpacePoints = spacePoints.getEntries();

    B2DEBUG(10, "Found " << nSpacePoints << " SpacePoints in Array " << m_inputSpacePoints.at(iCont).first.getName() << " for this event.")

    e_detTypes detType = m_inputSpacePoints.at(iCont).second;
    if (detType == c_PXD) {
      iPXD++; // increase for later use
      B2DEBUG(12, "This are PXD arrays")
      m_nPXDSpacePointsCtr += nSpacePoints;
    } else {
      iSVD++; // increase for later use
      B2DEBUG(12, "This are SVD arrays")
      m_nSVDSpacePointsCtr += nSpacePoints;
    }

    // have to get StoreArrays of TrueHits for both cases (regardless on the actual detType) here, to have them available throughout the event method (elsewise the code would have to be splitted into a PXD and a SVD section with a lot of duplicate code!)
    // accessing only the appropriate StoreArray below!!
    StoreArray<PXDTrueHit> pTrueHits("fooName"); // searching for a StoreArray with an arbitrary string as name such that the returned StoreArray is most probably empty
    if (iPXD >= 0) { pTrueHits = m_PXDTrueHits.at(iPXD); }
    StoreArray<SVDTrueHit> sTrueHits("fooName"); // searching for a StoreArray with an arbitrary string as name such that the returned StoreArray is most probably empty
    if (iSVD >= 0) { sTrueHits = m_SVDTrueHits.at(iSVD); }

    // loop over all SpacePoints in container
    for (int iSP = 0; iSP < nSpacePoints; ++iSP) {

      SpacePoint* spacePoint = spacePoints[iSP];

      baseMapT trueHitMap; // (multi)map of TrueHit indices to weights
      try {
        if (detType == c_PXD) { trueHitMap = getRelatedTrueHits<baseMapT, PXDCluster, PXDTrueHit>(spacePoint); } // at the moment searching in ALL containers of Clusters and TrueHits!
        // can only be SVD here! (everything else causes a B2FATAL in initialize)
        else { trueHitMap = getRelatedTrueHits<baseMapT, SVDCluster, SVDTrueHit>(spacePoint); }// at the moment searching in ALL containers of Clusters and TrueHits!
      } catch (NoClusterToSpacePoint& anE) {
        B2WARNING("Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what()); // leave B2WARNING here since this really is an exception that should not occur
        m_noClusterCtr++;
        continue; // continue with next SpacePoint
      } catch (NoTrueHitToCluster& anE) {
        B2WARNING("Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what()); // leave B2WARNING here since this really is an exception that should not occur
        m_noTrueHitCtr++;
        continue; // continue with next SpacePoint
      }

//       std::vector<int> uniqueTHinds = getUniqueKeys(trueHitMap);
      unsigned int nUniqueTHs = getUniqueSize(trueHitMap); // COULDDO: get this number from previously defined vector instead of a function call
      B2DEBUG(50, "Found " << nUniqueTHs << " TrueHits (unique) related to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName())
      // convert the number of related TrueHits to an index to increase the associated counter array entry. If the number is too big, truncate it, and increase the last entry (interpretation and output in terminate)
      unsigned int iSize = nUniqueTHs > 4 ? 4 : nUniqueTHs - 1;
      if (detType == c_PXD) { m_nRelPXDTrueHitsCtr.at(iSize)++; }
      else { m_nRelSVDTrueHitsCtr.at(iSize)++; }

      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 250, PACKAGENAME())) { // print the complete map if the debug level is set high enough
        std::string mapCont = printMap(trueHitMap);
        B2DEBUG(250, "The TrueHits and their weights for spacePoint " << spacePoint->getArrayIndex() << ": " + mapCont);
      }


      // if registerAll is set to true, simply register a relation for every found TrueHit. the weight of the relation is the sum of the weights of the relations between Cluster and TrueHit!
      if (m_PARAMregisterAll) {
        SpacePoint* newSP = spacePoint; // for less writing effort assign pointer to new variable
        if (m_PARAMstoreSeparate) {
          newSP = m_outputSpacePoints.at(iCont).appendNew(*spacePoint);  // get other pointer if needed
          B2DEBUG(50, "Added new SpacePoint to StoreArray " << m_outputSpacePoints.at(iCont).getName() << ".")
        }

        std::vector<int> uniqueTHinds = getUniqueKeys(trueHitMap); // get the indices of all related TrueHits and loop over them
        for (int aInd : uniqueTHinds) {
          std::vector<float> weights = getValuesToKey(trueHitMap, aInd);
          float sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0); // sum all weights to the index -> sum of weights is used as weight for the relation between TrueHit and SpacePoint
          VXDTrueHit* trueHit;
          if (detType == c_PXD) { trueHit = pTrueHits[aInd]; }
          else { trueHit = sTrueHits[aInd]; }
          newSP->addRelationTo(trueHit, sumOfWeights); // add relation to SpacePoint
          m_regRelationsCtr++;
          B2DEBUG(50, "Added Relation to TrueHit " << trueHit->getArrayIndex() << " (weight = " << sumOfWeights << ") for SpacePoint " << newSP->getArrayIndex() << " in Array " << newSP->getArrayName())
        }
      } else { // do not register relations blindly, but try to make a relation to only one TrueHit
        SpacePoint* newSP = spacePoint;

        std::pair<VXDTrueHit*, double> trueHitwWeight;
        try { // getTHwithWeight throws exceptions when the relating does not work
          if (detType == c_PXD) {
            trueHitwWeight = getTHwithWeight<baseMapT, PXDTrueHit>(trueHitMap, pTrueHits, spacePoint); // get the TrueHit (according to the conditions in header file)
          } else {
            trueHitwWeight = getTHwithWeight<baseMapT, SVDTrueHit>(trueHitMap, sTrueHits, spacePoint); // get the TrueHit (according to the conditions in header file)
          }
          if (trueHitwWeight.first == NULL) {
            B2ERROR("NULL pointer")
            throw NoClusterToSpacePoint(); // TODO: make exception for this case
          }
        } catch (SpacePointIsGhostHit& anE) {
          B2WARNING("Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what()); // B2WARNING for now -> change to B2DEBUG(1,...) when done
          m_ghostHitCtr++;
          continue; // continue with next SpacePoint
        } catch (...) {
          B2WARNING("Caught an exception. But code section is currently under design, so not possible to decide which exception")
          continue; // continue with next SpacePoint if an exception is caught
        }

        if (m_PARAMstoreSeparate) {
          newSP = m_outputSpacePoints.at(iCont).appendNew(*spacePoint);
          B2DEBUG(50, "Added new SpacePoint to StoreArray " << m_outputSpacePoints.at(iCont).getName() << ".");
        }
        newSP->addRelationTo(trueHitwWeight.first, trueHitwWeight.second);
        m_regRelationsCtr++;
        B2DEBUG(50, "Added Relation to TrueHit " << trueHitwWeight.first->getArrayIndex() << " (weight = " << trueHitwWeight.second << ") for SpacePoint " << newSP->getArrayIndex() << " in Array " << newSP->getArrayName())
      }
    }
  }
}

// ================================================================ TERMINATE =======================================================================================
void SpacePoint2TrueHitConnectorModule::terminate()
{
  stringstream pxdSummary;
  pxdSummary << "Got " << m_nPXDarrays << " PXD containers. In those " << m_nPXDSpacePointsCtr << " SpacePoints were contained. The number of related TrueHits to a SpacePoint are:\n";
  for (unsigned int i = 0; i < m_nRelPXDTrueHitsCtr.size() - 1; ++i) { pxdSummary << i + 1 << " related TrueHits to a SpacePoint: " << m_nRelPXDTrueHitsCtr.at(i) << "\n"; }
  pxdSummary << "more than 4 related TrueHits to a SpacePoint: " << m_nRelPXDTrueHitsCtr.at(4) << "\n"; // WARNING: hardcoded at the moment!!!

  stringstream svdSummary;
  svdSummary << "Got " << m_nSVDarrays << " SVD containers. In those " << m_nSVDSpacePointsCtr << " SpacePoints were contained. The number of related TrueHits to a SpacePoint are:\n";
  for (unsigned int i = 0; i < m_nRelSVDTrueHitsCtr.size() - 1; ++i) {svdSummary << i + 1 << " related TrueHits to a SpacePoint: " << m_nRelSVDTrueHitsCtr.at(i) << "\n"; }
  svdSummary << "more than 4 related TrueHits to a SpacePoint: " << m_nRelSVDTrueHitsCtr.at(4); // WARNING: hardcoded at the moment!!!

  stringstream generalSummary;
  generalSummary << "Registered " << m_regRelationsCtr << " new relations between SpacePoints and TrueHits. " << m_ghostHitCtr << " SpacePoints were probably ghost hits\n";

  B2INFO("SpacePoint2TrueHitConnector::terminate(): Tried to relate SpacePoints in " << m_nContainers << " different containers\n" << generalSummary.str() << pxdSummary.str() << svdSummary.str())
  B2INFO("total number of weights: " << m_totWeightsCtr << " of which " << m_negWeightCtr << " were negative")
  B2INFO("Number of SpacePoints that contained a Cluster to which no related TrueHit could be found " << m_noTrueHitCtr << ". Number of SpacePoints that contained no Cluster " << m_noClusterCtr);
  B2INFO("m_moreThan2Weights = " << m_moreThan2Weights);

  // Write one varible to root
  TFile* aFile = new TFile("simple.root", "recreate");
  TTree* aTree = new TTree("tree", "aTree");
  aTree->Branch("weightThing", &m_weightDiffsByAvg);
  aTree->Fill();
  aFile->cd();
  aTree->Write();
  aFile->Close();
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
      B2DEBUG(1, "Found no related TrueHit for Cluster " << cluster.getArrayIndex() << " contained by SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
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
  m_nContainers = 0;
  m_nPXDarrays = 0;
  m_nSVDarrays = 0;
  m_nPXDSpacePointsCtr = 0;
  m_nSVDSpacePointsCtr = 0;

  m_regRelationsCtr = 0;

  for (unsigned int i = 0; i < m_nRelPXDTrueHitsCtr.size(); ++i) {
    m_nRelPXDTrueHitsCtr.at(i) = 0;
    m_nRelPXDTrueHitsCtr.at(i) = 0;
  }

  m_negWeightCtr = 0;
  m_totWeightsCtr = 0;
  m_noTrueHitCtr = 0;

  m_noClusterCtr = 0;

  m_ghostHitCtr = 0;

  m_moreThan2Weights = 0;
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


  // 1) check if there is only one TrueHit
  if (indNweights.size() == 1) { // if there is only one TrueHit, sum up the weights in the map, and return
    B2DEBUG(999, "There is only one TrueHit for spacePoint " << spacePoint->getArrayIndex() << ". It has " << indNweights.at(0).second << " associated weights")
    std::vector<float> weights = getValuesToKey(aMap, indNweights.at(0).first);
    double sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0);

    TrueHitType* trueHit = trueHits[indNweights.at(0).first];
    if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, sumOfWeights); } // only return TrueHit if it is compatible
  }

  // 2) check if there are TrueHits with 2 weights
  std::vector<int> indsTwoWeights; // vector to collect all the indices, with two associated weights!
  for (auto aPair : indNweights) { if (aPair.second == 2) indsTwoWeights.push_back(aPair.first); }

  // count the TrueHits that have more than one weight (should be two)
  int nTwoWeights = indsTwoWeights.size();
  B2DEBUG(999, "number of entries in map with two weights: " << nTwoWeights)

  if (nTwoWeights == 1) { // if there is only one TrueHit with two weights, sum up the weights and return it
    int key = indsTwoWeights.at(0);
    B2DEBUG(999, "key of only entry in map with two weights: " << key << " (for SpacePoint " << spacePoint->getArrayIndex() << ")")
    std::vector<float> weights = getValuesToKey(aMap, key);
    double sumOfWeights = std::accumulate(weights.begin(), weights.end(), 0.0);

    TrueHitType* trueHit = trueHits[key];
    if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, sumOfWeights); } // only return TrueHit if it is compatible
  }

  // collect the sum of the weights of the TrueHits with 2 weights
  std::vector<std::pair<int, double> > weightSums;
  for (int anInd : indsTwoWeights) {
    std::vector<float> weights = getValuesToKey(aMap, anInd);
    weightSums.push_back(std::make_pair(anInd, std::accumulate(weights.begin(), weights.end(), 0.0)));
  }

  if (!weightSums.empty()) {
    std::sort(weightSums.begin(), weightSums.end(), [](const std::pair<int, double>& lP, const std::pair<int, double>& rP) {return lP.second > rP.second; }); // lambda function such that the highest value is first after sort!

    B2DEBUG(999, "Index of the TrueHit with the highest sum of weights: " << weightSums.at(0).first << ", sum of weights for TrueHit: " << weightSums.at(0).second)

    TrueHitType* trueHit = trueHits[weightSums.at(0).first];
    if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, weightSums.at(0).second); } // only return TrueHit if it is compatible
  } else { B2DEBUG(999, "There is no TrueHit with two weights for SpacePoint " << spacePoint->getArrayIndex()) } // very verbose debug output

  // 3) get the TrueHit with the biggest weight
  weightSums.clear();
  for (auto aPair : indNweights) {
    std::vector<float> weights = getValuesToKey(aMap, aPair.first);
    weightSums.push_back(std::make_pair(aPair.first, weights.at(0)));   // WARNING: only assuming here that there will be no more than one weight!!!
  }

  if (!weightSums.empty()) {
    std::sort(weightSums.begin(), weightSums.end(), [](const std::pair<int, double>& lP, const std::pair<int, double>& rP) {return lP.second > rP.second; }); // lambda function such that the highest value is first after sort!
    B2DEBUG(999, "Index of the TrueHit with the highest weight: " << weightSums.at(0).first << ", weight for TrueHit: " << weightSums.at(0).second)

    TrueHitType* trueHit = trueHits[indNweights.at(0).first];
    if (compatibleCombination(spacePoint, trueHit)) { return std::make_pair(trueHit, weightSums.at(0).second); } // only return TrueHit if it is compatible
  }

  return THwithWeight;
}

// ================================================================ CHECK IF SPACEPOINT AND TRUEHIT ARE COMPATIBLE ===============================================================================
template <typename TrueHitType>
bool SpacePoint2TrueHitConnectorModule::compatibleCombination(Belle2::SpacePoint* spacePoint, TrueHitType* trueHit)
{
  B2DEBUG(250, "Checking TrueHit " << trueHit->getArrayIndex() << " from Array " << trueHit->getArrayName() << " and SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " for compatibility")
  // TODO! for the moment let pass everything
  return true;
}
