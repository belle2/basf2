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

#include <limits>       // std::numeric_limits



using namespace std;
using namespace Belle2;


REG_MODULE(TBSpacePointCreator)

// TBSpacePointCreatorModule::m_spMetaInfo("", DataStore::c_Persistent);

TBSpacePointCreatorModule::TBSpacePointCreatorModule() :
  Module(),
  m_spMetaInfo("", DataStore::c_Persistent)
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


  // there shall be only one object for metaInfo (no matter the number of instances of this module, therefore no names for it.
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


  m_telClustersIndex = std::numeric_limits<short unsigned int>::max();
  if (m_telClusters.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_spacePoints.registerRelationTo(m_telClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

    // retrieve name again (faster than doing everything in the event):
    m_telClustersName = m_telClusters.getName();

    // prepare metaInfo for the SpacePoints containing the names for the Cluster-Containers:
    m_telClustersIndex = m_spMetaInfo->addName(m_telClustersName);
  }


  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = m_spacePoints.getName();


  B2INFO("TBSpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\ntelClusters: " << m_telClustersName <<
         "\nspacePoints: " << m_spacePointsName <<
         "\nspacePointsMetaInfo-storeObjPtr: " << m_spMetaInfo.getName() <<
         "\nindex for pxdClusters (short::max means, its not set): " << m_pxdClustersIndex <<
         "\nindex for svdClusters (short::max means, its not set): " << m_svdClustersIndex <<
         "\nindex for telClusters (short::max means, its not set): " << m_telClustersIndex
        )


  // set some counters for output:
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSVDClusterCtr = 0;
  m_TESTERTelClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



void TBSpacePointCreatorModule::event()
{

  storeSingleCluster(m_telClusters, m_spacePoints, m_telClustersIndex);

  storeSingleCluster(m_pxdClusters, m_spacePoints, m_pxdClustersIndex);


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
      const TBSpacePoint* sp = m_spacePoints[index];

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


