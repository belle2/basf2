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

using namespace Belle2;
using namespace std;

REG_MODULE(SpacePoint2TrueHitConnector) // register the modules

SpacePoint2TrueHitConnectorModule::SpacePoint2TrueHitConnectorModule() :
  Module()
{
  setDescription("Module that tries to find the appropriate TrueHit to each SpacePoint and to register a relation between them for making MC information for SpacePoints more easily accesible for Modules that need it. Module can also be used to filter out 'fishy' SpacePoints.");

  addParam("storeSeperate", m_PARAMstoreSeparate, "Set to false if you do not want to create seperate StoreArrays for processed SpacePoints. (i.e. a relation from SpacePoint to TrueHit will be set in the passed StoreArray. NOTE: this StoreArray will contain SpacePoints with a relation to TrueHits and such without after this module). The Names of the output StoreArrays will be the names of the input StoreArrays with 'outputSuffix' (module parameter) appended to them", true);

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
      m_inputPXDSpacePoints.push_back(StoreArray<SpacePoint>(m_PARAMspacePointNames.at(i)));
      m_PXDTrueHits.push_back(StoreArray<PXDTrueHit>(m_PARAMtrueHitNames.at(i)));
    } else {
      m_inputSVDSpacePoints.push_back(StoreArray<SpacePoint>(m_PARAMspacePointNames.at(i)));
      m_SVDTrueHits.push_back(StoreArray<SVDTrueHit>(m_PARAMtrueHitNames.at(i)));
    }
  }

  // get the number of PXD and SVD StoreArrays
  m_nPXDarrays = m_inputPXDSpacePoints.size();
  m_nSVDarrays = m_inputSVDSpacePoints.size();

  B2DEBUG(10, "Got " << m_nContainers << " containers in total of which " << m_nPXDarrays << " are PXD and " << m_nSVDarrays << " are SVD.");

  // check the StoreArrays
  for (unsigned int i = 0; i < m_nPXDarrays; ++i) {
    m_inputPXDSpacePoints.at(i).isRequired();
    m_PXDTrueHits.at(i).isRequired();
    if (!m_PARAMstoreSeparate) { m_inputPXDSpacePoints.at(i).registerRelationTo(m_PXDTrueHits.at(i), DataStore::c_Event, DataStore::c_DontWriteOut); }
    else {
      std::string name = m_inputPXDSpacePoints.at(i).getName() + m_PARAMoutputSuffix;
      m_outputPXDSpacePoints.push_back(StoreArray<SpacePoint>(name));
      m_outputPXDSpacePoints.at(i).registerInDataStore(name, DataStore::c_DontWriteOut);
      m_outputPXDSpacePoints.at(i).registerRelationTo(m_PXDTrueHits.at(i), DataStore::c_Event, DataStore::c_DontWriteOut);
    }
  }
  for (unsigned int i = 0; i < m_nSVDarrays; ++i) {
    m_inputSVDSpacePoints.at(i).isRequired();
    m_SVDTrueHits.at(i).isRequired();
    if (!m_PARAMstoreSeparate) { m_inputSVDSpacePoints.at(i).registerRelationTo(m_SVDTrueHits.at(i), DataStore::c_Event, DataStore::c_DontWriteOut); }
    else {
      std::string name = m_inputSVDSpacePoints.at(i).getName() + m_PARAMoutputSuffix;
      m_outputSVDSpacePoints.push_back(StoreArray<SpacePoint>(name));
      m_outputSVDSpacePoints.at(i).registerInDataStore(name, DataStore::c_DontWriteOut);
      m_outputSVDSpacePoints.at(i).registerRelationTo(m_SVDTrueHits.at(i), DataStore::c_Event, DataStore::c_DontWriteOut);
    }
  }
}

