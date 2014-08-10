/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <unordered_map>


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


  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\nspacePoints: " << m_spacePointsName <<
         "\nrelSpacePointsPXDClusters: " << m_relSpacePointsPXDClustersName <<
         "\nrelSpacePointsSVDClusters: " << m_relSpacePointsSVDClustersName)

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


  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(svdClusters, spacePoints);
  } else {
    provideSVDClusterCombinations(svdClusters, spacePoints);
  }




  B2DEBUG(1, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
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
  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", svdClusters: " << m_TESTERSVDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}



/// simply store one spacePoint for each SVDCluster:
void SpacePointCreatorModule::provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters, StoreArray<SpacePoint>& spacePoints)
{
  for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
    SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};
    vector<SpacePoint::SVDClusterInformation> currentClusterCombi = { currentCluster };
    spacePoints.appendNew(currentClusterCombi);
  }
}



/// stores all possible 2-Cluster-combinations
void SpacePointCreatorModule::provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters, StoreArray<SpacePoint>& spacePoints)
{
  unordered_map<VxdID::baseType, ClustersOnSensor> activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
  vector<vector<SpacePoint::SVDClusterInformation> > foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)


  // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
  for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
    SpacePoint::SVDClusterInformation currentCluster = {svdClusters[i], i};

    activatedSensors[svdClusters[i]->getSensorID().getID()].addCluster(currentCluster);
  }


  for (auto & aSensor : activatedSensors) {
    findPossibleCombinations(aSensor.second, foundCombinations);
  }


  for (auto & clusterCombi : foundCombinations) {
    spacePoints.appendNew(clusterCombi);
  }
}
