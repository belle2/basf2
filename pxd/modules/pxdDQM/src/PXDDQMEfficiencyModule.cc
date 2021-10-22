/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDQMEfficiencyModule.h>
#include <tracking/dataobjects/ROIid.h>

#include <pxd/reconstruction/PXDPixelMasker.h>
#include <mdst/dataobjects/Track.h>
#include <framework/gearbox/Const.h>

#include "TDirectory.h"
#include "TMatrixDSym.h"
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMEfficiencyModule::PXDDQMEfficiencyModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("Create basic histograms for PXD efficiency");

  // What exactly is needed for this to be true?
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("pxdClustersName", m_pxdClustersName, "name of StoreArray with PXD cluster", std::string(""));
  addParam("tracksName", m_tracksName, "name of StoreArray with RecoTracks", std::string(""));
  addParam("ROIsName", m_ROIsName, "name of the list of HLT ROIs, if available in output", std::string(""));
  addParam("PXDInterceptListName", m_PXDInterceptListName, "name of the list of interceptions", std::string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDEFF"));
  addParam("binsU", m_u_bins, "histogram bins in u direction", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction", int(6));
  addParam("distCut", m_distcut, "max distance in [cm] for cluster to be counted to a track", double(0.0500));
  addParam("pCut", m_pcut, "Set a cut on the track p-value", double(1e-20));
  addParam("requireROIs", m_requireROIs, "require tracks to lie inside a ROI", bool(false));
  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", bool(true));
  addParam("maskDeadPixels", m_maskDeadPixels, "Do not consider tracks going through known dead or hot pixels for the efficiency",
           bool(false));
  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered", 0u);
  addParam("momCut", m_momCut, "Set a cut on the track momentum in GeV/c, 0 disables", double(0));
  addParam("pTCut", m_pTCut, "Set a cut on the track pT in GeV/c, 0 disables", double(1));
  addParam("cutBorders", m_cutBorders, "Do not use tracks near the borders of the sensor", bool(true));
  addParam("maskedDistance", m_maskedDistance, "Distance inside which no masked pixel or sensor border is allowed", int(10));
  addParam("trackUFactorDistCut", m_uFactor, "Set a cut on u error of track (factor*err<dist), 0 disables", double(2.0));
  addParam("trackVFactorDistCut", m_vFactor, "Set a cut on v error of track (factor*err<dist), 0 disables", double(2.0));
  addParam("z0minCut", m_z0minCut, "Set a cut z0 minimum in cm (large negativ value eg -9999 disables)", double(-1));
  addParam("z0maxCut", m_z0maxCut, "Set a cut z0 maximum in cm (large positiv value eg 9999 disables)", double(1));
  addParam("d0Cut", m_d0Cut, "Set a cut abs(d0) in cm (large positiv value eg 9999 disables)", double(0.5));
  addParam("verboseHistos", m_verboseHistos, "Add more verbose histograms for cuts (not for ereoc)", bool(false));
}


void PXDDQMEfficiencyModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM;

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_pxdclusters.isOptional(m_pxdClustersName);
  m_tracks.isOptional(m_tracksName);
  m_ROIs.isOptional(m_ROIsName);
  m_intercepts.isOptional(m_PXDInterceptListName);
}

void PXDDQMEfficiencyModule::beginRun()
{
  for (auto& h : m_h_track_hits) if (h.second) h.second->Reset();
  for (auto& h : m_h_matched_cluster) if (h.second) h.second->Reset();
  for (auto& h : m_h_p) if (h.second) h.second->Reset();
  for (auto& h : m_h_pt) if (h.second) h.second->Reset();
  for (auto& h : m_h_su) if (h.second) h.second->Reset();
  for (auto& h : m_h_sv) if (h.second) h.second->Reset();
  for (auto& h : m_h_p2) if (h.second) h.second->Reset();
  for (auto& h : m_h_pt2) if (h.second) h.second->Reset();
  for (auto& h : m_h_su2) if (h.second) h.second->Reset();
  for (auto& h : m_h_sv2) if (h.second) h.second->Reset();
}

