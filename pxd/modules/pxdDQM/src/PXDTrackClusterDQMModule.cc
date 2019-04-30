/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Björn Spruck                                             *
 * Use: PXD-Tracking-Cluster DQM                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDTrackClusterDQMModule.h>
#include <framework/datastore/RelationArray.h>
#include <TDirectory.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDTrackClusterDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDTrackClusterDQMModule::PXDTrackClusterDQMModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("DQM for PXD Cluster matched to a Track");
  setPropertyFlags(c_ParallelProcessingCertified);

}


//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDTrackClusterDQMModule::initialize()
{
  m_recoTracks.isOptional(m_RecoTracksStoreArrayName);
  m_tracks.isOptional(m_TracksStoreArrayName);

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

}

void PXDTrackClusterDQMModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PXDER");
  oldDir->cd("PXDER");

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    //Only interested in PXD sensors

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    m_trackClusterCharge[avxdid] = new TH1F("PXD_Track_Cluster_Charge_" + buff, "PXD Track Cluster Charge " + buff + ";Charge/ADU;",
                                            256, 0, 256);
  }

  oldDir->cd();

}

void PXDTrackClusterDQMModule::beginRun()
{
  for (auto& it : m_trackClusterCharge) it.second->Reset();
}


void PXDTrackClusterDQMModule::event()
{
  for (const Track& track : m_tracks) {
    /// loop over tracks, this means that all clusters attached are already
    /// used in the fit with reasonable prob and other cuts. Thus no add
    /// cleaning or cutting should be needed.
    RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_RecoTracksStoreArrayName);
    if (!recoTrack.size()) continue;
    RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack[0]);

    for (auto& cluster : pxdClustersTrack) {
      if (m_trackClusterCharge[cluster.getSensorID()]) m_trackClusterCharge[cluster.getSensorID()]->Fill(cluster.getCharge());
    }
  }
}
