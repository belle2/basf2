/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/modules/vtxSpacePointCreator/VTXSpacePointCreatorModule.h>

#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(VTXSpacePointCreator)

VTXSpacePointCreatorModule::VTXSpacePointCreatorModule() :
  Module()
{
  InitializeCounters();

  setDescription("Imports Clusters of the VXT detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("VTXClusters", m_vtxClustersName,
           "VTXCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string("VTXSpacePoints"));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module",
           string("VTXSpacePoints"));
}



void VTXSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_vtxClusters.isRequired(m_vtxClustersName);


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_vtxClusters, DataStore::c_Event, DataStore::c_DontWriteOut);


  B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
          "vtxClusters: " << m_vtxClusters.getName() <<
          "\nspacePoints: " << m_spacePoints.getName());


  // set some counters for output:
  InitializeCounters();
}



void VTXSpacePointCreatorModule::event()
{

  for (unsigned int i = 0; i < uint(m_vtxClusters.getEntries()); ++i) {
    const VTXCluster* currentCluster = m_vtxClusters[i];
    SpacePoint* newSP = m_spacePoints.appendNew((currentCluster));
    newSP->addRelationTo(currentCluster);
  }

  B2DEBUG(1, "VTXSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints created! Size of arrays:\n" <<
          "vtxClusters: " << m_vtxClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries());


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      VTXCluster* relatedCluster = sp->getRelatedTo<VTXCluster>(m_vtxClusters.getName());

      B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is stored in Array: " << sp->getArrayName() <<
              " and the related VTXCluster is in " << relatedCluster->getArrayName() <<
              " with index " << relatedCluster->getArrayIndex());
    }
  }

  m_TESTERVTXClusterCtr += m_vtxClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void VTXSpacePointCreatorModule::terminate()
{
  B2DEBUG(10, "VTXSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
          "vtxClusters: " << m_TESTERVTXClusterCtr <<
          ", spacePoints: " << m_TESTERSpacePointCtr);
}


void VTXSpacePointCreatorModule::InitializeCounters()
{
  m_TESTERVTXClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



