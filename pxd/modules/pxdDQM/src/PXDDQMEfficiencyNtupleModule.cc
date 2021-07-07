/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDQMEfficiencyNtupleModule.h>
#include <tracking/dataobjects/ROIid.h>

#include <pxd/reconstruction/PXDPixelMasker.h>
#include <mdst/dataobjects/Track.h>
#include <framework/gearbox/Const.h>

#include "TMatrixDSym.h"
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMEfficiencyNtuple)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMEfficiencyNtupleModule::PXDDQMEfficiencyNtupleModule() : Module(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("Create basic histograms for PXD efficiency");

  // setPropertyFlags(c_ParallelProcessingCertified);// for ntuple not certified!!!

  // Parameter definitions
  addParam("ntupleName", m_ntupleName, "name of ntuple file", std::string("test.root"));
  addParam("pxdClustersName", m_pxdClustersName, "name of StoreArray with PXD cluster", std::string(""));
  addParam("recoTracksName", m_recoTracksName, "name of StoreArray with RecoTracks", std::string(""));
  addParam("tracksName", m_tracksName, "name of StoreArray with Tracks", std::string(""));
  addParam("ROIsName", m_ROIsName, "name of the list of HLT ROIs, if available in output", std::string(""));
  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", true);
  addParam("pCut", m_pcut, "Set a cut on the track fit p-value (0=no cut)", double(0));
  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered", 0u);
  addParam("momCut", m_momCut, "Set a cut on the track momentum in GeV/c, 0 disables", double(0));
  addParam("pTCut", m_pTCut, "Set a cut on the track pT in GeV/c, 0 disables", double(0));
  addParam("maskedDistance", m_maskedDistance, "Distance inside which no masked pixel or sensor border is allowed", int(10));
}


void PXDDQMEfficiencyNtupleModule::terminate()
{
  auto dir = gDirectory;
  if (m_tuple) {
    if (m_file) { // no file -> no write to file
      m_file->cd();
    }
    m_tuple->Write();
    delete m_tuple;
    m_tuple = nullptr;
  }
  if (m_file) {
    m_file->Write();
    m_file->Close();
    delete m_file;
    m_file = nullptr;
  }
  dir->cd();
}


void PXDDQMEfficiencyNtupleModule::initialize()
{
  m_file = new TFile(m_ntupleName.data(), "recreate");
  if (m_file) m_file->cd();
  m_tuple = new TNtuple("effcontrol", "effcontrol",
                        "vxdid:u:v:p:pt:distu:distv:sigu:sigv:dist:inroi:clborder:cldead:matched:z0:d0:svdhits:charge:phi:costheta");

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_pxdclusters.isOptional(m_pxdClustersName);
  m_recoTracks.isOptional(m_recoTracksName);
  m_tracks.isOptional(m_tracksName);
  m_ROIs.isOptional(m_ROIsName);
  m_intercepts.isOptional(m_PXDInterceptListName);
}


