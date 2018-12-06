/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/PXDEfficiencyModule.h>
#include "TMatrixDSym.h"
#include <tracking/dataobjects/ROIid.h>
#include <pxd/dataobjects/PXDRawHit.h>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDEfficiencyModule::PXDEfficiencyModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  // Set module properties
  setDescription("writes data to a root file which can be used for the estimation of the PXD efficiency");

  // Parameter definitions
  addParam("pxdclustersname", m_pxdclustersname, "name of StoreArray with PXD cluster", std::string(""));
  addParam("pxddigitsname", m_pxddigitsname, "name of StoreArray with PXD digits", std::string(""));
  addParam("eventmetadataname", m_eventmetadataname, "name of StoreObjPtr to event meta data", std::string(""));
  addParam("tracksname", m_tracksname, "name of StoreArray with RecoTracks", std::string(""));

  addParam("distCut", m_distcut, "max distance in [cm] for cluster/digit to be counted to a track", double(0.0500));
  addParam("otherLayerDistCut", m_otherdistcut,
           "max distance in [cm] for cluster/digit on the other layer to be associated to a track", double(0.0500));

  addParam("useAlignment", m_useAlignment, "if true the alignment will be used", bool(false));
  addParam("writeTree", m_writeTree, "if true a tree with useful info will be filled", bool(false));

  addParam("ROIsName", m_ROIsName, "name of the list of HLT ROIs, if available in output", std::string(""));
}


PXDEfficiencyModule::~PXDEfficiencyModule()
{
}

void PXDEfficiencyModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM;

  //register the required arrays
  m_pxdclusters.isRequired(m_pxdclustersname);
  m_pxddigits.isRequired(m_pxddigitsname);
  storeEventMetaData.isRequired(m_eventmetadataname);
}