void SpacePoint2TrueHitConnectorModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCtr = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "SpacePoint2TrueHitConnector::event(). Processing event " << eventCtr << " -----------------------");

  // loop over vectors of StoreArrays seperately -> TODO: check if this can somehow be merged or templated to avoid duplicate code. For now leaving it seperated
  for (unsigned int iPXD = 0; iPXD < m_nPXDarrays; ++iPXD) { // loop over PXD StoreArrays
    // for better readability declare StoreArrays here with shorter names
    StoreArray<SpacePoint> spacePoints = m_inputPXDSpacePoints.at(iPXD);
    StoreArray<PXDTrueHit> trueHits = m_PXDTrueHits.at(iPXD);

    // get a RelationArray between Clusters and TrueHits -> Later get Cluster from SpacePoint and then get TrueHit via RelationArray
    StoreArray<PXDCluster> pxdClusters; // WARNING: no name of StoreArray here. COULDDO: specify name via module parameter!
    RelationArray clusters2TrueHits(pxdClusters, trueHits);
    B2DEBUG(12, "Found " << clusters2TrueHits.getEntries() << " relations from PXDTrueHits to PXDClusters");

    const int nSpacePoints = spacePoints.getEntries();
    const int nTrueHits = trueHits.getEntries();
    B2DEBUG(15, "In PXD Arrays " << iPXD + 1 << " of " << m_nPXDarrays << ": found " << nSpacePoints << " SpacePoints and " << nTrueHits << " TrueHits");


    m_nPXDSpacePointsCtr += nSpacePoints;
    // loop over spacePoints in StoreArray
    for (int iSP = 0; iSP < nSpacePoints; ++iSP) {
      SpacePoint* spacePoint = spacePoints[iSP];

      std::vector<unsigned int> uniqueIndices = getUniqueTrueHitIndices<PXDCluster>(spacePoint, clusters2TrueHits);
      // get the number of unique TrueHit indices, and "convert" it to an index that can be used to acces elements in the counter array. If the number is to big truncate it, and simply increase the last entry (the interpretation and output is done in terminate)
      // WARNING: hardcoded values here at the moment!
      unsigned int iSize = uniqueIndices.size() > 4 ? 4 : uniqueIndices.size() - 1;
      m_nRelPXDTrueHitsCtr.at(iSize)++;

      // NOTE: for now only relating TrueHits that have only one related TrueHit!
      if (uniqueIndices.size() == 1) {
        PXDTrueHit* trueHit = trueHits[uniqueIndices.at(0)];
        if (m_PARAMstoreSeparate) {
          SpacePoint* newSP = m_outputPXDSpacePoints.at(iPXD).appendNew(*spacePoint);
          newSP->addRelationTo(trueHit);
          B2DEBUG(50, "Added new SpacePoint to StoreArray " << m_outputPXDSpacePoints.at(iPXD).getName() << " and added relation to TrueHit " << trueHit->getArrayIndex())
        } else {
          spacePoint->addRelationTo(trueHit);
          B2DEBUG(50, "Added Relation to TrueHit " << trueHit->getArrayIndex() << " for SpacePoint " << spacePoint->getArrayIndex())
        }
        m_regPXDrelCtr++;
      }
    }
  }
  for (unsigned int iSVD = 0; iSVD < m_nSVDarrays; ++iSVD) { // loop over SVD StoreArrays
    StoreArray<SpacePoint> spacePoints = m_inputSVDSpacePoints.at(iSVD);
    StoreArray<SVDTrueHit> trueHits = m_SVDTrueHits.at(iSVD);

    // get a RelationArray between Clusters and TrueHits -> Later get Cluster from SpacePoint and then get TrueHit via RelationArray
    StoreArray<SVDCluster> svdClusters; // WARNING: no name of StoreArray here. COULDDO: specify name via module parameter!
    RelationArray clusters2TrueHits(svdClusters, trueHits);
    B2DEBUG(12, "Found " << clusters2TrueHits.getEntries() << " relations from SVDTrueHits to SVDClusters");

    const int nSpacePoints = spacePoints.getEntries();
    const int nTrueHits = trueHits.getEntries();
    B2DEBUG(15, "In SVD Arrays " << iSVD + 1 << " of " << m_nPXDarrays << ": found " << nSpacePoints << " SpacePoints and " << nTrueHits << " TrueHits");

    m_nSVDSpacePointsCtr += nSpacePoints;
    // loop over spacePoints in StoreArray
    for (int iSP = 0; iSP < nSpacePoints; ++iSP) {
      SpacePoint* spacePoint = spacePoints[iSP];

      std::vector<unsigned int> uniqueIndices = getUniqueTrueHitIndices<SVDCluster>(spacePoint, clusters2TrueHits);
      // get the number of unique TrueHit indices, and "convert" it to an index that can be used to acces elements in the counter array. If the number is to big truncate it, and simply increase the last entry (the interpretation and output is done in terminate)
      // WARNING: hardcoded values here at the moment!
      unsigned int iSize = uniqueIndices.size() > 4 ? 4 : uniqueIndices.size() - 1;
      m_nRelSVDTrueHitsCtr.at(iSize)++;

      // NOTE: for now only relating TrueHits that have only one related TrueHit!
      if (uniqueIndices.size() == 1) {
        SVDTrueHit* trueHit = trueHits[uniqueIndices.at(0)];
        if (m_PARAMstoreSeparate) {
          SpacePoint* newSP = m_outputSVDSpacePoints.at(iSVD).appendNew(*spacePoint);
          newSP->addRelationTo(trueHit);
          B2DEBUG(50, "Added new SpacePoint to StoreArray " << m_outputSVDSpacePoints.at(iSVD).getName() << " and added relation to TrueHit " << trueHit->getArrayIndex())
        } else {
          spacePoint->addRelationTo(trueHit);
          B2DEBUG(50, "Added Relation to TrueHit " << trueHit->getArrayIndex() << " for SpacePoint " << spacePoint->getArrayIndex())
        }
        m_regSVDrelCtr++;
      }
    }
  }
}

