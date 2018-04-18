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
  addParam("DATCONSimpleSVDCluster", m_storeDATCONSimpleSVDClustersName,
           "DATCONSimpleSVDCluster collection name", string(""));
  addParam("DATCONSVDSpacePoints", m_storeDATCONSVDSpacePointsName,
           "DATCONSVDSpacePoints collection name", string(""));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "SVDTrueHits collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more",
           bool(false));
}


void DATCONSVDSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_storeDATCONSVDSpacePoints.registerInDataStore(m_storeDATCONSVDSpacePointsName,
                                                  DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_storeDATCONSimpleSVDClusters.isRequired(m_storeDATCONSimpleSVDClustersName);

  m_storeDATCONSVDSpacePointsName = m_storeDATCONSVDSpacePoints.getName();
  m_storeDATCONSimpleSVDClustersName = m_storeDATCONSimpleSVDClusters.getName();

  m_storeTrueHits.isOptional(m_storeTrueHitsName);
  if (m_storeTrueHits.isValid()) {
    m_storeTrueHitsName = m_storeTrueHits.getName();
    m_storeDATCONSVDSpacePoints.registerRelationTo(m_storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  m_storeMCParticles.isOptional(m_storeMCParticlesName);
  if (m_storeMCParticles.isValid()) {
    m_storeMCParticlesName = m_storeMCParticles.getName();
    m_storeDATCONSVDSpacePoints.registerRelationTo(m_storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  //Relations to cluster objects only if the ancestor relations exist:
  m_storeDATCONSVDSpacePoints.registerRelationTo(m_storeDATCONSimpleSVDClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

}

void DATCONSVDSpacePointCreatorModule::event()
{
  if (m_onlySingleClusterSpacePoints == true) {
    provideDATCONSVDClusterSingles(m_storeDATCONSimpleSVDClusters,
                                   m_storeDATCONSVDSpacePoints); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideDATCONSVDClusterCombinations(m_storeDATCONSimpleSVDClusters, m_storeDATCONSVDSpacePoints);
  }


  B2DEBUG(1, "DATCONSVDSpacePointCreatorModule(" << m_nameOfInstance <<
          ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          ", svdClusters: " << m_storeDATCONSimpleSVDClusters.getEntries() <<
          ", spacePoints: " << m_storeDATCONSVDSpacePoints.getEntries());

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_storeDATCONSVDSpacePoints.getEntries(); index++) {
      const DATCONSVDSpacePoint* sp = m_storeDATCONSVDSpacePoints[index];

      B2DEBUG(10, "DATCONSVDSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getArrayName());
    }
  }
}