void PXDEfficiencyModule::event()
{
  //Set dummy values in all maps
  dummyAllMaps();



  StoreArray<PXDRawHit> RawHits("PXDRawHits");
  for (int ihit = 0; ihit < RawHits.getEntries(); ihit++) {
    VxdID hitID = RawHits[ihit]->getSensorID();
    m_hit_count[hitID]++;
    unsigned short frameNr = RawHits[ihit]->getFrameNr();
    m_h_frame_nr[hitID]->Fill(frameNr);
    if (frameNr > 1) {
      m_h_broken_frame_pos[hitID]->Fill(RawHits[ihit]->getColumn(), RawHits[ihit]->getRow());
    }
  }


  StoreArray<RecoTrack> tracks(m_tracksname);

  B2DEBUG(1, "Number of clusters found: " << m_pxdclusters.getEntries());
  B2DEBUG(1, "Number of digits found: " << m_pxddigits.getEntries());
  B2DEBUG(1, "Number of tracks found: " << tracks.getEntries());

  //hard cut on the number of tracks as more tracks will complicate things
  if (tracks.getEntries() != 1) return;

  if (!tracks[0]->wasFitSuccessful()) return;

  //not sure at which position that is but the correct momentum is not really needed
  genfit::MeasuredStateOnPlane trackstate;
  const genfit::FitStatus* fitstatus = NULL;
  try {//RecoTrack now gives a B2FATAL, so with the check above this might be superfluous now
    fitstatus = tracks[0]->getTrackFitStatus(); //(const AbsTrackRep* rep = NULL)
    trackstate = tracks[0]->getMeasuredStateOnPlaneFromFirstHit();

    //Function getMeasuredStateOnPlaneFromHit has become private, but with argument 0 this does the same
    //Argument was false to take unbiased, but due to wrong ordering in the genfit::Track function it actually meant taking sensorID 0, and it stayed biased. Recreating this here...
  } catch (...) {
    return;
  }

  //loop over all PXD sensors to get the intersections
  std::map<VxdID, TVector3> intersecs;
  std::map<VxdID, TVector3> best_digipos;
  std::map<VxdID, int> best_digiindex;
  std::map<VxdID, TVector3> best_cluspos;
  std::map<VxdID, int> best_clusindex;
  std::map<VxdID, bool> isgoodintersec;
  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();

  bool foundL1 = false;
  bool foundL2 = false;

  //TODO Need to change this for multiple tracks
  //WARNING: if there are multiple intersections on one layer (which should not be the case) only the last one will be considered!
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);

    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    int layer = aVxdID.getLayerNumber();

    bool isgood = false;
    double sigu(-9999);
    double sigv(-9999);
    TVector3 intersec_buff = getTrackInterSec(info, tracks[0], isgood, sigu, sigv);

    if (isgood) {
      m_u_fit[aVxdID] = intersec_buff.X();
      m_v_fit[aVxdID] = intersec_buff.Y();

      m_ucell_fit[aVxdID] = info.getUCellID(m_u_fit[aVxdID]);
      m_vcell_fit[aVxdID] = info.getVCellID(m_v_fit[aVxdID]);

      m_sigma_u_fit[aVxdID] = sigu;
      m_sigma_v_fit[aVxdID] = sigv;

      int bestdigit = findClosestDigit(aVxdID, intersec_buff);
      int bestcluster = findClosestCluster(aVxdID, intersec_buff);

      if (bestdigit >= 0) {
        m_u_digi[aVxdID] = info.getUCellPosition(m_pxddigits[bestdigit]->getUCellID());
        m_v_digi[aVxdID] = info.getVCellPosition(m_pxddigits[bestdigit]->getVCellID());
        m_ucell_digi[aVxdID] = m_pxddigits[bestdigit]->getUCellID();
        m_vcell_digi[aVxdID] = m_pxddigits[bestdigit]->getVCellID();
      }
      if (bestcluster >= 0) {

        m_u_clus[aVxdID] = m_pxdclusters[bestcluster]->getU();
        m_v_clus[aVxdID] = m_pxdclusters[bestcluster]->getV();
        m_ucell_clus[aVxdID] = info.getUCellID(m_u_clus[aVxdID]);
        m_vcell_clus[aVxdID] = info.getVCellID(m_v_clus[aVxdID]);

        // Get cluster sizes and charge of the best cluster candidate

        m_clus_charge[aVxdID] = m_pxdclusters[bestcluster]->getCharge();
        m_clus_seedcharge[aVxdID] = m_pxdclusters[bestcluster]->getSeedCharge();

        m_clus_size[aVxdID] = m_pxdclusters[bestcluster]->getSize();
        m_clus_usize[aVxdID] = m_pxdclusters[bestcluster]->getUSize();
        m_clus_vsize[aVxdID] = m_pxdclusters[bestcluster]->getVSize();
      }
    }

    //sanity checks
    if (layer == 1 && isgood) foundL1 = true;
    if (layer == 2 && isgood) foundL2 = true;

  }//loop over all sensors

  //require that two layers have an intersection!
  if (!foundL1 || !foundL2) return;



  //ROI info

  //This does not always exist (not on the root files from prague)
  StoreArray<ROIid> ROIs(m_ROIsName);

  //WARNING: Only the last ROI for each sensor is saved! Number of ROIs is correct, so use it to cut on events with multiple ROIs on a sensor if this is a problem
  if (ROIs.getEntries() > 0) {
    for (auto& roit : ROIs) {
      VxdID thisSensorROI = roit.getSensorID();

      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(thisSensorROI);

      m_roi_number_of[thisSensorROI]++;
      m_roi_minU[thisSensorROI] = roit.getMinUid();
      m_roi_minV[thisSensorROI] = roit.getMinVid();
      m_roi_maxU[thisSensorROI] = roit.getMaxUid();
      m_roi_maxV[thisSensorROI] = roit.getMaxVid();
      m_roi_widthU[thisSensorROI] = roit.getMaxUid() - roit.getMinUid();
      m_roi_widthV[thisSensorROI] = roit.getMaxVid() - roit.getMinVid();
      m_roi_centerU[thisSensorROI] = (roit.getMaxUid() + roit.getMinUid()) / 2;
      m_roi_centerV[thisSensorROI] = (roit.getMaxVid() + roit.getMinVid()) / 2;
      m_roi_area[thisSensorROI] = (roit.getMaxUid() - roit.getMinUid()) * (roit.getMaxVid() - roit.getMinVid());

      bool fit_inside = false;
      if (m_ucell_fit[thisSensorROI] < roit.getMaxUid()
          && m_ucell_fit[thisSensorROI] > roit.getMinUid()
          && m_vcell_fit[thisSensorROI] < roit.getMaxVid()
          && m_vcell_fit[thisSensorROI] > roit.getMinVid()) {
        fit_inside = true;
      }
      m_roi_fit_inside[thisSensorROI] = fit_inside;

      bool clus_inside = false;
      if (info.getUCellID(m_u_clus[thisSensorROI]) < roit.getMaxUid()
          && info.getUCellID(m_u_clus[thisSensorROI]) > roit.getMinUid()
          && info.getVCellID(m_v_clus[thisSensorROI]) < roit.getMaxVid()
          && info.getVCellID(m_v_clus[thisSensorROI]) > roit.getMinVid()) {
        clus_inside = true;
      }
      bool digi_inside = false;
      if (info.getUCellID(m_u_digi[thisSensorROI]) < roit.getMaxUid()
          && info.getUCellID(m_u_digi[thisSensorROI]) > roit.getMinUid()
          && info.getVCellID(m_v_digi[thisSensorROI]) < roit.getMaxVid()
          && info.getVCellID(m_v_digi[thisSensorROI]) > roit.getMinVid()) {
        digi_inside = true;
      }

      m_roi_digi_inside[thisSensorROI] = digi_inside;
      m_roi_clus_inside[thisSensorROI] = clus_inside;

      m_roi_u_residual[thisSensorROI] = m_u_fit[thisSensorROI] - info.getUCellPosition(m_roi_centerU[thisSensorROI]);
      m_roi_v_residual[thisSensorROI] = m_v_fit[thisSensorROI] - info.getVCellPosition(m_roi_centerV[thisSensorROI]);
      m_roi_ucell_residual[thisSensorROI] = m_ucell_fit[thisSensorROI] - m_roi_centerU[thisSensorROI];
      m_roi_vcell_residual[thisSensorROI] = m_vcell_fit[thisSensorROI] - m_roi_centerV[thisSensorROI];
    }
  }


  //fill the needed track info
  //TODO These vars should be containers when more than 1 track
  m_event = int(storeEventMetaData->getEvent());
  m_run = int(storeEventMetaData->getRun());
  m_subrun = int(storeEventMetaData->getSubrun());
  m_fit_pValue = fitstatus->getPVal();
  m_fit_ndf = fitstatus->getNdf();
  m_fit_charge = trackstate.getCharge();
  TVector3 mom = trackstate.getMom();
  m_fit_mom = mom.Mag();
  m_fit_theta = mom.Theta();
  m_fit_phi = mom.Phi();
  m_fit_x = mom.X();
  m_fit_y = mom.Y();
  m_fit_z = mom.Z();






  std::map<int, bool> DigiMatched;
  std::map<int, bool> ClusMatched;
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);

    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    int layer = aVxdID.getLayerNumber();


    TVector3 dist_digit(m_u_fit[aVxdID] - m_u_digi[aVxdID], m_v_fit[aVxdID] - m_v_digi[aVxdID], 0);
    TVector3 dist_clus(m_u_fit[aVxdID] - m_u_clus[aVxdID], m_v_fit[aVxdID] - m_v_clus[aVxdID], 0);

    int iu = m_vxdGeometry.getSensorInfo(aVxdID).getUCellID(m_u_fit[aVxdID]);
    int iv = m_vxdGeometry.getSensorInfo(aVxdID).getVCellID(m_v_fit[aVxdID]);


    //fill ROI like so no cut on having hit on the other layer
    {
      m_h_tracksROI[aVxdID]->Fill(iu, iv);
      if (dist_clus.Mag() <= m_distcut)  {
        m_h_clusterROI[aVxdID]->Fill(iu, iv);
        m_cluster_matched[aVxdID] = 0;

      }
      if (dist_digit.Mag() <= m_distcut) {
        m_digit_matched[aVxdID] = 0;
        m_h_digitsROI[aVxdID]->Fill(iu, iv);

        //For now only match digits, this should work in almost all cases and is much easier
        for (int ihit = 0; ihit < RawHits.getEntries(); ihit++) {
          if (RawHits[ihit]->getSensorID() == aVxdID && RawHits[ihit]->getColumn() == m_ucell_digi[aVxdID]
              && RawHits[ihit]->getRow() == m_vcell_digi[aVxdID]) {
            m_matched_frame[aVxdID] = RawHits[ihit]->getFrameNr();
          }
        }
      }
    }

    if (dist_digit.Mag() <= m_otherdistcut) DigiMatched[layer] = true;
    if (dist_clus.Mag() <= m_otherdistcut) ClusMatched[layer] = true;
  }


  //Not a good solution, but easiest to reproduce old var, works only for 2 pxd layers, but other number of layers never happens
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);

    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    int layer = aVxdID.getLayerNumber();

    if ((layer == 1 && DigiMatched[2]) || (layer == 2 && DigiMatched[1])) m_otherpxd_digit_matched[aVxdID] = 0;
    if ((layer == 1 && ClusMatched[2]) || (layer == 2 && ClusMatched[1])) m_otherpxd_cluster_matched[aVxdID] = 0;

  }

  //Old histos, don't work with arbitrary geometry but keep as reference for re-implementation later
  /*
  //fill histograms to calculate efficiencies
  // WARNING: this uses hardcoded sensor ids!
  VxdID L1id(1, 1, 2);
  VxdID L2id(2, 1, 2);

  TVector3 dist_digit_L1(m_u_fit[L1id] - m_u_digi[L1id], m_v_fit[L1id] - m_v_digi[L1id], 0);
  TVector3 dist_clus_L1(m_u_fit[L1id] - m_u_clus[L1id], m_v_fit[L1id] - m_v_clus[L1id], 0);
  TVector3 dist_digit_L2(m_u_fit[L2id] - m_u_digi[L2id], m_v_fit[L2id] - m_v_digi[L2id], 0);
  TVector3 dist_clus_L2(m_u_fit[L2id] - m_u_clus[L2id], m_v_fit[L2id] - m_v_clus[L2id], 0);

  //has digit on L1
  if (dist_digit_L1.Mag() <= m_distcut) {
    int iu = m_vxdGeometry.getSensorInfo(L2id).getUCellID(m_u_fit[L2id]);
    int iv = m_vxdGeometry.getSensorInfo(L2id).getVCellID(m_v_fit[L2id]);
    m_h_tracksdigit[L2id]->Fill(iu, iv);
    if (dist_digit_L2.Mag() <= m_distcut) m_h_digits[L2id]->Fill(iu, iv);
  }
  //has digit on L2
  if (dist_digit_L2.Mag() <= m_distcut) {
    int iu = m_vxdGeometry.getSensorInfo(L1id).getUCellID(m_u_fit[L1id]);
    int iv = m_vxdGeometry.getSensorInfo(L1id).getVCellID(m_v_fit[L1id]);
    m_h_tracksdigit[L1id]->Fill(iu, iv);
    if (dist_digit_L1.Mag() <= m_distcut) m_h_digits[L1id]->Fill(iu, iv);
  }
  //has cluster on L1
  if (dist_clus_L1.Mag() <= m_distcut) {
    int iu = m_vxdGeometry.getSensorInfo(L2id).getUCellID(m_u_fit[L2id]);
    int iv = m_vxdGeometry.getSensorInfo(L2id).getVCellID(m_v_fit[L2id]);

    m_h_trackscluster[L2id]->Fill(iu, iv);
    if (dist_clus_L2.Mag() <= m_distcut) m_h_cluster[L2id]->Fill(iu, iv);
  }
  //has cluster on L2
  if (dist_clus_L2.Mag() <= m_distcut) {
    int iu = m_vxdGeometry.getSensorInfo(L1id).getUCellID(m_u_fit[L1id]);
    int iv = m_vxdGeometry.getSensorInfo(L1id).getVCellID(m_v_fit[L1id]);
    m_h_trackscluster[L1id]->Fill(iu, iv);
    if (dist_clus_L1.Mag() <= m_distcut) m_h_cluster[L1id]->Fill(iu, iv);
  }
  */

  if (m_writeTree) m_tree->Fill();
}


