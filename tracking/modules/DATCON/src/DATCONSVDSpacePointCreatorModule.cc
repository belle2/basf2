/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONSVDSpacePointCreatorModule.h>

#include <tracking/modules/DATCON/DATCONSpacePointHelperFunctions.h>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;


REG_MODULE(DATCONSVDSpacePointCreator)

DATCONSVDSpacePointCreatorModule::DATCONSVDSpacePointCreatorModule() :
  Module()
{
  setDescription("Imports Clusters of the SVD detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("DATCONSVDCluster", m_storeDATCONSVDClustersName,
           "DATCONSVDCluster collection name", string("DATCONSVDCluster"));
  addParam("DATCONSVDSpacePoints", m_storeDATCONSVDSpacePointsName,
           "DATCONSVDSpacePoints collection name", string("DATCONSVDSpacePoints"));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "SVDTrueHits collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2.Modification parameters:
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more",
           bool(false));
}


void DATCONSVDSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  storeDATCONSVDSpacePoints.registerInDataStore(m_storeDATCONSVDSpacePointsName,
                                                DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  storeDATCONSVDClusters.isRequired(m_storeDATCONSVDClustersName);

  m_storeDATCONSVDSpacePointsName = storeDATCONSVDSpacePoints.getName();
  m_storeDATCONSVDClustersName = storeDATCONSVDClusters.getName();

  storeTrueHits.isOptional(m_storeTrueHitsName);
  if (storeTrueHits.isValid()) {
    m_storeTrueHitsName = storeTrueHits.getName();
    storeDATCONSVDSpacePoints.registerRelationTo(storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  storeMCParticles.isOptional(m_storeMCParticlesName);
  if (storeMCParticles.isValid()) {
    m_storeMCParticlesName = storeMCParticles.getName();
    storeDATCONSVDSpacePoints.registerRelationTo(storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  //Relations to cluster objects only if the ancestor relations exist:
  storeDATCONSVDSpacePoints.registerRelationTo(storeDATCONSVDClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

}

void DATCONSVDSpacePointCreatorModule::event()
{
  if (m_onlySingleClusterSpacePoints == true) {
    provideDATCONSVDClusterSingles(storeDATCONSVDClusters,
                                   storeDATCONSVDSpacePoints); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideDATCONSVDClusterCombinations(storeDATCONSVDClusters, storeDATCONSVDSpacePoints);
  }
}