void SpacePoint2TrueHitConnectorModule::terminate()
{
  stringstream pxdSummary;
  pxdSummary << "Got " << m_nPXDarrays << " PXD containers. In those " << m_nPXDSpacePointsCtr << " SpacePoints were contained. The number of related TrueHits to a SpacePoint are:\n";
  for (unsigned int i = 0; i < m_nRelPXDTrueHitsCtr.size() - 1; ++i) { pxdSummary << i + 1 << " related TrueHits to a SpacePoint: " << m_nRelPXDTrueHitsCtr.at(i) << "\n"; }
  pxdSummary << "more than 4 related TrueHits to a SpacePoint: " << m_nRelPXDTrueHitsCtr.at(4) << "\n"; // WARNING: hardcoded at the moment!!!
  pxdSummary << "registered " << m_regPXDrelCtr << " relations from SpacePoint to TrueHit for PXD\n";

  stringstream svdSummary;
  svdSummary << "Got " << m_nSVDarrays << " SVD containers. In those " << m_nSVDSpacePointsCtr << " SpacePoints were contained. The number of related TrueHits to a SpacePoint are:\n";
  for (unsigned int i = 0; i < m_nRelSVDTrueHitsCtr.size() - 1; ++i) {svdSummary << i + 1 << " related TrueHits to a SpacePoint: " << m_nRelSVDTrueHitsCtr.at(i) << "\n"; }
  svdSummary << "more than 4 related TrueHits to a SpacePoint: " << m_nRelSVDTrueHitsCtr.at(4) << "\n"; // WARNING: hardcoded at the moment!!!
  svdSummary << "registered " << m_regSVDrelCtr << " relations from SpacePoint to TrueHit for SVD"; // now newline needed here!

  B2INFO("SpacePoint2TrueHitConnector::terminate(): Tried to relate SpacePoints in " << m_nContainers << " different containers\n" << pxdSummary.str() << svdSummary.str())
}

