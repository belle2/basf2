/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/spacePointCreatorTEL/SpacePointCreatorTELModule.h>

#include <framework/logging/Logger.h>

#include <limits>       // std::numeric_limits



using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreatorTEL)

SpacePointCreatorTELModule::SpacePointCreatorTELModule() :
  Module()
{
  // set some counters for output:
  InitializeCounters();

  setDescription("Imports Clusters of the TEL detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("TelClusters", m_telClustersName,
           "TelCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));


  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
}



void SpacePointCreatorTELModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut);
  m_telClusters.isRequired(m_telClustersName);


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_telClusters, DataStore::c_Event, DataStore::c_DontWriteOut);


  B2INFO("SpacePointCreatorTELModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
         "\ntelClusters: " << m_telClusters.getName() <<
         "\nspacePoints: " << m_spacePoints.getName());


  // set some counters for output:
  InitializeCounters();
}



void SpacePointCreatorTELModule::event()
{

  for (unsigned int i = 0; i < uint(m_telClusters.getEntries()); ++i) {
    const TelCluster* currentCluster = m_telClusters[i];
    TBSpacePoint* newSP = m_spacePoints.appendNew((currentCluster));
    newSP->addRelationTo(currentCluster);
  }

  B2DEBUG(1, "SpacePointCreatorTELModule(" << m_nameOfInstance <<
          ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          "\ntelClusters: " << m_telClusters.getEntries() <<
          "\nspacePoints: " << m_spacePoints.getEntries());


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const TBSpacePoint* sp = m_spacePoints[index];

      B2DEBUG(10, "SpacePointCreatorTELModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getArrayName());
    }
  }

  m_TESTERTelClusterCtr += m_telClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void SpacePointCreatorTELModule::terminate()
{
  B2INFO("SpacePointCreatorTELModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
         "\ntelClusters: " << m_TESTERTelClusterCtr <<
         "\nspacePoints: " << m_TESTERSpacePointCtr);
}




void SpacePointCreatorTELModule::InitializeCounters()
{
  m_TESTERTelClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}

