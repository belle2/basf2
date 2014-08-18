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
  StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  StoreArray<SVDCluster> svdClusters(m_svdClustersName);
  StoreArray<SpacePoint> spacePoints(m_spacePointsName);
  StoreArray<TelCluster> telClusters(m_telClustersName);


  spacePoints.registerInDataStore(DataStore::c_DontWriteOut);
  pxdClusters.isOptional();
  svdClusters.isOptional();
  telClusters.isOptional();


  //Relations to simulation objects only if the ancestor relations exist
  //Relations to simulation objects only if the ancestor relations exist
  if (pxdClusters.isOptional() == true) { spacePoints.registerRelationTo(pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut); }
  if (svdClusters.isOptional() == true) { spacePoints.registerRelationTo(svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut); }
  if (telClusters.isOptional() == true) { spacePoints.registerRelationTo(telClusters, DataStore::c_Event, DataStore::c_DontWriteOut); }


  // retrieve names again (faster than doing everything in the event):
  m_pxdClustersName = pxdClusters.getName();
  m_svdClustersName = svdClusters.getName();
  m_telClustersName = telClusters.getName();
  m_spacePointsName = spacePoints.getName();


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
  const StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  const StoreArray<SVDCluster> svdClusters(m_svdClustersName);
  const StoreArray<TelCluster> telClusters(m_telClustersName);
  StoreArray<SpacePoint> spacePoints(m_spacePointsName);

  if (spacePoints.isValid() == false) {
    spacePoints.create();
  } else {
    spacePoints.getPtr()->Clear();
  }


  for (unsigned int i = 0; i < uint(telClusters.getEntries()); ++i) {
    spacePoints.appendNew(TBSpacePoint((telClusters[i]), i));
    spacePoints[spacePoints.getEntries() - 1]->addRelationTo(telClusters[i]);
  }


  for (unsigned int i = 0; i < uint(pxdClusters.getEntries()); ++i) {
    spacePoints.appendNew((pxdClusters[i]), i);
    spacePoints[spacePoints.getEntries() - 1]->addRelationTo(pxdClusters[i]);
  }


  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(svdClusters, spacePoints); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(svdClusters, spacePoints);
  }




  B2DEBUG(1, "TBSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "pxdClusters: " << pxdClusters.getEntries() <<
          "\nsvdClusters: " << svdClusters.getEntries() <<
          "\ntelClusters: " << telClusters.getEntries() <<
          "\nspacePoints: " << spacePoints.getEntries())

  /// WARNING TODO next steps: create relations, think about mcParticle-relations, prepare converter for GFTrackCandidates including clusters to XXTrackCandidates including SpacePoints and vice versa.

  m_TESTERPXDClusterCtr += pxdClusters.getEntries();
  m_TESTERSVDClusterCtr += svdClusters.getEntries();
  m_TESTERTelClusterCtr += telClusters.getEntries();
  m_TESTERSpacePointCtr += spacePoints.getEntries();
}



void TBSpacePointCreatorModule::terminate()
{
  B2INFO("TBSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         "\nsvdClusters: " << m_TESTERSVDClusterCtr <<
         "\ntelClusters: " << m_TESTERTelClusterCtr <<
         "\nspacePoints: " << m_TESTERSpacePointCtr)
}
