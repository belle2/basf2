/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas, Uwe                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDQMEfficiencyNtupleModule.h>
#include <tracking/dataobjects/ROIid.h>

#include <pxd/reconstruction/PXDPixelMasker.h>

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
  addParam("pxdClustersName", m_pxdClustersName, "name of StoreArray with PXD cluster", std::string(""));
  addParam("tracksName", m_tracksName, "name of StoreArray with RecoTracks", std::string(""));
  addParam("ROIsName", m_ROIsName, "name of the list of HLT ROIs, if available in output", std::string(""));

  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", true);

  addParam("pCut", m_pcut, "Set a cut on the p-value ", double(0));

  addParam("minSVDHits", m_minSVDHits, "Number of SVD hits required in a track to be considered", 0u);

  addParam("momCut", m_momCut, "Set a cut on the track momentum", double(0));

  addParam("pTCut", m_pTCut, "Set a cut on the track pT", double(0));

  addParam("maskedDistance", m_maskedDistance, "Distance inside which no masked pixel or sensor border is allowed", int(10));
}


void PXDDQMEfficiencyNtupleModule::terminate()
{
  auto dir = gDirectory;
  if (m_tuple) {
    m_file->cd();
    m_tuple->Write();
    delete m_tuple;
  }
  if (m_file) {
    m_file->Write();
    m_file->Close();
    delete m_file;
  }
  dir->cd();
}


void PXDDQMEfficiencyNtupleModule::initialize()
{
  m_tuple = new TNtuple("effcontrol", "effcontrol", "vxdid:u:v:p:pt:distu:distv:sigu:sigv:dist:inroi:clborder:cldead:matched");
  m_file = new TFile("test.root", "recreate");

  //register the required arrays
  //Register as optional so validation for cases where they are not available still succeeds, but module will not do any meaningful work without them
  m_pxdclusters.isOptional(m_pxdClustersName);
  m_tracks.isOptional(m_tracksName);
  m_ROIs.isOptional(m_ROIsName);
}


void PXDDQMEfficiencyNtupleModule::event()
{
  if (!m_pxdclusters.isValid()) {
    B2INFO("PXDClusters array is missing, no efficiencies");
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
    if (trackstate.getMom().Pt() < m_pTCut) continue;

    //loop over all PXD sensors to get the intersections
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& aVxdID : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);
      if (info.getType() != VXD::SensorInfoBase::PXD) continue;
      //Search for intersections of the track with all PXD layers
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

        int bestcluster = findClosestCluster(aVxdID, intersec_buff);
        double du_clus = 0;
        double dv_clus = 0;
        double d_clus = 0;
        bool matched = false;
        if (bestcluster >= 0) {
          double u_clus = m_pxdclusters[bestcluster]->getU();
          double v_clus = m_pxdclusters[bestcluster]->getV();

          //is the closest cluster close enough to the track to count as measured?
          TVector3 dist_clus(u_fit - u_clus, v_fit - v_clus, 0);
          du_clus = u_fit - u_clus;
          dv_clus = v_fit - v_clus;
          d_clus = dist_clus.Mag();
          matched = true;
        }
        m_tuple->Fill((int)aVxdID, u_fit, v_fit, trackstate.getMom().Mag(), trackstate.getMom().Pt(), du_clus, dv_clus, sigu, sigv,
                      d_clus, fitInsideROI, closeToBoarder, closeToDead, matched);

      }
    }
  }
}



TVector3 PXDDQMEfficiencyNtupleModule::getTrackInterSec(VXD::SensorInfoBase& pxdSensorInfo, const RecoTrack& aTrack, bool& isgood,
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
    sensorPlane->setO(pxdSensorInfo.pointToGlobal(zeroVec, m_useAlignment));
    sensorPlane->setUV(pxdSensorInfo.vectorToGlobal(uVec, m_useAlignment), pxdSensorInfo.vectorToGlobal(vVec, m_useAlignment));

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
  intersec = pxdSensorInfo.pointToLocal(gfTrackState.getPos(), m_useAlignment);

  //try to get the momentum
  B2DEBUG(1, "Fitted momentum on the plane p = " << gfTrackState.getMom().Mag());

  // no tolerance currently! Maybe one should be added!
  double tolerance = 0.0;
  bool inside = pxdSensorInfo.inside(intersec.X(), intersec.Y(), tolerance, tolerance);

  // get intersection point in local coordinates with covariance matrix
  TMatrixDSym covMatrix = gfTrackState.getCov(); // 5D with elements q/p,u',v',u,v in plane system

  // get ROI by covariance matrix and local intersection point
  du = std::sqrt(covMatrix(3, 3));
  dv = std::sqrt(covMatrix(4, 4));

  if (inside) isgood = true;

  return intersec;
}


int
PXDDQMEfficiencyNtupleModule::findClosestCluster(VxdID& avxdid, TVector3 intersection)
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

bool PXDDQMEfficiencyNtupleModule::isDeadPixelClose(int u, int v, int checkDistance, VxdID& moduleID)
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