template<typename ClusterType>
std::vector<unsigned int> SpacePoint2TrueHitConnectorModule::getUniqueTrueHitIndices(Belle2::SpacePoint* spacePoint, RelationArray& clusters2TrueHits)
{
  std::vector<unsigned int> returnVector;

  RelationVector<ClusterType> spacePointClusters = spacePoint->getRelationsTo<ClusterType>("ALL"); // NOTE: searching all StoreArrays here -> specify via module parameter maybe?
  B2DEBUG(50, "Found " << spacePointClusters.size() << " related Clusters to SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME())) { // print some additional debug output if desired
    stringstream indStream;
    for (const ClusterType & cluster : spacePointClusters) { indStream << cluster.getArrayIndex() << ", "; }
    B2DEBUG(100, "Indices of related Clusters are: " << indStream.str())
  }

  // collect all indices of related TrueHits together with weights in a vector of pairs (convinient to sort by indices and keep asociated weights)
  std::vector<std::pair<unsigned int, float> > trueHitIndWeights;

  for (const ClusterType & cluster : spacePointClusters) {
    // collect all TrueHit indices and weights for this cluster
    int iCluster = cluster.getArrayIndex();
    const std::vector<unsigned int> trueHitInds = clusters2TrueHits[iCluster].getToIndices();
    const std::vector<float> trueHitWeights = clusters2TrueHits[iCluster].getWeights();

    B2DEBUG(60, "Found " << trueHitInds.size() << " related TrueHits to Cluster " << cluster.getArrayIndex())

    for (unsigned int iTH = 0; iTH < trueHitInds.size(); ++iTH) {
      trueHitIndWeights.push_back(std::make_pair(trueHitInds.at(iTH), trueHitWeights.at(iTH)));
      B2DEBUG(150, "Added index " << trueHitInds.at(iTH) << " with weight " << trueHitWeights.at(iTH) << " to trueHitIndWeights.");
    }
  }

  if (trueHitIndWeights.size() == 1) { // if there is only one entry there is no need to do the sort & unique stuff, simply return this value (should save some time)
    returnVector.push_back(trueHitIndWeights.at(0).first);
    B2DEBUG(60, "Found only one TrueHit (before sort & unique) to all Clusters of SpacePoint " << spacePoint->getArrayIndex() << ". TrueHit has index " << trueHitIndWeights.at(0).first << ".")
    return returnVector;
  }

  B2DEBUG(60, "Collected " << trueHitIndWeights.size() << " TrueHits related to all Clusters of SpacePoint " << spacePoint->getArrayIndex())
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME())) { // print some additional debug output if desired
    stringstream indStream;
    for (auto element : trueHitIndWeights) { indStream << element.first << ", "; }
    B2DEBUG(100, "The indices are " << indStream.str());
  }

  // sort by TrueHit indices and then unique (i.e. after this steps only unique TrueHit indices are left in the vector regardless if the weights differ)
  // COULDDO: copy
  // WARNING: if the weights differ at least one weight is lost in this step!
  std::sort(trueHitIndWeights.begin(), trueHitIndWeights.end(), [](const std::pair<unsigned int, float>& lPair, const std::pair<unsigned int, float>& rPair) { return lPair.first < rPair.first; });
  auto newEnd = std::unique(trueHitIndWeights.begin(), trueHitIndWeights.end(),
  [](const std::pair<unsigned int, float>& lPair, const std::pair<unsigned int, float>& rPair) { return lPair.first == rPair.first; });
  trueHitIndWeights.resize(std::distance(trueHitIndWeights.begin(), newEnd)); // resize vector manually as unique does not change the size of the vector

  stringstream indStream;
  for (auto element : trueHitIndWeights) {
    returnVector.push_back(element.first);
    indStream << element.first << ", ";
  }

  B2DEBUG(60, "Found " << returnVector.size() << " unique TrueHit (after sort & unique) to all Clusters of SpacePoint " << spacePoint->getArrayIndex() << ". The indices are: " << indStream.str());
  return returnVector;
}

// initialize the counters
void SpacePoint2TrueHitConnectorModule::initializeCounters()
{
  m_nContainers = 0;
  m_nPXDarrays = 0;
  m_nSVDarrays = 0;
  m_nPXDSpacePointsCtr = 0;
  m_nSVDSpacePointsCtr = 0;

  m_regSVDrelCtr = 0;
  m_regPXDrelCtr = 0;

  for (unsigned int i = 0; i < m_nRelPXDTrueHitsCtr.size(); ++i) {
    m_nRelPXDTrueHitsCtr.at(i) = 0;
    m_nRelPXDTrueHitsCtr.at(i) = 0;
  }
}