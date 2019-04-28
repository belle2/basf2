/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDQM/SVDDQMEfficiencyModule.h>
#include "TDirectory.h"
#include "TMatrixDSym.h"
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMEfficiencyModule::SVDDQMEfficiencyModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("Create basic histograms for SVD efficiency");

  // What exactly is needed for this to be true?
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("svdClustersName", m_svdClustersName, "name of StoreArray with SVD cluster", std::string(""));
  addParam("tracksName", m_tracksName, "name of StoreArray with RecoTracks", std::string(""));

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("svdeff"));

  addParam("binsU", m_u_bins, "histogram bins in u direction", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction", int(6));

  addParam("distCut", m_distcut, "max distance in [cm] for cluster to be counted to a track", double(0.0500));

  addParam("pCut", m_pcut, "Set a cut on the p-value ", double(0));

  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", bool(true));

  addParam("maskStrips", m_maskStrips, "Do not consider tracks going through known masked strips for the efficiency",
           bool(false));

  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered", 0u);
  addParam("minPXDHits", m_minPXDHits, "Number of PXD hits required in a track to be considered", 0u);

  addParam("momCut", m_momCut, "Set a cut on the track momentum", double(0));

  addParam("cutBorders", m_cutBorders, "Do not use tracks near the borders of the sensor", bool(true));

  addParam("maskedDistance", m_maskedDistance, "Distance inside which no masked strip or sensor border is allowed", int(10));
}


void SVDDQMEfficiencyModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM;

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_svdclusters.isOptional(m_svdClustersName);
  m_tracks.isOptional(m_tracksName);
}


void SVDDQMEfficiencyModule::event()
{
  if (!m_svdclusters.isValid()) {
    B2INFO("SVDClusters array is missing, no efficiencies");
    return;
  }
  if (!m_tracks.isValid()) {
    B2INFO("RecoTrack array is missing, no efficiencies");
    return;
  }

  for (auto& a_track : m_tracks) {

    //If fit failed assume position pointed to is useless anyway
    if (!a_track.wasFitSuccessful()) continue;

    if (a_track.getNumberOfSVDHits() < m_minSVDHits) continue;

    const genfit::FitStatus* fitstatus = a_track.getTrackFitStatus();
    if (fitstatus->getPVal() < m_pcut) continue;

    genfit::MeasuredStateOnPlane trackstate;
    trackstate = a_track.getMeasuredStateOnPlaneFromFirstHit();
    if (trackstate.getMom().Mag() < m_momCut) continue;

    //loop over all SVD sensors to get the intersections
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& aVxdID : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);
      if (info.getType() != VXD::SensorInfoBase::SVD) continue;

      //Search for intersections of the track with all SVD layers
      //Traditional (aka the person before did it like this) method
      //If there is a way to find out sensors crossed by a track directly, that would most likely be faster

      bool isgood = false;
      //true = track intersects current sensor
      double sigu(-9999);
      double sigv(-9999);
      TVector3 intersec_buff = getTrackInterSec(info, a_track, isgood, sigu, sigv);

      if (!isgood) {
        continue;//track does not go through this sensor-> nothing to measure anyway
      } else {

        double u_fit = intersec_buff.X();
        double v_fit = intersec_buff.Y();

        int ucell_fit = info.getUCellID(intersec_buff.X());
        int vcell_fit = info.getVCellID(intersec_buff.Y());

        /*        if (m_cutBorders && isCloseToBorder(ucell_fit, vcell_fit, m_maskedDistance)) {
                continue;
              }

              if (m_maskStrips && isMaskedStripClose(ucell_fit, vcell_fit, m_maskedDistance, aVxdID)) {
                continue;
          }*/

        //This track should be on the sensor
        m_h_track_hits[aVxdID]->Fill(ucell_fit, vcell_fit);
        m_TrackHits->fill(aVxdID, 0, 1);
        m_TrackHits->fill(aVxdID, 1, 1);
        //Now check if the sensor measured a hit here
        int bestcluster = findClosestCluster(aVxdID, intersec_buff);
        TVector3 dist_clus(0, 0, 0);
        if (bestcluster >= 0) {
          if (m_svdclusters[bestcluster]->isUCluster()) {
            double u_clus = m_svdclusters[bestcluster]->getPosition();
            dist_clus.SetXYZ(u_fit - u_clus, 0, 0);
          } else {
            double v_clus = m_svdclusters[bestcluster]->getPosition();
            dist_clus.SetXYZ(0, v_fit - v_clus, 0);
          }

          //is the closest cluster close enough to the track to count as measured?
          if (dist_clus.Mag() <= m_distcut)  {
            m_h_matched_cluster[aVxdID]->Fill(ucell_fit, vcell_fit);
            if (m_svdclusters[bestcluster]->isUCluster()) {
              m_MatchedHits->fill(aVxdID, 1, 1);
            } else
              m_MatchedHits->fill(aVxdID, 0, 1);
          }
        }
      }
    }
  }
}



