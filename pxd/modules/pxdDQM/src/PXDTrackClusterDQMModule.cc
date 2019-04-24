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

    m_trackClusterCharge[avxdid] = new TH1F("PXD_Cluster_Charge_" + buff, "PXD Cluster Charge " + buff + ";Charge/ADU;", 256, 0, 256);
    m_trackClusterChargeFitted[avxdid] = new TH1F("PXD_Cluster_Charge_Fitted_" + buff,
                                                  "PXD Cluster Charge Fitted " + buff + ";Charge/ADU;", 256, 0, 256);
  }

//   m_trackClusterCharge2d = new TH2F( "PXD_Cluster_Charge", "PXD Cluster Charge";Charge/ADU;ModuleID", 256, 0, 256, nr of modules);

  oldDir->cd();

}

void PXDTrackClusterDQMModule::beginRun()
{
  for (auto& it : m_trackClusterCharge) it.second->Reset();
  for (auto& it : m_trackClusterChargeFitted) it.second->Reset();
}


void PXDTrackClusterDQMModule::event()
{
  for (const Track& track : m_tracks) {  // over tracks
    RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_RecoTracksStoreArrayName);
    if (!recoTrack.size()) continue;
    RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(recoTrack[0]);

    for (auto& cluster : pxdClustersTrack) {
      if (m_trackClusterCharge[cluster.getSensorID()]) m_trackClusterCharge[cluster.getSensorID()]->Fill(cluster.getCharge());
    }

    /// TODO some more cuts to be sure about track quality?
    const TrackFitResult* tfr = track.getTrackFitResultWithClosestMass(Const::pion);
    if (tfr == nullptr) continue;

    if (!recoTrack[0]->wasFitSuccessful()) continue;
    if (!recoTrack[0]->getTrackFitStatus()) continue;

    /*
            // add NDF:
            float NDF = recoTrack[0]->getTrackFitStatus()->getNdf();
            m_NDF->Fill(NDF);
            // add Chi2/NDF:
            m_Chi2->Fill(recoTrack[0]->getTrackFitStatus()->getChi2());
            if (NDF) {
              float Chi2NDF = recoTrack[0]->getTrackFitStatus()->getChi2() / NDF;
              m_Chi2NDF->Fill(Chi2NDF);
            }
            // add p-value:
            float pValue = recoTrack[0]->getTrackFitStatus()->getPVal();
            m_PValue->Fill(pValue);*/

    for (auto recoHitInfo : recoTrack[0]->getRecoHitInformations()) {  // over recohits
      if (!recoHitInfo) {
        continue;
      }
      if (!recoHitInfo->useInFit())
        continue;

      if (recoHitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) {
        VxdID sensorID = recoHitInfo->getRelatedTo<PXDCluster>()->getSensorID();
        if (m_trackClusterChargeFitted[sensorID]) m_trackClusterChargeFitted[sensorID]->Fill(
            recoHitInfo->getRelatedTo<PXDCluster>()->getCharge());
      }
    }
  }
}
