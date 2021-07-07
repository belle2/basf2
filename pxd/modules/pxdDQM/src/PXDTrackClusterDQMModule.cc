/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDTrackClusterDQMModule.h>
#include <pxd/unpacking/PXDMappingLookup.h>
#include <pxd/geometry/SensorInfo.h>
#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::PXD;

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

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDER"));
  addParam("moreHistos", m_moreHistos, "Fill additional histograms (not for ereco)", false);
  addParam("ASICHistos", m_ASICHistos, "Fill additional histograms ASIC combination", true);
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
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    //Only interested in PXD sensors

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    m_trackClusterCharge[avxdid] = new TH1F("PXD_Track_Cluster_Charge_" + buff, "PXD Track Cluster Charge " + buff + ";Charge/ADU;",
                                            100, 0, 100);
    if (m_moreHistos) {
      m_trackClusterChargeUC[avxdid] = new TH1F("PXD_Track_Cluster_Charge_UC_" + buff,
                                                "PXD Track Cluster Charge (uncorrected)" + buff + ";Charge/ADU;", 100, 0, 100);
    }
    if (m_ASICHistos) {
      // for now, we only want to have this module in
      if (avxdid == VxdID("1.5.1")) {
        for (int s = 0; s < 6; s++) {
          for (int d = 0; d < 4; d++) {
            m_trackASICClusterCharge[avxdid][s][d] = new TH1F("PXD_Track_Cluster_Charge_" + buff + Form("_sw%d_dcd%d", s + 1, d + 1),
                                                              "PXD Track Cluster Charge " + buff + Form(" sw%d dcd%d ", s + 1, d + 1) + ";Charge/ADU;", 100, 0, 100);
          }
        }
      }
    }
  }

  m_trackedClusters = new TH1F("PXD_Tracked_Clusters", "PXD_Tracked_Clusters", 64, 0, 64);

  for (auto i = 0; i < 64; i++) {
    auto layer = (((i >> 5) & 0x1) + 1);
    auto ladder = ((i >> 1) & 0xF);
    auto sensor = ((i & 0x1) + 1);

    auto id = Belle2::VxdID(layer, ladder, sensor);
    // Check if sensor exist
    if (Belle2::VXD::GeoCache::getInstance().validSensorID(id)) {
      m_vxd_to_dhe[id] = i;
    }
  }

  oldDir->cd();

}

void PXDTrackClusterDQMModule::beginRun()
{
  for (auto& it : m_trackClusterCharge) if (it.second) it.second->Reset();
  for (auto& it : m_trackClusterChargeUC) if (it.second) it.second->Reset();
  if (m_trackedClusters) m_trackedClusters->Reset();
  for (const auto& it1 : m_trackASICClusterCharge) {
    for (const auto& it2 : it1.second) {
      for (const auto& it3 : it2) {
        if (it3) it3->Reset();
      }
    }
  }
}


void PXDTrackClusterDQMModule::event()
{
  if (m_trackedClusters) m_trackedClusters->Fill(-1); // Underflow as event counter
  for (const Track& track : m_tracks) {
    /// loop over tracks, this means that all clusters attached are already
    /// used in the fit with reasonable prob and other cuts. Thus no add
    /// cleaning or cutting should be needed.
    RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_RecoTracksStoreArrayName);
    if (!recoTrack.size()) continue;
    RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack[0]);

    const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
    double correction = 1.0;
    if (tfr) correction = sin(tfr->getMomentum().Theta());
    for (auto& cluster : pxdClustersTrack) {
      m_trackedClusters->Fill(m_vxd_to_dhe[cluster.getSensorID()]);
      if (m_trackClusterChargeUC[cluster.getSensorID()]) m_trackClusterChargeUC[cluster.getSensorID()]->Fill(cluster.getCharge());
      if (tfr && m_trackClusterCharge[cluster.getSensorID()]) m_trackClusterCharge[cluster.getSensorID()]->Fill(
          cluster.getCharge()*correction);
      if (m_ASICHistos && tfr) {
        // for now, we only want to have this module in
        if (cluster.getSensorID() == VxdID("1.5.1")) {
          auto SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(cluster.getSensorID()));
          auto d = PXDMappingLookup::getDCDID(SensorInfo.getUCellID(cluster.getU()), SensorInfo.getVCellID(cluster.getV()),
                                              cluster.getSensorID());
          auto s = PXDMappingLookup::getSWBID(SensorInfo.getVCellID(cluster.getV()));

          TH1F* h = nullptr;
          try {
            h = m_trackASICClusterCharge[cluster.getSensorID()].at(s - 1).at(d - 1);
          } catch (...) {
          }
          if (h) h->Fill(cluster.getCharge()*correction);
        }
      }
    }
  }
}
