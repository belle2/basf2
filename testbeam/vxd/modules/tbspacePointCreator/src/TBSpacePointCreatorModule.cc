/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/tbspacePointCreator/TBSpacePointCreatorModule.h>

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(TBSpacePointCreator)

TBSpacePointCreatorModule::TBSpacePointCreatorModule() : Module()
{
  setDescription("Imports Clusters of the silicon detectors and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("TelClusters", m_telClustersName,
           "TelCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));


  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters", bool(false));
}



void TBSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut);
  m_pxdClusters.isOptional(m_pxdClustersName);
  m_svdClusters.isOptional(m_svdClustersName);
  m_telClusters.isOptional(m_telClustersName);


  //prepare collecting info for SpacePoints:
  unsigned short cntActivatedClusterTypes = 0;
  vector<string> collectionNames; // will contain the names of the cluster storeArrays


  if (m_pxdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_pxdClustersName = m_pxdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    collectionNames.push_back(m_pxdClustersName);
    m_pxdClustersIndex = cntActivatedClusterTypes;
    cntActivatedClusterTypes++;

  }


  if (m_svdClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_svdClustersName = m_svdClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    collectionNames.push_back(m_svdClustersName);
    m_svdClustersIndex = cntActivatedClusterTypes;
    cntActivatedClusterTypes++;

  }


  if (m_telClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_telClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_telClustersName = m_telClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    collectionNames.push_back(m_telClustersName);
    m_telClustersIndex = cntActivatedClusterTypes;
    cntActivatedClusterTypes++;

  }


  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = m_spacePoints.getName();


  // store the collected names in the MetaInfo of the SpacePoints:
  SpacePoint initMetaData = SpacePoint(collectionNames);


  B2INFO("TBSpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\ntelClusters: " << m_telClustersName <<
         "\nspacePoints: " << m_spacePointsName)


  // set some counters for output:
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSVDClusterCtr = 0;
  m_TESTERTelClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



void TBSpacePointCreatorModule::event()
{

  for (unsigned int i = 0; i < uint(m_telClusters.getEntries()); ++i) {
    SpacePoint* newSP = m_spacePoints.appendNew(TBSpacePoint((m_telClusters[i]), i, m_telClustersIndex));
    newSP->addRelationTo(m_telClusters[i]);
  }


  for (unsigned int i = 0; i < uint(m_pxdClusters.getEntries()); ++i) {
    SpacePoint* newSP = m_spacePoints.appendNew((m_pxdClusters[i]), i, m_pxdClustersIndex);
    newSP->addRelationTo(m_pxdClusters[i]);
  }


  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(m_svdClusters, m_spacePoints, m_svdClustersIndex); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(m_svdClusters, m_spacePoints, m_svdClustersIndex);
  }




  B2DEBUG(1, "TBSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "pxdClusters: " << m_pxdClusters.getEntries() <<
          "\nsvdClusters: " << m_svdClusters.getEntries() <<
          "\ntelClusters: " << m_telClusters.getEntries() <<
          "\nspacePoints: " << m_spacePoints.getEntries())


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
  m_TESTERTelClusterCtr += m_telClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void TBSpacePointCreatorModule::terminate()
{
  B2INFO("TBSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         "\nsvdClusters: " << m_TESTERSVDClusterCtr <<
         "\ntelClusters: " << m_TESTERTelClusterCtr <<
         "\nspacePoints: " << m_TESTERSpacePointCtr)
}


