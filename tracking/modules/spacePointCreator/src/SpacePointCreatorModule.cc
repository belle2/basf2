/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorModule.h>

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreator)

SpacePointCreatorModule::SpacePointCreatorModule() : Module()
{
  setDescription("Imports Clusters of the silicon detectors and converts them to spacePoints.");

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters", bool(false));
}



void SpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  StoreArray<SVDCluster> svdClusters(m_svdClustersName);
  StoreArray<SpacePoint> spacePoints(m_spacePointsName);


  spacePoints.registerInDataStore(DataStore::c_DontWriteOut);
  pxdClusters.isOptional();
  svdClusters.isOptional();


  //prepare collecting info for SpacePoints:
  unsigned short cntActivatedClusterTypes = 0;
  vector<string> collectionNames; // will contain the names of the cluster storeArrays


  if (pxdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    spacePoints.registerRelationTo(pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_pxdClustersName = pxdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    collectionNames.push_back(m_pxdClustersName);
    m_pxdClustersIndex = cntActivatedClusterTypes;
    cntActivatedClusterTypes++;

  }


  if (svdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    spacePoints.registerRelationTo(svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_svdClustersName = svdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    collectionNames.push_back(m_svdClustersName);
    m_svdClustersIndex = cntActivatedClusterTypes;
    cntActivatedClusterTypes++;

  }


  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = spacePoints.getName();


  // store the collected names in the MetaInfo of the SpacePoints:
  SpacePoint initMetaData = SpacePoint(collectionNames);


  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\nspacePoints: " << m_spacePointsName)


  // set some counters for output:
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSVDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



void SpacePointCreatorModule::event()
{
  const StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  const StoreArray<SVDCluster> svdClusters(m_svdClustersName);

  StoreArray<SpacePoint> spacePoints(m_spacePointsName);
  spacePoints.clear();

  for (unsigned int i = 0; i < uint(pxdClusters.getEntries()); ++i) {
    SpacePoint* newSP = spacePoints.appendNew((pxdClusters[i]), i, m_pxdClustersIndex);
    newSP->addRelationTo(pxdClusters[i]);
  }


  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(svdClusters, spacePoints, m_svdClustersIndex); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(svdClusters, spacePoints, m_svdClustersIndex);
  }




  B2DEBUG(1, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "pxdClusters: " << pxdClusters.getEntries() <<
          ", svdClusters: " << svdClusters.getEntries() <<
          ", spacePoints: " << spacePoints.getEntries())


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < spacePoints.getEntries(); index++) {
      const SpacePoint* sp = spacePoints[index];

      B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getClusterStoreName())
    }
  }


  /// WARNING TODO next steps: think about mcParticle-relations and how to deal with multi-pass-setups, create container like VXDTFTrackCandidate compatible with spacePoints

  m_TESTERPXDClusterCtr += pxdClusters.getEntries();
  m_TESTERSVDClusterCtr += svdClusters.getEntries();
  m_TESTERSpacePointCtr += spacePoints.getEntries();
}



void SpacePointCreatorModule::terminate()
{
  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", svdClusters: " << m_TESTERSVDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}


