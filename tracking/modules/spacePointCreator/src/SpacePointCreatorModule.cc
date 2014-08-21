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

SpacePointCreatorModule::SpacePointCreatorModule() :
  Module(),
  m_spMetaInfo("", DataStore::c_Persistent)
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
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut);
  m_pxdClusters.isOptional(m_pxdClustersName);
  m_svdClusters.isOptional(m_svdClustersName);


  // there shall be only one, therefore no names for it.
  m_spMetaInfo.registerInDataStore();
  if (m_spMetaInfo.isValid() == false) { m_spMetaInfo.create(); }


  m_pxdClustersIndex = std::numeric_limits<short unsigned int>::max();
  if (m_pxdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_pxdClustersName = m_pxdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    m_pxdClustersIndex = m_spMetaInfo->addName(m_pxdClustersName);
  }


  m_svdClustersIndex = std::numeric_limits<short unsigned int>::max();
  if (m_svdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_svdClustersName = m_svdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    m_svdClustersIndex = m_spMetaInfo->addName(m_svdClustersName);
  }


  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = m_spacePoints.getName();


  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\nspacePoints: " << m_spacePointsName <<
         "\nspacePointsMetaInfo-storeObjPtr: " << m_spMetaInfo.getName() <<
         "\nindex for pxdClusters (short::max means, its not set): " << m_pxdClustersIndex <<
         "\nindex for svdClusters (short::max means, its not set): " << m_svdClustersIndex)


  // set some counters for output:
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSVDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



void SpacePointCreatorModule::event()
{

  storeSingleCluster(m_pxdClusters, m_spacePoints, m_pxdClustersIndex);


  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(m_svdClusters, m_spacePoints, m_svdClustersIndex); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(m_svdClusters, m_spacePoints, m_svdClustersIndex);
  }


  B2DEBUG(1, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "pxdClusters: " << m_pxdClusters.getEntries() <<
          ", svdClusters: " << m_svdClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries())


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getClusterStoreName())
    }
  }


  /// WARNING TODO next steps: think about mcParticle-relations and how to deal with multi-pass-setups, create container like VXDTFTrackCandidate compatible with spacePoints

  m_TESTERPXDClusterCtr += m_pxdClusters.getEntries();
  m_TESTERSVDClusterCtr += m_svdClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void SpacePointCreatorModule::terminate()
{
  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", svdClusters: " << m_TESTERSVDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}


