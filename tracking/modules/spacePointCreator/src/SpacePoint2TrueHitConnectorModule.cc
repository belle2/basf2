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
  addParam("TrueHitNames", m_PARAMtrueHitNames, "Container names of TrueHits.", defaultInList);
  addParam("SpacePointNames", m_PARAMspacePointNames, "Container names of SpacePoints.", defaultInList);
  addParam("DetectorTypes", m_PARAMdetectorTypes, "detector types to determine which entries in 'TrueHitNames' and 'SpacePointNames' belong to which detector type. Entries have to be 'SVD' or 'PXD'!");

  addParam("outputSuffix", m_PARAMoutputSuffix, "Suffix that will be appended to the container names if 'storeSeperate' is set to true", std::string("_relTH"));

  addParam("maxGlobalPosDiff", m_PARAMmaxGlobalDiff, "max difference of global position coordinates between TrueHit and SpacePoint (in each direction) in cm. NOTE: the default value is still subject to tuning and finding the appropriate value!", 0.05);
//   addParam("maxLocalPosDiff", m_PARAMmaxLocalDiff, "max difference of local position coordinates between TrueHit and SpacePoint (in U & V direction) in cm. NOTE: the default value is still subject to tuning and finding the appropriate value!", 0.01);

  addParam("maxPosSigma", m_PARAMmaxPosSigma, "Define the maximum local position difference in units of PitchSize / sqrt(12)", 4.);
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

  // square the max position difference (later also squared values are used)
  m_PARAMmaxGlobalDiff *= m_PARAMmaxGlobalDiff;
//   m_PARAMmaxLocalDiff *= m_PARAMmaxLocalDiff;
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
      B2DEBUG(49, "Processing SpacePoint " << spacePoint->getArrayIndex())

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
          if (trueHitwWeight.first == NULL) { throw SpacePointIsGhostHit(); } // if there is no TrueHit it is MOST PROBABLY a ghost hit
        } catch (SpacePointIsGhostHit& anE) {
          B2DEBUG(1, "Caught an exception while trying to relate SpacePoints and TrueHits: " << anE.what()); // B2WARNING for now -> change to B2DEBUG(1,...) when done
          m_ghostHitCtr++;
          continue; // continue with next SpacePoint
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
  svdSummary << "more than 4 related TrueHits to a SpacePoint: " << m_nRelSVDTrueHitsCtr.at(4) << "\n"; // WARNING: hardcoded at the moment!!!

  stringstream furtherSummary;
  furtherSummary << "possible/accepted relations for cases:\n";
  furtherSummary << m_all2WTHCtr << "/" << m_accAll2WTHCtr << " SP with THs (more than one) with all THs having two weights\n";
  furtherSummary << m_single2WTHCtr << "/" << m_accSingle2WTHCtr << " SP with THs (more than one) with only one TH having two weights\n";
  furtherSummary << m_nonSingle2WTHCtr << "/" << m_accNonSingle2WTHCtr << " SP with THs (more than one) with more than one but not all THs having two weights\n";
  furtherSummary << "In " << m_oneCluster2THCtr << " cases there was a SP with only one Cluster but more than one related TrueHits";

  stringstream generalSummary;
  generalSummary << "Registered " << m_regRelationsCtr << " new relations between SpacePoints and TrueHits. " << m_ghostHitCtr << " SpacePoints were probably ghost hits\n";

  B2INFO("SpacePoint2TrueHitConnector::terminate(): Tried to relate SpacePoints in " << m_nContainers << " different containers\n" << generalSummary.str() << pxdSummary.str() << svdSummary.str() << furtherSummary.str())
//   B2INFO("total number of weights: " << m_totWeightsCtr << " of which " << m_negWeightCtr << " were negative")
//   B2INFO("Number of SpacePoints that contained a Cluster to which no related TrueHit could be found " << m_noTrueHitCtr << ". Number of SpacePoints that contained no Cluster " << m_noClusterCtr);
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

  m_single2WTHCtr = 0;
  m_nonSingle2WTHCtr = 0;
  m_all2WTHCtr = 0;
  m_accSingle2WTHCtr = 0;
  m_accNonSingle2WTHCtr = 0;
  m_accAll2WTHCtr = 0;

  m_oneCluster2THCtr = 0;
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
    std::vector<float> weights = getValuesToKey(aMap, indNweights.at(0).first);
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
    std::vector<float> weights = getValuesToKey(aMap, key);
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
      std::vector<float> weights = getValuesToKey(aMap, anInd);
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
      std::vector<float> weights = getValuesToKey(aMap, aPair.first);
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
    if (pow(spacePoint->X() - trueHitGlobalPos.X(), 2) > m_PARAMmaxGlobalDiff || pow(spacePoint->Y() - trueHitGlobalPos.Y(), 2) > m_PARAMmaxGlobalDiff ||
        pow(spacePoint->Z() - trueHitGlobalPos.Z(), 2) > m_PARAMmaxGlobalDiff) {
      B2DEBUG(150, "The position differences are for X: " << spacePoint->X() - trueHitGlobalPos.X() << ", Y: " << spacePoint->Y() - trueHitGlobalPos.Y() << " Z: " << spacePoint->Z() - trueHitGlobalPos.Z() << " but the maximum position difference is set to: " << sqrt(m_PARAMmaxGlobalDiff))
      return false;
    }
  } else {
    B2DEBUG(1, "For SpacePoint " << spacePoint->getArrayIndex() << " one of the local coordinates was not assigned. The global positions and the un-assigned local coordinate were not compared!")
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