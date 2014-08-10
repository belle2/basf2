/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorModule.h>

#include <framework/datastore/StoreArray.h>
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
  addParam("RelSpacePointsPXDClusters", m_relSpacePointsPXDClustersName,
           "SpacePoints <-> PXDClusters relation name", string(""));
  addParam("RelSpacePointsSVDClusters", m_relSpacePointsSVDClustersName,
           "SpacePoints <-> SVDClusters relation name", string(""));
}

void SpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  StoreArray<SVDCluster> svdClusters(m_svdClustersName);
  StoreArray<SpacePoint> spacePoints(m_spacePointsName);


  spacePoints.registerAsPersistent();
  pxdClusters.isOptional();
  svdClusters.isOptional();


  RelationArray relSpacePointsPXDClusters(spacePoints, pxdClusters);
  RelationArray relSpacePointsSVDClusters(spacePoints, svdClusters);


  //Relations to simulation objects only if the ancestor relations exist
  if (pxdClusters.isOptional() == true) { relSpacePointsPXDClusters.registerAsPersistent(); }
  if (svdClusters.isOptional() == true) { relSpacePointsSVDClusters.registerAsPersistent(); }


  // retrieve names again (faster than doing everything in the event):
  m_pxdClustersName = pxdClusters.getName();
  m_svdClustersName = svdClusters.getName();
  m_spacePointsName = spacePoints.getName();
  m_relSpacePointsPXDClustersName = relSpacePointsPXDClusters.getName();
  m_relSpacePointsSVDClustersName = relSpacePointsSVDClusters.getName();


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

  if (spacePoints.isValid() == false) {
    spacePoints.create();
  } else {
    spacePoints.getPtr()->Clear();
  }

  RelationArray relSpacePointsPXDClusters(spacePoints, pxdClusters, m_relSpacePointsPXDClustersName);
  if (relSpacePointsPXDClusters) { relSpacePointsPXDClusters.clear(); }
  RelationArray relSpacePointsSVDClusters(spacePoints, svdClusters, m_relSpacePointsSVDClustersName);
  if (relSpacePointsSVDClusters) { relSpacePointsSVDClusters.clear(); }


  for (unsigned int i = 0; i < uint(pxdClusters.getEntries()); ++i) {
    spacePoints.appendNew((pxdClusters[i]), i);
  }


  for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
    SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};
    vector<SpacePoint::SVDClusterInformation> currentClusterCombi = { currentCluster };
    spacePoints.appendNew(currentClusterCombi);
  }

  B2DEBUG(1, "SpacePointCreatorModule::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "pxdClusters: " << pxdClusters.getEntries() <<
          ", svdClusters: " << svdClusters.getEntries() <<
          ", spacePoints: " << spacePoints.getEntries() <<
          ", relSpacePointsPXDClusters: " << relSpacePointsPXDClusters.getEntries() <<
          ", relSpacePointsSVDClusters: " << relSpacePointsSVDClusters.getEntries())

  /// WARNING TODO next steps: write simple SVDCluster-Combiner for spacepoints, create relations, think about mcParticle-relations, prepare converter for GFTrackCandidates including clusters to XXTrackCandidates including SpacePoints and vice versa.

  m_TESTERPXDClusterCtr += pxdClusters.getEntries();
  m_TESTERSVDClusterCtr += svdClusters.getEntries();
  m_TESTERSpacePointCtr += spacePoints.getEntries();
}

void SpacePointCreatorModule::terminate()
{
  B2INFO("SpacePointCreatorModule::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", svdClusters: " << m_TESTERSVDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}
