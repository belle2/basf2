/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdSpacePointCreator/PXDSpacePointCreatorModule.h>

#include <framework/logging/Logger.h>




using namespace std;
using namespace Belle2;


REG_MODULE(PXDSpacePointCreator);

PXDSpacePointCreatorModule::PXDSpacePointCreatorModule() :
  Module()
{
  InitializeCounters();

  setDescription("Imports Clusters of the PXD detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string("PXDSpacePoints"));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module",
           string("PXDSpacePoints"));
}



void PXDSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_pxdClusters.isRequired(m_pxdClustersName);
  m_eventLevelTrackingInfo.isOptional();


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_pxdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);


  B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
          "pxdClusters: " << m_pxdClusters.getName() <<
          "\nspacePoints: " << m_spacePoints.getName());


  // set some counters for output:
  InitializeCounters();
}



void PXDSpacePointCreatorModule::event()
{
  // Abort in case SVDSpacePointCreator was aborted (high occupancy events)
  // as we do not add PXD hits to CDC standalone tracks
  if (m_eventLevelTrackingInfo.isValid()) {
    if (m_eventLevelTrackingInfo->hasSVDSpacePointCreatorAbortionFlag()) {
      return;
    }
  }

  for (unsigned int i = 0; i < uint(m_pxdClusters.getEntries()); ++i) {
    const PXDCluster* currentCluster = m_pxdClusters[i];
    SpacePoint* newSP = m_spacePoints.appendNew((currentCluster));
    newSP->addRelationTo(currentCluster);
  }

  B2DEBUG(1, "PXDSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoints created! Size of arrays:\n" <<
          "pxdClusters: " << m_pxdClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries());


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      PXDCluster* relatedCluster = sp->getRelatedTo<PXDCluster>(m_pxdClusters.getName());

      B2DEBUG(10, "SpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is stored in Array: " << sp->getArrayName() <<
              " and the related PXDCluster is in " << relatedCluster->getArrayName() <<
              " with index " << relatedCluster->getArrayIndex());
    }
  }

  m_TESTERPXDClusterCtr += m_pxdClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();
}



void PXDSpacePointCreatorModule::terminate()
{
  B2DEBUG(10, "PXDSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
          "pxdClusters: " << m_TESTERPXDClusterCtr <<
          ", spacePoints: " << m_TESTERSpacePointCtr);
}


void PXDSpacePointCreatorModule::InitializeCounters()
{
  m_TESTERPXDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}