void PXDDQMEfficiencyModule::event()
{
  if (!m_pxdclusters.isValid()) {
    B2INFO("PXDClusters array is missing, no efficiencies");
    return;
  }
  if (!m_tracks.isValid()) {
    B2INFO("RecoTrack array is missing, no efficiencies");
    return;
  }
  if (!m_ROIs.isValid() && m_requireROIs) {
    B2INFO("ROI array is missing but required hits in ROIs, aborting");
    return;
  }
  if (!m_intercepts.isValid()) {
    B2INFO("Intercept array is missing, no efficiencies");
    return;
  }


  for (auto& a_track : m_tracks) {

    //If fit failed assume position pointed to is useless anyway
    if (!a_track.wasFitSuccessful()) continue;

    if (a_track.getNumberOfSVDHits() < m_minSVDHits) continue;

    RelationVector<PXDIntercept> interceptList = a_track.getRelationsTo<PXDIntercept>(m_PXDInterceptListName);
    if (!interceptList.size()) continue;

    const genfit::FitStatus* fitstatus = a_track.getTrackFitStatus();
    if (fitstatus->getPVal() < m_pcut) continue;

    genfit::MeasuredStateOnPlane trackstate;
    trackstate = a_track.getMeasuredStateOnPlaneFromFirstHit();
    if (trackstate.getMom().Mag() < m_momCut) continue;
    if (trackstate.getMom().Pt() < m_pTCut) continue;

    auto ptr = a_track.getRelated<Track>("Tracks");

    if (!ptr) {
      B2ERROR("expect a track for fitted recotracks");
      continue;
    }
    auto ptr2 = ptr->getTrackFitResultWithClosestMass(Const::pion);
    if (!ptr2) {
      B2ERROR("expect a track fit result for mass");
      continue;
    }

    // Vertex cut
    if (ptr2->getZ0() < m_z0minCut || ptr2->getZ0() > m_z0maxCut || fabs(ptr2->getD0()) > m_d0Cut) continue;

    //loop over all PXD sensors to get the intersections
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (auto intercept : interceptList) {
      auto const aVxdID = intercept.getSensorID();
      auto&  info = m_vxdGeometry.getSensorInfo(aVxdID);
      //Search for intersections of the track with all PXD layers
      //Traditional (aka the person before did it like this) method
      //If there is a way to find out sensors crossed by a track directly, that would most likely be faster

      //true = track intersects current sensor
      double sigu(-9999);
      double sigv(-9999);
      {
        if (m_verboseHistos) {
          if (m_h_p[aVxdID]) m_h_p[aVxdID]->Fill(trackstate.getMom().Mag());
          if (m_h_pt[aVxdID]) m_h_pt[aVxdID]->Fill(trackstate.getMom().Pt());
          if (m_h_su[aVxdID]) m_h_su[aVxdID]->Fill(sigu);
          if (m_h_sv[aVxdID]) m_h_sv[aVxdID]->Fill(sigv);
        }
        if (m_uFactor * sigu > m_distcut) continue; // Error ufak*SigmaU > cut
        if (m_vFactor * sigv > m_distcut) continue; // Error vfak*SigmaV > cut

        double u_fit = intercept.getCoorU();
        double v_fit = intercept.getCoorV();

        int ucell_fit = info.getUCellID(u_fit); // check wie overflow!!!
        int vcell_fit = info.getVCellID(v_fit); // Check wie overflow

        if (m_cutBorders && isCloseToBorder(ucell_fit, vcell_fit, m_maskedDistance)) {
          continue;
        }

        if (m_maskDeadPixels && isDeadPixelClose(ucell_fit, vcell_fit, m_maskedDistance, aVxdID)) {
          continue;
        }

        if (m_requireROIs) {
          //Check if the intersection is inside a ROI
          //If not, even if measured the cluster was thrown away->Not PXD's fault
          bool fitInsideROI = false;
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
          if (!fitInsideROI) {
            continue;//Hit wouldn't have been recorded
          }
        }

        //This track should be on the sensor
        m_h_track_hits[aVxdID]->Fill(ucell_fit, vcell_fit);

        //Now check if the sensor measured a hit here

        int bestcluster = findClosestCluster(aVxdID, TVector3(u_fit, v_fit, 0));
        if (bestcluster >= 0) {
          double u_clus = m_pxdclusters[bestcluster]->getU();
          double v_clus = m_pxdclusters[bestcluster]->getV();

          //is the closest cluster close enough to the track to count as measured?
          TVector3 dist_clus(u_fit - u_clus, v_fit - v_clus, 0);
          if (dist_clus.Mag() <= m_distcut)  {
            m_h_matched_cluster[aVxdID]->Fill(ucell_fit, vcell_fit);
            if (m_verboseHistos) {
              if (m_h_p2[aVxdID]) m_h_p2[aVxdID]->Fill(trackstate.getMom().Mag());
              if (m_h_pt2[aVxdID]) m_h_pt2[aVxdID]->Fill(trackstate.getMom().Pt());
              if (m_h_su2[aVxdID]) m_h_su2[aVxdID]->Fill(sigu);
              if (m_h_sv2[aVxdID]) m_h_sv2[aVxdID]->Fill(sigv);
            }
          }
        }
      }
    }
  }
}

void PXDDQMEfficiencyModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    //Only interested in PXD sensors

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    int nu = info.getUCells();
    int nv = info.getVCells();

    //nu + 1,nv + 1 Bin numbers when using one bin per pixel
    m_h_track_hits[avxdid] = new TH2F("track_hits_" + buff, "tracks through sensor " + buff,
                                      m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    m_h_matched_cluster[avxdid] = new TH2F("matched_cluster_" + buff, "clusters matched to track intersections " + buff,
                                           m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);

    if (m_verboseHistos) {
      m_h_p[avxdid] = new TH1F("p_" + buff, "p " + buff, 100, 0, 10);
      m_h_pt[avxdid] = new TH1F("pt_" + buff, "pt " + buff, 100, 0, 10);
      m_h_su[avxdid] = new TH1F("su_" + buff, "su " + buff, 1000, 0, 1);
      m_h_sv[avxdid] = new TH1F("sv_" + buff, "sv " + buff, 1000, 0, 1);
      m_h_p2[avxdid] = new TH1F("p2_" + buff, "p2 " + buff, 100, 0, 10);
      m_h_pt2[avxdid] = new TH1F("pt2_" + buff, "pt2 " + buff, 100, 0, 10);
      m_h_su2[avxdid] = new TH1F("su2_" + buff, "su2 " + buff, 1000, 0, 1);
      m_h_sv2[avxdid] = new TH1F("sv2_" + buff, "sv2 " + buff, 1000, 0, 1);
    }
  }
  // cd back to root directory
  oldDir->cd();
}


int
PXDDQMEfficiencyModule::findClosestCluster(const VxdID& avxdid, TVector3 intersection)
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

bool PXDDQMEfficiencyModule::isCloseToBorder(int u, int v, int checkDistance)
{

  if (u - checkDistance < 0 || u + checkDistance >= 250 ||
      v - checkDistance < 0 || v + checkDistance >= 768) {
    return true;
  }
  return false;
}

bool PXDDQMEfficiencyModule::isDeadPixelClose(int u, int v, int checkDistance, const VxdID& moduleID)
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
