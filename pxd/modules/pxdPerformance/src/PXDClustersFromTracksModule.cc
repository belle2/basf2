/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdPerformance/PXDClustersFromTracksModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <framework/datastore/StoreArray.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClustersFromTracks)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClustersFromTracksModule::PXDClustersFromTracksModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription("PXDClustersFromTracks module for creating a new StoreArray of track matched PXDCluster.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("PXDClustersName", m_pxdClustersName, "StoreArray name of the input PXD clusters", std::string("PXDClusters"));
  addParam("TracksName", m_tracksName, "StoreArray name of the input tracks", std::string("Tracks"));
  addParam("RecoTracksName", m_recoTracksName, "StoreArray name of the input recoTracks", std::string("RecoTracks"));
  addParam("outputArrayName", m_outputArrayName, "StoreArray name of the output PXD clusters",
           std::string("PXDClustersFromTracks"));
  addParam("InheritRelations", m_inheritance,
           "Set true if you want to inherit PXDCluster relation with StoreArray RecoTracks, the default is true", bool(true));
}

PXDClustersFromTracksModule::~PXDClustersFromTracksModule()
{
  B2DEBUG(20, "Destructor");
}


void PXDClustersFromTracksModule::initialize()
{

  B2DEBUG(10, "PXDClustersName: " << m_pxdClustersName);
  B2DEBUG(10, "TracksName: " << m_tracksName);
  B2DEBUG(10, "RecoTracksName: " << m_recoTracksName);
  B2DEBUG(10, "outputArrayName: " <<  m_outputArrayName);
  B2DEBUG(10, "InheritRelations: " <<  m_inheritance);

  StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);
  StoreArray<RecoTrack> recoTracks(m_recoTracksName);
  StoreArray<Track> tracks(m_tracksName);
  pxdClusters.isRequired();
  recoTracks.isRequired();
  tracks.isRequired();
  m_selectedPXDClusters.registerSubset(pxdClusters, m_outputArrayName);
  if (m_inheritance) {
    m_selectedPXDClusters.inheritRelationsTo(recoTracks);
  }

}


void PXDClustersFromTracksModule::beginRun()
{
}


void PXDClustersFromTracksModule::event()
{
  StoreArray<PXDCluster> pxdClusters(m_pxdClustersName);

  m_selectedPXDClusters.select([](const PXDCluster * thePXDCluster) {
    return isRelatedToTrack(thePXDCluster);
  });

}

void PXDClustersFromTracksModule::endRun()
{
}

void PXDClustersFromTracksModule::terminate()
{
}

bool PXDClustersFromTracksModule::isRelatedToTrack(const PXDCluster* pxdCluster)
{
  RelationVector<RecoTrack> recoTrack_cluster = pxdCluster->getRelationsTo<RecoTrack>();
  if (recoTrack_cluster.size() == 0) {
    return false;
  } else {
    RelationVector<Track> track_recoTrack = recoTrack_cluster[0]->getRelationsFrom<Track>();
    if (!track_recoTrack.size()) {
      return false;
    } else {
      return true;
    }
  }
}