void PXDDQMEfficiencyNtupleModule::event()
{
  if (!m_pxdclusters.isValid()) {
    B2INFO("PXDClusters array is missing, no efficiencies");
    return;
  }
  if (!m_recoTracks.isValid()) {
    B2INFO("RecoTrack array is missing, no efficiencies");
    return;
  }
  if (!m_tracks.isValid()) {
    B2INFO("Track array is missing, no efficiencies");
    return;
  }
  if (!m_intercepts.isValid()) {
    B2INFO("Intercept array is missing, no efficiencies");
    return;
  }

  for (auto& track : m_tracks) {
    RelationVector<RecoTrack> recoTrack = track.getRelationsTo<RecoTrack>(m_recoTracksName);
    if (!recoTrack.size()) continue;

    auto a_track = recoTrack[0];
    //If fit failed assume position pointed to is useless anyway
    if (!a_track->wasFitSuccessful()) continue;

    if (a_track->getNumberOfSVDHits() < m_minSVDHits) continue;

    RelationVector<PXDIntercept> interceptList = a_track->getRelationsTo<PXDIntercept>(m_PXDInterceptListName);
    if (!interceptList.size()) continue;

    const genfit::FitStatus* fitstatus = a_track->getTrackFitStatus();
    if (fitstatus->getPVal() < m_pcut) continue;

    genfit::MeasuredStateOnPlane trackstate;
    trackstate = a_track->getMeasuredStateOnPlaneFromFirstHit();
    if (trackstate.getMom().Mag() < m_momCut) continue;
    if (trackstate.getMom().Pt() < m_pTCut) continue;

    const TrackFitResult* ptr2 = track.getTrackFitResultWithClosestMass(Const::pion);
    if (!ptr2) {
      B2ERROR("expect a track fit result for mass");
      continue;
    }

    //loop over all PXD sensors to get the intersections
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (auto intercept : interceptList) {
      auto aVxdID = intercept.getSensorID();
      auto& info = m_vxdGeometry.getSensorInfo(aVxdID);
      //Search for intersections of the track with all PXD layers
      //Traditional (aka the person before did it like this) method
      //If there is a way to find out sensors crossed by a track directly, that would most likely be faster

      double sigu = intercept.getSigmaU();
      double sigv = intercept.getSigmaV();

      double u_fit = intercept.getCoorU();
      double v_fit = intercept.getCoorV();

      int ucell_fit = info.getUCellID(u_fit); // check wie overflow!!!
      int vcell_fit = info.getVCellID(v_fit); // Check wie overflow

      bool closeToBoarder = false;
      if (isCloseToBorder(ucell_fit, vcell_fit, m_maskedDistance)) {
        closeToBoarder = true;
      }

      bool closeToDead = false;
      if (isDeadPixelClose(ucell_fit, vcell_fit, m_maskedDistance, aVxdID)) {
        closeToDead = true;
      }

      bool fitInsideROI = false;
      if (m_ROIs.isValid()) {
        //Check if the intersection is inside a ROI
        //If not, even if measured the cluster was thrown away->Not PXD's fault
        for (auto& roit : m_ROIs) {
          if (aVxdID != roit.getSensorID()) {
            continue; //ROI on other sensor
          }

          if (ucell_fit < roit.getMaxUid()
              && ucell_fit > roit.getMinUid()
              && vcell_fit < roit.getMaxVid()
              && vcell_fit > roit.getMinVid()) {
            fitInsideROI = true;
          }
        }
      }

      //Now check if the sensor measured a hit here

      int bestcluster = findClosestCluster(aVxdID, TVector3(u_fit, v_fit, 0));
      double du_clus = 0;
      double dv_clus = 0;
      double d_clus = 0;
      float charge = 0;
      bool matched = false;
      if (bestcluster >= 0) {
        double u_clus = m_pxdclusters[bestcluster]->getU();
        double v_clus = m_pxdclusters[bestcluster]->getV();

        //is the closest cluster close enough to the track to count as measured?
        TVector3 dist_clus(u_fit - u_clus, v_fit - v_clus, 0);
        du_clus = u_fit - u_clus;
        dv_clus = v_fit - v_clus;
        d_clus = dist_clus.Mag();
        charge = m_pxdclusters[bestcluster]->getCharge();
        matched = true;
      }
      float fill[22] = {float((int)aVxdID), float(u_fit), float(v_fit), float(trackstate.getMom().Mag()), float(trackstate.getMom().Pt()),
                        float(du_clus), float(dv_clus), float(sigu), float(sigv), float(d_clus),
                        float(fitInsideROI), float(closeToBoarder), float(closeToDead), float(matched),
                        float(ptr2->getZ0()), float(ptr2->getD0()), float(a_track->getNumberOfSVDHits()),
                        charge, float(trackstate.getMom().Phi()), float(trackstate.getMom().CosTheta())
                       };
      m_tuple->Fill(fill);
    }
  }
}

int
PXDDQMEfficiencyNtupleModule::findClosestCluster(const VxdID& avxdid, TVector3 intersection)
{
  int closest = -1;
  double mindist = 999999999999; //definitely outside of the sensor

  VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);

  //loop the clusters
  for (int iclus = 0; iclus < m_pxdclusters.getEntries(); iclus++) {
    //Do not consider as different if only segment differs!
    //As of this writing segment is never filled for clusters, but just to be sure
    VxdID clusterID = m_pxdclusters[iclus]->getSensorID();
    if (avxdid.getLayerNumber() != clusterID.getLayerNumber() ||
        avxdid.getLadderNumber() != clusterID.getLadderNumber() ||
        avxdid.getSensorNumber() != clusterID.getSensorNumber()) {
      continue;
    }
    //only cluster on the correct sensor and direction should survive

    double u = m_pxdclusters[iclus]->getU();
    double v = m_pxdclusters[iclus]->getV();
    TVector3 current(u, v, 0);

    //2D dist sqared
    double dist = (intersection - current).Mag();
    if (dist < mindist) {
      closest = iclus;
      mindist = dist;
    }
  }

  return closest;

}

bool PXDDQMEfficiencyNtupleModule::isCloseToBorder(int u, int v, int checkDistance)
{

  if (u - checkDistance < 0 || u + checkDistance >= 250 ||
      v - checkDistance < 0 || v + checkDistance >= 768) {
    return true;
  }
  return false;
}

bool PXDDQMEfficiencyNtupleModule::isDeadPixelClose(int u, int v, int checkDistance, const VxdID& moduleID)
{

  //Iterate over square around the intersection to see if any close pixel is dead
  for (int u_iter = u - checkDistance; u_iter <= u + checkDistance ; ++u_iter) {
    for (int v_iter = v - checkDistance; v_iter <= v + checkDistance ; ++v_iter) {
      if (PXD::PXDPixelMasker::getInstance().pixelDead(moduleID, u_iter, v_iter)
          || !PXD::PXDPixelMasker::getInstance().pixelOK(moduleID, u_iter, v_iter)) {
        return true;
      }
    }
  }
  return false;
}