TVector3 SVDDQMEfficiencyModule::getTrackInterSec(VXD::SensorInfoBase& svdSensorInfo, const RecoTrack& aTrack, bool& isgood,
                                                  double& du, double& dv)
{
  //will be set true if the intersect was found
  isgood = false;

  TVector3 intersec(99999999, 9999999, 0); //point outside the sensor

  genfit::MeasuredStateOnPlane gfTrackState = aTrack.getMeasuredStateOnPlaneFromFirstHit();

  //adopted (aka stolen) from tracking/modules/pxdClusterRescue/PXDClusterRescueROIModule
  try {
    // get sensor plane
    TVector3 zeroVec(0, 0, 0);
    TVector3 uVec(1, 0, 0);
    TVector3 vVec(0, 1, 0);

    genfit::DetPlane* sensorPlane = new genfit::DetPlane();
    sensorPlane->setO(svdSensorInfo.pointToGlobal(zeroVec, m_useAlignment));
    sensorPlane->setUV(svdSensorInfo.vectorToGlobal(uVec, m_useAlignment), svdSensorInfo.vectorToGlobal(vVec, m_useAlignment));

    //boost pointer (will be deleted automatically ?!?!?)
    genfit::SharedPlanePtr sensorPlaneSptr(sensorPlane);

    // do extrapolation
    gfTrackState.extrapolateToPlane(sensorPlaneSptr);
  } catch (genfit::Exception& gfException) {
    B2WARNING("Fitting failed: " << gfException.getExcString());
    isgood = false;
    return intersec;
  } catch (...) {
    B2WARNING("Fitting failed: for some reason");
    isgood = false;
    return intersec;
  }

  //local position
  intersec = svdSensorInfo.pointToLocal(gfTrackState.getPos(), m_useAlignment);

  //try to get the momentum
  B2DEBUG(1, "Fitted momentum on the plane p = " << gfTrackState.getMom().Mag());

  // no tolerance currently! Maybe one should be added!
  double tolerance = 0.0;
  bool inside = svdSensorInfo.inside(intersec.X(), intersec.Y(), tolerance, tolerance);

  // get intersection point in local coordinates with covariance matrix
  TMatrixDSym covMatrix = gfTrackState.getCov(); // 5D with elements q/p,u',v',u,v in plane system

  // get ROI by covariance matrix and local intersection point
  du = std::sqrt(covMatrix(3, 3));
  dv = std::sqrt(covMatrix(4, 4));

  if (inside) isgood = true;

  return intersec;
}


void SVDDQMEfficiencyModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  m_TrackHits = new SVDSummaryPlots("TrackHits@view", "Number of Tracks intercepting the @view/@side Side");
  m_MatchedHits = new SVDSummaryPlots("MatchedHits@view", "Number of Matched Clusters on the @view/@side Side");
  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    //Only interested in SVD sensors

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    int nu = info.getUCells();
    int nv = info.getVCells();

    //nu + 1,nv + 1 Bin numbers when using one bin per pixel
    m_h_track_hits[avxdid] = new TH2D("track_hits_" + buff, "tracks through sensor " + buff,
                                      m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    m_h_matched_cluster[avxdid] = new TH2D("matched_cluster_" + buff, "clusters matched to track intersections " + buff,
                                           m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
  }
  // cd back to root directory
  oldDir->cd();
}


int
SVDDQMEfficiencyModule::findClosestCluster(VxdID& avxdid, TVector3 intersection)
{
  int closest = -1;
  double mindist = 999999999999; //definitely outside of the sensor

  VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);

  //loop the clusters
  for (int iclus = 0; iclus < m_svdclusters.getEntries(); iclus++) {
    //Do not consider as different if only segment differs!
    //As of this writing segment is never filled for clusters, but just to be sure
    VxdID clusterID = m_svdclusters[iclus]->getSensorID();
    if (avxdid.getLayerNumber() != clusterID.getLayerNumber() ||
        avxdid.getLadderNumber() != clusterID.getLadderNumber() ||
        avxdid.getSensorNumber() != clusterID.getSensorNumber()) {
      continue;
    }
    //only cluster on the correct sensor and direction should survive

    double u = 0;
    double v = 0;
    TVector3 current(0, 0, 0);
    if (m_svdclusters[iclus]->isUCluster()) {
      u = m_svdclusters[iclus]->getPosition();
      current.SetXYZ(u, 0, 0);
    } else {
      v = m_svdclusters[iclus]->getPosition();
      current.SetXYZ(0, v, 0);
    }


    //2D dist sqared
    double dist = (intersection - current).Mag();
    if (dist < mindist) {
      closest = iclus;
      mindist = dist;
    }
  }

  return closest;

}

/*bool SVDDQMEfficiencyModule::isCloseToBorder(int u, int v, int checkDistance)
{

  if (u - checkDistance < 0 || u + checkDistance >= 250 ||
      v - checkDistance < 0 || v + checkDistance >= 768) {
    return true;
  }
  return false;
  }*/

/*bool SVDDQMEfficiencyModule::isMaskedStripClose(int u, int v, int checkDistance, VxdID& moduleID)
{
 return false;
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
}*/
