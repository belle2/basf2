/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SpacePointCreatorPXDModule.h>

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreatorPXD)

SpacePointCreatorPXDModule::SpacePointCreatorPXDModule() :
  Module()
{
  InitializeCounters();

  setDescription("Imports Clusters of the PXD detector and converts them to spacePoints.");

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string("PXDSpacePoints"));
}



void SpacePointCreatorPXDModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut);
  m_pxdClusters.isRequired(m_pxdClustersName);


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);


  B2INFO("SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "pxdClusters: " << m_pxdClusters.getName() <<
         "\nspacePoints: " << m_spacePoints.getName())


  // set some counters for output:
  InitializeCounters();
}



void SpacePointCreatorPXDModule::event()
{

  storeSingleCluster(m_pxdClusters, m_spacePoints);


  B2DEBUG(1, "SpacePointCreatorPXDModule(" << m_nameOfInstance << ")::event: spacePoints created! Size of arrays:\n" <<
          "pxdClusters: " << m_pxdClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries())


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      PXDCluster* relatedCluster = sp->getRelatedTo<PXDCluster>(m_pxdClusters.getName());

      B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is stored in Array: " << sp->getArrayName() <<
              " and the related PXDCluster is in " << relatedCluster->getArrayName() <<
              " with index " << relatedCluster->getArrayIndex())
    }
  }

  m_TESTERPXDClusterCtr += m_pxdClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void SpacePointCreatorPXDModule::terminate()
{
  B2INFO("SpacePointCreatorPXDModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "pxdClusters: " << m_TESTERPXDClusterCtr <<
         ", spacePoints: " << m_TESTERSpacePointCtr)
}


void SpacePointCreatorPXDModule::InitializeCounters()
{
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}
