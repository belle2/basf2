/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorTestModule.h>

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreatorTest)

SpacePointCreatorTestModule::SpacePointCreatorTestModule() :
  Module(),
  m_spMetaInfo("", DataStore::c_Persistent)
{
  setDescription("Tester module for the validity of the SpacePointCreatorModule.");

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
//   addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
//            "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters", bool(false));
}



void SpacePointCreatorTestModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.isRequired(m_spacePointsName);
  m_pxdClusters.isOptional(m_pxdClustersName);
  m_svdClusters.isOptional(m_svdClustersName);
  m_spMetaInfo.isRequired();


  if (m_pxdClusters.isOptional() == true) {
    // retrieve name again (faster than doing everything in the event):
    m_pxdClustersName = m_pxdClusters.getName();
  }


  if (m_svdClusters.isOptional() == true) {
    // retrieve name again (faster than doing everything in the event):
    m_svdClustersName = m_svdClusters.getName();
  }


  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = m_spacePoints.getName();


  B2INFO("SpacePointCreatorTestModule(" << m_nameOfInstance << ")::initialize: names found for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\nspacePoints: " << m_spacePointsName <<
         "\nspacePointsMetaInfo-storeObjPtr: " << m_spMetaInfo.getName() <<
         "\nspacePointsMetaInfo # of names stored: " << m_spMetaInfo->getnNames())


  // set some counters for output:
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSVDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



void SpacePointCreatorTestModule::event()
{

  B2DEBUG(2, "SpacePointCreatorTestModule(" << m_nameOfInstance << "): starting event with " << m_spacePoints.getEntries() << " spacePoints")
  for (unsigned int i = 0; i < uint(m_spacePoints.getEntries()); ++i) {
    B2DEBUG(2, " Executing SpacePoint " << i)
    const SpacePoint* sp = m_spacePoints[i];

    vector<unsigned int> indices;
    string clusterContainer = "";

    if (sp->getType() == VXD::SensorInfoBase::SensorType::SVD) {
      B2DEBUG(2, " SpacePoint " << i << " is attached to SVDCluster(s) of StoreArray " << sp->getClusterStoreName())
      for (const SVDCluster & aCluster : sp->getRelationsTo<SVDCluster>(sp->getClusterStoreName())) {
        indices.push_back(aCluster.getArrayIndex());
        clusterContainer = aCluster.getArrayName();

        B2DEBUG(2, " SpacePoint " << i <<
                " got pointer to SVDCluster with index " << aCluster.getArrayIndex() <<
                "stored in Array " << aCluster.getArrayName())
      }
    } else if (sp->getType() == VXD::SensorInfoBase::SensorType::PXD) {
      B2DEBUG(2, " SpacePoint " << i << " is attached to PXDCluster of StoreArray " << sp->getClusterStoreName())
      const PXDCluster* aCluster = sp->getRelatedTo<PXDCluster>(sp->getClusterStoreName());
      B2INFO("1")
      indices.push_back(aCluster->getArrayIndex());
      B2INFO("2")
      clusterContainer = aCluster->getArrayName();

      B2DEBUG(2, " SpacePoint " << i <<
              " got pointer to PXDCluster with index " << aCluster->getArrayIndex() <<
              "stored in Array " << aCluster->getArrayName())
    } else { B2ERROR(" SpacePoint is of unknown type " << sp->getType()) }


    B2DEBUG(1, "SpacePoint " << i <<
            " got sensorType: " << sp->getType() <<
            ", VxdID: " << VxdID(sp->getVxdID()) <<
            ", storeName for Cluster(says SpacePoint): " << sp->getClusterStoreName() <<
            ", storeName for Cluster(says Cluster): " << clusterContainer)
  }
//   storeSingleCluster(m_pxdClusters, m_spacePoints, m_pxdClustersIndex);
//
//
//   if (m_onlySingleClusterSpacePoints == true) {
//     provideSVDClusterSingles(m_svdClusters, m_spacePoints, m_svdClustersIndex); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
//   } else {
//     provideSVDClusterCombinations(m_svdClusters, m_spacePoints, m_svdClustersIndex);
//   }
//
//
//   B2DEBUG(1, "SpacePointCreatorTestModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
//           "pxdClusters: " << m_pxdClusters.getEntries() <<
//           ", svdClusters: " << m_svdClusters.getEntries() <<
//           ", spacePoints: " << m_spacePoints.getEntries())
//
//
//   if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
//     for (int index = 0; index < m_spacePoints.getEntries(); index++) {
//       const SpacePoint* sp = m_spacePoints[index];
//
//    B2DEBUG(10, "SpacePointCreatorTestModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
//               " with type " << sp->getType() <<
//               " and VxdID " << VxdID(sp->getVxdID()) <<
//               " is tied to a cluster in: " << sp->getClusterStoreName())
//     }
//   }
//
//
//   /// WARNING TODO next steps: think about mcParticle-relations and how to deal with multi-pass-setups, create container like VXDTFTrackCandidate compatible with spacePoints
//
//   m_TESTERPXDClusterCtr += m_pxdClusters.getEntries();
//   m_TESTERSVDClusterCtr += m_svdClusters.getEntries();
//   m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void SpacePointCreatorTestModule::terminate()
{
  B2INFO("SpacePointCreatorTestModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", svdClusters: " << m_TESTERSVDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}