TVector3 PXDEfficiencyModule::getTrackInterSec(VXD::SensorInfoBase& svdSensorInfo, const RecoTrack* aTrack, bool& isgood,
                                               double& du, double& dv)
{
  //will be set true if the intersect was found
  isgood = false;

  TVector3 intersec(99999999, 9999999, 0); //point outside the sensor

  genfit::MeasuredStateOnPlane gfTrackState = aTrack->getMeasuredStateOnPlaneFromFirstHit();

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


void PXDEfficiencyModule::defineHisto()
{
  // deleting all histograms and trees should be handled by the HistoModule !?
  // Correct me if I am wrong because otherwise this is a nice Memory leak!
  m_tree = new TTree("PXDEffiTree", "Tree for calculating pxd efficiencies");

  m_tree->Branch("event", &m_event, "event/I");
  m_tree->Branch("run", &m_run, "run/I");
  m_tree->Branch("subrun", &m_subrun, "subrun/I");
  m_tree->Branch("fit_pValue", &m_fit_pValue, "fit_pValue/D");
  m_tree->Branch("fit_mom", &m_fit_mom, "fit_mom/D");
  m_tree->Branch("fit_charge", &m_fit_charge, "fit_charge/D");
  m_tree->Branch("fit_theta", &m_fit_theta,  "fit_theta/D");
  m_tree->Branch("fit_phi", &m_fit_phi, "fit_phi/D");
  m_tree->Branch("fit_x", &m_fit_x, "fit_x/D");
  m_tree->Branch("fit_y", &m_fit_y, "fit_y/D");
  m_tree->Branch("fit_z", &m_fit_z, "fit_z/D");
  m_tree->Branch("fit_chi2",  &m_fit_chi2, "fit_chi2/D");
  m_tree->Branch("fit_ndf", &m_fit_ndf, "fit_ndf/I");

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();

  dummyAllMaps();

  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);

    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    TString buff = (std::string)avxdid;
    buff.ReplaceAll(".", "_");

    m_tree->Branch("u_clus_" + buff, &(m_u_clus[avxdid]), "u_clus_" + buff + "/D");
    m_tree->Branch("v_clus_" + buff, &(m_v_clus[avxdid]), "v_clus_" + buff + "/D");
    m_tree->Branch("ucell_clus_" + buff, &(m_ucell_clus[avxdid]), "ucell_clus_" + buff + "/I");
    m_tree->Branch("vcell_clus_" + buff, &(m_vcell_clus[avxdid]), "vcell_clus_" + buff + "/I");
    m_tree->Branch("u_digi_" + buff, &(m_u_digi[avxdid]), "u_digi_" + buff + "/D");
    m_tree->Branch("v_digi_" + buff, &(m_v_digi[avxdid]), "v_digi_" + buff + "/D");

    m_tree->Branch("clus_charge_" + buff, &(m_clus_charge[avxdid]), "clus_charge_" + buff + "/D");
    m_tree->Branch("clus_seedcharge_" + buff, &(m_clus_seedcharge[avxdid]), "clus_seedcharge_" + buff + "/D");
    m_tree->Branch("clus_size_" + buff, &(m_clus_size[avxdid]), "clus_size__" + buff + "/I");
    m_tree->Branch("clus_usize_" + buff, &(m_clus_usize[avxdid]), "clus_usize__" + buff + "/I");
    m_tree->Branch("clus_vsize_" + buff, &(m_clus_vsize[avxdid]), "clus_vsize__" + buff + "/I");

    m_tree->Branch("u_fit_" + buff, &(m_u_fit[avxdid]), "u_fit_" + buff + "/D");
    m_tree->Branch("v_fit_" + buff, &(m_v_fit[avxdid]), "v_fit_" + buff + "/D");
    m_tree->Branch("ucell_fit_" + buff, &(m_ucell_fit[avxdid]), "ucell_fit_" + buff + "/I");
    m_tree->Branch("vcell_fit_" + buff, &(m_vcell_fit[avxdid]), "vcell_fit_" + buff + "/I");
    m_tree->Branch("sigma_u_fit_" + buff, &(m_sigma_u_fit[avxdid]), "sigma_u_fit_" + buff + "/D");
    m_tree->Branch("sigma_v_fit_" + buff, &(m_sigma_v_fit[avxdid]), "sigma_v_fit_" + buff + "/D");
    m_tree->Branch("digit_matched_" + buff, &(m_digit_matched[avxdid]), "digit_matched_" + buff + "/I");
    m_tree->Branch("cluster_matched_" + buff, &(m_cluster_matched[avxdid]), "cluster_matched_" + buff + "/I");
    m_tree->Branch("otherpxd_digit_matched_" + buff, &(m_otherpxd_digit_matched[avxdid]), "otherpxd_digit_matched_" + buff + "/I");
    m_tree->Branch("otherpxd_cluster_matched_" + buff, &(m_otherpxd_cluster_matched[avxdid]),
                   "otherpxd_cluster_matched_" + buff + "/I");

    //New Entries to study effect of ROI size and similar
    m_tree->Branch("roi_number_of_" + buff, &(m_roi_number_of[avxdid]), "roi_number_of_" + buff + "/I");
    m_tree->Branch("roi_min_u_" + buff, &(m_roi_minU[avxdid]), "roi_min_u_" + buff + "/I");
    m_tree->Branch("roi_min_v_" + buff, &(m_roi_minV[avxdid]), "roi_min_v_" + buff + "/I");
    m_tree->Branch("roi_max_u_" + buff, &(m_roi_maxU[avxdid]), "roi_max_u_" + buff + "/I");
    m_tree->Branch("roi_max_v_" + buff, &(m_roi_maxV[avxdid]), "roi_max_v_" + buff + "/I");
    m_tree->Branch("roi_width_u_" + buff, &(m_roi_widthU[avxdid]), "roi_width_u_" + buff + "/I");
    m_tree->Branch("roi_width_v_" + buff, &(m_roi_widthV[avxdid]), "roi_width_v_" + buff + "/I");
    m_tree->Branch("roi_center_u_" + buff, &(m_roi_centerU[avxdid]), "roi_center_u_" + buff + "/I");
    m_tree->Branch("roi_center_v_" + buff, &(m_roi_centerV[avxdid]), "roi_center_v_" + buff + "/I");
    m_tree->Branch("roi_area_" + buff, &(m_roi_area[avxdid]), "roi_area_" + buff + "/I");
    m_tree->Branch("roi_fit_inside_" + buff, &(m_roi_fit_inside[avxdid]), "roi_fit_inside_" + buff + "/I");
    m_tree->Branch("roi_clus_inside_" + buff, &(m_roi_clus_inside[avxdid]), "roi_clus_inside_" + buff + "/I");
    m_tree->Branch("roi_digi_inside_" + buff, &(m_roi_digi_inside[avxdid]), "roi_digi_inside_" + buff + "/I");
    m_tree->Branch("roi_u_residual_" + buff, &(m_roi_u_residual[avxdid]), "roi_u_residual_" + buff + "/D");
    m_tree->Branch("roi_v_residual_" + buff, &(m_roi_v_residual[avxdid]), "roi_v_residual_" + buff + "/D");
    m_tree->Branch("roi_ucell_residual_" + buff, &(m_roi_ucell_residual[avxdid]), "roi_ucell_residual_" + buff + "/I");
    m_tree->Branch("roi_vcell_residual_" + buff, &(m_roi_vcell_residual[avxdid]), "roi_vcell_residual_" + buff + "/I");
    m_tree->Branch("fit_matched_frame_" + buff, &(m_matched_frame[avxdid]), "fit_matched_frame_" + buff + "/I");
    m_tree->Branch("number_of_hits_" + buff, &(m_hit_count[avxdid]), "number_of_hits_" + buff + "/I");

    int nu = info.getUCells();
    int nv = info.getVCells();
    //double widthu = info.getUSize();
    //double widthv = info.getVSize();

    m_h_digits[avxdid] = new TH2D("digits_" + buff, "hits for sensor " + buff,
                                  nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
    m_h_cluster[avxdid] = new TH2D("clusters_" + buff, "hits for sensor " + buff,
                                   nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
    m_h_tracksdigit[avxdid] = new TH2D("normdigits_" + buff, "tracks for sensor " + buff,
                                       nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
    m_h_trackscluster[avxdid] = new TH2D("normcluster_" + buff, "tracks for sensor " + buff,
                                         nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);

    //ROI like
    m_h_tracksROI[avxdid] = new TH2D("ROInorm_" + buff, "tracks for sensor " + buff,
                                     nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
    m_h_digitsROI[avxdid] = new TH2D("digitsROI_" + buff, "hits for sensor " + buff,
                                     nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
    m_h_clusterROI[avxdid] = new TH2D("clustersROI_" + buff, "hits for sensor " + buff,
                                      nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);

    m_h_frame_nr[avxdid] = new TH1D("frameNr_" + buff, "Frame Nr. distribution on sensor " + buff,
                                    64, -0.5, 63.5);
    m_h_broken_frame_pos[avxdid] = new TH2D("frameNr_broken_pos_" + buff, "Position of Frame Nr>1 on sensor " + buff,
                                            nu + 1, -0.5, nu + 0.5, nv + 1, -0.5, nv + 0.5);
  }
}

int
PXDEfficiencyModule::findClosestDigit(VxdID& avxdid, TVector3 intersection)
{
  int closest = -1;
  double mindist = 999999999999; //definetly outside of the sensor

  VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);

  //loop the digits
  for (int idigi = 0; idigi < m_pxddigits.getEntries(); idigi++) {
    //Do not consider as different if only segment differs!
    VxdID digitID = m_pxddigits[idigi]->getSensorID();
    if (avxdid.getLayerNumber() != digitID.getLayerNumber()) {
      if (avxdid.getLadderNumber() != digitID.getLadderNumber()) {
        if (avxdid.getSensorNumber() != digitID.getSensorNumber()) {
          continue;
        }
      }
    }
    //only digit on the correct sensor and direction should survive

    double u = info.getUCellPosition(m_pxddigits[idigi]->getUCellID());
    double v = info.getVCellPosition(m_pxddigits[idigi]->getVCellID());
    TVector3 current(u, v, 0);

    //2D dist sqared
    double dist = (intersection - current).Mag();
    if (dist < mindist) {
      closest = idigi;
      mindist = dist;
    }
  }

  return closest;

}

int
PXDEfficiencyModule::findClosestCluster(VxdID& avxdid, TVector3 intersection)
{
  int closest = -1;
  double mindist = 999999999999; //definetly outside of the sensor

  VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);

  //loop the clusters
  for (int iclus = 0; iclus < m_pxdclusters.getEntries(); iclus++) {
    //Do not consider as different if only segment differs!
    VxdID clusterID = m_pxdclusters[iclus]->getSensorID();
    if (avxdid.getLayerNumber() != clusterID.getLayerNumber()) {
      if (avxdid.getLadderNumber() != clusterID.getLadderNumber()) {
        if (avxdid.getSensorNumber() != clusterID.getSensorNumber()) {
          continue;
        }
      }
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


void PXDEfficiencyModule::dummyAllMaps(void)
{

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();

  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);

    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    m_sigma_u_fit[aVxdID] = -9999;
    m_sigma_v_fit[aVxdID] = -9999;

    m_u_fit[aVxdID] = 999999;//take positive default so that the difference is not zero!
    m_v_fit[aVxdID] = 999999;//take positive default so that the difference is not zero!
    m_u_clus[aVxdID] = -9999;
    m_v_clus[aVxdID] = -9999;
    m_ucell_clus[aVxdID] = -9999;
    m_vcell_clus[aVxdID] = -9999;
    m_u_digi[aVxdID] = -9999;
    m_v_digi[aVxdID] = -9999;
    m_ucell_digi[aVxdID] = -9999;
    m_vcell_digi[aVxdID] = -9999;
    m_sigma_u_fit[aVxdID] = -9999;
    m_sigma_v_fit[aVxdID] = -9999;

    m_ucell_fit[aVxdID] = 999999;
    m_vcell_fit[aVxdID] = 999999;
    m_digit_matched[aVxdID] = -1;
    m_cluster_matched[aVxdID] = -1;
    m_otherpxd_digit_matched[aVxdID] = -1;
    m_otherpxd_cluster_matched[aVxdID] = -1;

    m_clus_charge[aVxdID] = -9999;
    m_clus_seedcharge[aVxdID] = -9999;
    m_clus_size[aVxdID] = -9999;
    m_clus_usize[aVxdID] = -9999;
    m_clus_vsize[aVxdID] = -9999;

    m_roi_number_of[aVxdID] = 0;
    m_roi_minU[aVxdID] = -99999;
    m_roi_minV[aVxdID] = -99999;
    m_roi_maxU[aVxdID] = -99999;
    m_roi_maxV[aVxdID] = -99999;
    m_roi_widthU[aVxdID] = -99999;
    m_roi_widthV[aVxdID] = -99999;
    m_roi_centerU[aVxdID] = -99999;
    m_roi_centerV[aVxdID] = -99999;
    m_roi_area[aVxdID] = -99999;
    m_roi_fit_inside[aVxdID] = false;
    m_roi_clus_inside[aVxdID] = false;
    m_roi_digi_inside[aVxdID] = false;
    m_roi_u_residual[aVxdID] = -99999;
    m_roi_v_residual[aVxdID] = -99999;
    m_roi_ucell_residual[aVxdID] = -99999;
    m_roi_vcell_residual[aVxdID] = -99999;
    m_matched_frame[aVxdID] = -99999;
    //Count these upwards
    m_hit_count[aVxdID] = 0;
  }
}
