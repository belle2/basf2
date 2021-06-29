/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI, Yun-Tsung Lai, Junhao Yin                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// include
#include <trg/grl/modules/trggrl/TRGGRLMatchModule.h>
#include <trg/grl/dataobjects/TRGGRLMATCH.h>
#include <trg/grl/dataobjects/TRGGRLMATCHKLM.h>
#include <trg/grl/dataobjects/TRGGRLPHOTON.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/klm/dataobjects/KLMTrgSummary.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <trg/grl/dataobjects/TRGGRLShortTrack.h>
#include <trg/grl/dataobjects/TRGGRLInnerTrack.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

//framework aux
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <stdlib.h>


using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGGRLMatch)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGGRLMatchModule::TRGGRLMatchModule() : Module()
{
  // Set module properties
  setDescription("match CDC trigger tracks and ECL trigger clusters");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SimulationMode", m_simulationMode, "TRGGRL simulation switch", 1);
  addParam("FastSimulationMode", m_fastSimulationMode, "TRGGRL fast simulation mode", m_fastSimulationMode);
  addParam("FirmwareSimulationMode", m_firmwareSimulationMode, "TRGGRL firmware simulation mode", m_firmwareSimulationMode);

  addParam("DrMatch", m_dr_threshold, "the threshold of dr between track and cluster if they are matched successfully", 25.);
  addParam("DzMatch", m_dz_threshold, "the threshold of dz between track and cluster if they are matched successfully", 30.);
  addParam("DphidMatch", m_dphi_d_threshold, "the threshold of dphi_d between track and cluster if they are matched successfully", 2);
  addParam("Ephoton", m_e_threshold, "the threshold of cluster energy as a photon", 1.0);
  addParam("KLMMatch", m_dphi_klm_threshold,
           "the threshold of dphi (in degree) between track and KLM sector if they are matched successfully", 32.5);
  addParam("2DtrackCollection", m_2d_tracklist, "the 2d track list used in the match", std::string("TRGCDC2DFinderTracks"));
  addParam("3DtrackCollection", m_3d_tracklist, "the 3d track list used in the match", std::string("TRGCDCNeuroTracks"));
  addParam("TRGECLClusterCollection", m_clusterlist, "the cluster list used in the match", std::string("TRGECLClusters"));
  addParam("KLMTrgrSummary", m_klmtrgsummarylist, "the KLM track list used in the match", std::string("KLMTrgSummary"));
  addParam("2DmatchCollection", m_2dmatch_tracklist, "the 2d tracklist with associated cluster", std::string("TRG2DMatchTracks"));
  addParam("PhimatchCollection", m_phimatch_tracklist, "the 2d tracklist with associated cluster", std::string("TRGPhiMatchTracks"));
  addParam("3DmatchCollection", m_3dmatch_tracklist, "the 3d NN tracklist with associated cluster", std::string("TRG3DMatchTracks"));
  addParam("KLMmatchCollection", m_klmmatch_tracklist, "the 2d tracklist with associated KLM track",
           std::string("TRGKLMMatchTracks"));
  addParam("GRLphotonCollection", m_grlphotonlist, "the isolated cluster list", std::string("TRGGRLPhotons"));
  addParam("hitCollectionName", m_hitCollectionName, "Name of the input StoreArray of CDCTriggerSegmentHits.", std::string(""));
  addParam("TrgGrlInformation", m_TrgGrlInformationName,
           "Name of the StoreArray holding the information of tracks and clusters from cdc ecl klm.",
           std::string("TRGGRLObjects"));
  addParam("grlstCollectionName", m_grlstCollectionName, "Name of the output StoreArray of TRGGRLShortTrack.",
           std::string("TRGGRLShortTracks"));
  addParam("grlitCollectionName", m_grlitCollectionName, "Name of the output StoreArray of TRGGRLInnerTrack.",
           std::string("TRGGRLInnerTracks"));


}

TRGGRLMatchModule::~TRGGRLMatchModule()
{
}

void TRGGRLMatchModule::initialize()
{
  B2DEBUG(100, "TRGGRLMatchModule processing");
  StoreArray<CDCTriggerTrack> track2Dlist(m_2d_tracklist);
  StoreArray<CDCTriggerTrack> track3Dlist(m_3d_tracklist);
  track2Dlist.isRequired();
  track3Dlist.isRequired();
  StoreArray<TRGECLCluster> clusterslist(m_clusterlist);
  clusterslist.isRequired();
  clusterslist.registerRelationTo(track2Dlist);
  clusterslist.registerRelationTo(track3Dlist);
  StoreObjPtr<KLMTrgSummary> klmtrgsummary(m_klmtrgsummarylist);
  klmtrgsummary.isRequired();

  StoreArray<CDCTriggerSegmentHit> tslist(m_hitCollectionName);
  tslist.isRequired();

  StoreArray<TRGGRLMATCH> track2Dmatch;
  track2Dmatch.registerInDataStore(m_2dmatch_tracklist);
  track2Dmatch.registerRelationTo(track2Dlist);
  track2Dmatch.registerRelationTo(clusterslist);

  StoreArray<TRGGRLMATCH> trackphimatch;
  trackphimatch.registerInDataStore(m_phimatch_tracklist);
  trackphimatch.registerRelationTo(track2Dlist);
  trackphimatch.registerRelationTo(clusterslist);

  StoreArray<TRGGRLMATCH> track3Dmatch;
  track3Dmatch.registerInDataStore(m_3dmatch_tracklist);
  track3Dmatch.registerRelationTo(clusterslist);
  track3Dmatch.registerRelationTo(track3Dlist);

  StoreArray<TRGGRLMATCHKLM> trackKLMmatch;
  trackKLMmatch.registerInDataStore(m_klmmatch_tracklist);
  trackKLMmatch.registerRelationTo(track2Dlist);

  StoreArray<TRGGRLPHOTON> grlphoton;
  grlphoton.registerInDataStore(m_grlphotonlist);
  grlphoton.registerRelationTo(clusterslist);

  StoreArray<TRGGRLShortTrack> grlst;
  grlst.registerInDataStore(m_grlstCollectionName);

  StoreArray<TRGGRLInnerTrack> grlit;
  grlit.registerInDataStore(m_grlitCollectionName);

  m_TRGGRLInfo.registerInDataStore(m_TrgGrlInformationName);

//-- Fill the patterns for short tracking

  fill_pattern_base2(patterns_base2);

  for (int p = 0; p < 137; p++) {
    int x0 = patterns_base2[p][0];
    int x1 = patterns_base2[p][1];
    int x2 = 0;
    int x3 = patterns_base2[p][2];
    int x4 = patterns_base2[p][3];
    int d = x2 - x0;
    x1 += d;
    x2 += d;
    x3 += d;
    x4 += d;
    patterns_base0.push_back({x1, x2, x3, x4});
  }


}

void TRGGRLMatchModule::beginRun()
{
}

void TRGGRLMatchModule::event()
{

  StoreArray<CDCTriggerTrack> track2Dlist(m_2d_tracklist);
  StoreArray<CDCTriggerTrack> track3Dlist(m_3d_tracklist);
  StoreArray<TRGECLCluster> clusterlist(m_clusterlist);
  StoreObjPtr<KLMTrgSummary> klmtrgsummary(m_klmtrgsummarylist);
  StoreArray<CDCTriggerSegmentHit> tslist(m_hitCollectionName);
  StoreArray<TRGGRLMATCH> track2Dmatch(m_2dmatch_tracklist);
  StoreArray<TRGGRLMATCH> trackphimatch(m_phimatch_tracklist);
  StoreArray<TRGGRLMATCH> track3Dmatch(m_3dmatch_tracklist);
  StoreArray<TRGGRLMATCHKLM> trackKLMmatch(m_klmmatch_tracklist);
  StoreArray<TRGGRLPHOTON> grlphoton(m_grlphotonlist);
  StoreArray<TRGGRLShortTrack> grlst(m_grlstCollectionName);
  StoreArray<TRGGRLInnerTrack> grlit(m_grlitCollectionName);
  StoreObjPtr<TRGGRLInfo> trgInfo(m_TrgGrlInformationName);
  trgInfo.create();

//initialize the phi map

  track_phimap.clear();
  track_phimap_i.clear();
  eecl_phimap.clear();
  eecl_phimap_fwd.clear();
  eecl_phimap_bwd.clear();
  eecl_sectormap_fwd.clear();
  eecl_sectormap_bwd.clear();
  eklm_sectormap.clear();
  eklm_sectormap_fwd.clear();
  eklm_sectormap_bwd.clear();

  for (int i = 0; i < 36; i++) {
    track_phimap.push_back(false);
    track_phimap_i.push_back(false);
    eecl_phimap.push_back(false);
    eecl_phimap_fwd.push_back(false);
    eecl_phimap_bwd.push_back(false);
  }
  for (int i = 0; i < 4; i++) {
    eecl_sectormap_fwd.push_back(false);
    eecl_sectormap_bwd.push_back(false);
    eklm_sectormap.push_back(false);
    eklm_sectormap_fwd.push_back(false);
    eklm_sectormap_bwd.push_back(false);
  }

//do 2d track match with ECL and KLM cluster
  int klmtrack_ind_phi_map[8] = {};
  for (int i = 0; i < track2Dlist.getEntries(); i++) {

    double dr_tmp = 99999.;
    int dphi_d_tmp = 100;
    double dphi_klm_tmp = 100;
    int cluster_ind = -1;
    int cluster_ind_phi = -1;
    int klmtrack_ind_phi = -1;

// do 2d track match with KLMTrgSummary
    sectormatching_klm(track2Dlist[i], klmtrgsummary, dphi_klm_tmp, klmtrack_ind_phi);

    for (int j = 0; j < clusterlist.getEntries(); j++) {
      // skip the end-cap cluster
      double _cluster_x = clusterlist[j]->getPositionX();
      double _cluster_y = clusterlist[j]->getPositionY();
      double _cluster_z = clusterlist[j]->getPositionZ();
      double _cluster_theta = atan(_cluster_z / (sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y)));
      _cluster_theta = 0.5 * M_PI - _cluster_theta;
      if (_cluster_theta < M_PI * 35.0 / 180.0 || _cluster_theta > M_PI * 126.0 / 180.0) continue;

      double ds_ct[2] = {99999., 99999.};
      calculationdistance(track2Dlist[i], clusterlist[j], ds_ct, 0);
      int dphi_d = 0;
      calculationphiangle(track2Dlist[i], clusterlist[j], dphi_d, track_phimap, track_phimap_i);

      if (dr_tmp > ds_ct[0]) {
        dr_tmp = ds_ct[0];
        cluster_ind = j;
      }
      if (dphi_d_tmp > dphi_d) {
        dphi_d_tmp = dphi_d;
        cluster_ind_phi = j;
      }

    }

    if (dr_tmp < m_dr_threshold && cluster_ind != -1) {
      TRGGRLMATCH* mat2d = track2Dmatch.appendNew();
      mat2d->setDeltaR(dr_tmp);
      mat2d->addRelationTo(track2Dlist[i]);
      mat2d->addRelationTo(clusterlist[cluster_ind]);
      //   track2Dlist[i]->addRelationTo(clusterlist[cluster_ind]);
      clusterlist[cluster_ind]->addRelationTo(track2Dlist[i]);
    }
    if (dphi_d_tmp < m_dphi_d_threshold && cluster_ind_phi != -1) {
      TRGGRLMATCH* matphi = trackphimatch.appendNew();
      matphi->set_dphi_d(dphi_d_tmp);
      matphi->addRelationTo(track2Dlist[i]);
      matphi->addRelationTo(clusterlist[cluster_ind_phi]);
      matphi->set_e(clusterlist[cluster_ind_phi]->getEnergyDep());
      //   track2Dlist[i]->addRelationTo(clusterlist[cluster_ind]);
      clusterlist[cluster_ind_phi]->addRelationTo(track2Dlist[i]);
    }

    if (dphi_klm_tmp < m_dphi_klm_threshold * M_PI / 180.0 && klmtrack_ind_phi > -1 && klmtrack_ind_phi < 8) {
      if (klmtrack_ind_phi_map[klmtrack_ind_phi] == 0) {
        TRGGRLMATCHKLM* matklm = trackKLMmatch.appendNew();
        matklm->set_dphi(dphi_klm_tmp);
        matklm->set_sector(klmtrack_ind_phi);
        matklm->addRelationTo(track2Dlist[i]);
        klmtrack_ind_phi_map[klmtrack_ind_phi] = 1;
      }
    }

  }


//do 3d track match with cluster
  for (int i = 0; i < track3Dlist.getEntries(); i++) {

    double dr_tmp = 99999.;
    double dz_tmp = 99999.;
    int cluster_ind = -1;
    for (int j = 0; j < clusterlist.getEntries(); j++) {
      // skip the end-cap cluster
      double _cluster_x = clusterlist[j]->getPositionX();
      double _cluster_y = clusterlist[j]->getPositionY();
      double _cluster_z = clusterlist[j]->getPositionZ();
      double _cluster_theta = atan(_cluster_z / (sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y)));
      _cluster_theta = 0.5 * M_PI - _cluster_theta;
      if (_cluster_theta < M_PI * 35.0 / 180.0 || _cluster_theta > M_PI * 126.0 / 180.0) continue;

      double ds_ct[2] = {99999., 99999.};
      calculationdistance(track3Dlist[i], clusterlist[j], ds_ct, 1);
      if (dr_tmp > ds_ct[0]) {
        dr_tmp = ds_ct[0];
        dz_tmp = ds_ct[1];
        cluster_ind = j;
      }
    }
    if (dr_tmp < m_dr_threshold && dz_tmp < m_dz_threshold && cluster_ind != -1) {
      TRGGRLMATCH* mat3d = track3Dmatch.appendNew();
      mat3d->setDeltaR(dr_tmp);
      mat3d->setDeltaZ(dz_tmp);
      mat3d->addRelationTo(track3Dlist[i]);
      mat3d->addRelationTo(clusterlist[cluster_ind]);
      clusterlist[cluster_ind]->addRelationTo(track3Dlist[i]);
    }
  }

//pick up isolated clusters as photons with energy thrshold
  for (int j = 0; j < clusterlist.getEntries(); j++) {
    if (photon_cluster(clusterlist[j], track_phimap, m_e_threshold)) {
      TRGGRLPHOTON* photon = grlphoton.appendNew();
      photon->set_e(clusterlist[j]->getEnergyDep());
      photon->addRelationTo(clusterlist[j]);
    }
  }

//endcap cluster map
  make_eecl_map(clusterlist, eecl_phimap, eecl_phimap_fwd, eecl_phimap_bwd, eecl_sectormap_fwd, eecl_sectormap_bwd);
  make_eklm_map(klmtrgsummary, eklm_sectormap, eklm_sectormap_fwd, eklm_sectormap_bwd);

// Short tracking
  std::vector<bool> map_veto(64, 0);
  make_veto_map(track2Dlist, map_veto);
  short_tracking(tslist, map_veto, track_phimap_i, eecl_phimap_fwd, eecl_phimap_bwd, eklm_sectormap_fwd, eklm_sectormap_bwd,
                 patterns_base0, patterns_base2, grlst, trgInfo);

// Inner tracking
  inner_tracking(tslist, track_phimap_i, eecl_phimap, eklm_sectormap, grlit, trgInfo);

// EECL-EKLM matching
  matching_eecl_eklm(eecl_sectormap_fwd, eecl_sectormap_bwd, eklm_sectormap_fwd, eklm_sectormap_bwd, trgInfo);

}

void TRGGRLMatchModule::endRun()
{
}

void TRGGRLMatchModule::terminate()
{
}

void TRGGRLMatchModule::calculationdistance(CDCTriggerTrack* _track, TRGECLCluster* _cluster, double* ds, int _match3D)
{

//double    _pt = _track->getTransverseMomentum(1.5);
  double    _r = 1.0 / _track->getOmega() ;
  double    _phi = _track->getPhi0() ;

  //-- cluster/TRGECL information
  double    _cluster_x = _cluster->getPositionX();
  double    _cluster_y = _cluster->getPositionY();
  double    _cluster_z = _cluster->getPositionZ();
  double    _R = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y);
//double    _D = sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y + _cluster_z * _cluster_z);
//double    _re_scaled_p = _pt * _D / _R;

  //-- calculation
  if (_R > abs(2 * _r)) {
    ds[0] = 99999.;
  } else {
    double theta0 = _phi - asin(_R / (2 * _r));

    double ex_x0 = _R * cos(theta0), ex_y0 = _R * sin(theta0);
    ds[0] = sqrt((ex_x0 - _cluster_x) * (ex_x0 - _cluster_x) + (ex_y0 - _cluster_y) * (ex_y0 - _cluster_y));
  }
  //z information
  if (_match3D == 1) {
    double      _z0 = _track->getZ0();
    double      _slope = _track->getCotTheta();
    double      _ex_z = _z0 + _slope * 2 * _r * asin(_R / (2 * _r));
    ds[1] = fabs(_cluster_z - _ex_z);

  }

}

void TRGGRLMatchModule::calculationphiangle(CDCTriggerTrack* _track, TRGECLCluster* _cluster, int& dphi_d,
                                            std::vector<bool>& phimap, std::vector<bool>& phimap_i)
{

  //-- 2D track information
  double    _r = 1.0 / _track->getOmega() ;
  double    _phi = _track->getPhi0() ;

  //-- 2D phi angle calculation
  double phi_p = acos(126.0 / (2 * fabs(_r))); // adjustment angle between 0 to 0.5*M_PI
  int charge = 0;
  if (_r > 0) {charge = 1;}
  else if (_r < 0) {charge = -1;}
  else {charge = 0;}

  double phi_CDC = 0.0;
  if (charge == 1) {
    phi_CDC = _phi + phi_p - 0.5 * M_PI;
  } else if (charge == -1) {
    phi_CDC = _phi - phi_p + 0.5 * M_PI;
  } else {
    phi_CDC = _phi;
  }

  if (phi_CDC > 2 * M_PI) {phi_CDC = phi_CDC - 2 * M_PI;}
  else if (phi_CDC < 0) {phi_CDC = phi_CDC + 2 * M_PI;}
  if (_phi > 2 * M_PI) {_phi = _phi - 2 * M_PI;}
  else if (_phi < 0) {_phi = _phi + 2 * M_PI;}

  //-- cluster/TRGECL information
  double    _cluster_x = _cluster->getPositionX();
  double    _cluster_y = _cluster->getPositionY();

  // -- ECL phi angle
  double phi_ECL = 0.0;
  if (_cluster_x >= 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x);}
  else if (_cluster_x < 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
  else if (_cluster_x < 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
  else if (_cluster_x >= 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + 2 * M_PI;}

  int phi_ECL_d = 0, phi_CDC_d = 0, phi_i_d = 0;
  // digitization on both angle
  for (int i = 0; i < 36; i++) {
    if (phi_ECL > i * M_PI / 18 && phi_ECL < (i + 1)*M_PI / 18) {phi_ECL_d = i;}
    if (_phi > i * M_PI / 18 && _phi < (i + 1)*M_PI / 18) {phi_i_d = i;}
    if (phi_CDC > i * M_PI / 18 && phi_CDC < (i + 1)*M_PI / 18) {phi_CDC_d = i;}
  }

  phimap[phi_CDC_d] = true;
  phimap_i[phi_i_d] = true;

  if (abs(phi_ECL_d - phi_CDC_d) == 0 || abs(phi_ECL_d - phi_CDC_d) == 36) {dphi_d = 0;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 1 || abs(phi_ECL_d - phi_CDC_d) == 35) {dphi_d = 1;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 2 || abs(phi_ECL_d - phi_CDC_d) == 34) {dphi_d = 2;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 3 || abs(phi_ECL_d - phi_CDC_d) == 33) {dphi_d = 3;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 4 || abs(phi_ECL_d - phi_CDC_d) == 32) {dphi_d = 4;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 5 || abs(phi_ECL_d - phi_CDC_d) == 31) {dphi_d = 5;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 6 || abs(phi_ECL_d - phi_CDC_d) == 30) {dphi_d = 6;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 7 || abs(phi_ECL_d - phi_CDC_d) == 29) {dphi_d = 7;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 8 || abs(phi_ECL_d - phi_CDC_d) == 28) {dphi_d = 8;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 9 || abs(phi_ECL_d - phi_CDC_d) == 27) {dphi_d = 9;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 10 || abs(phi_ECL_d - phi_CDC_d) == 26) {dphi_d = 10;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 11 || abs(phi_ECL_d - phi_CDC_d) == 25) {dphi_d = 11;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 12 || abs(phi_ECL_d - phi_CDC_d) == 24) {dphi_d = 12;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 13 || abs(phi_ECL_d - phi_CDC_d) == 23) {dphi_d = 13;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 14 || abs(phi_ECL_d - phi_CDC_d) == 22) {dphi_d = 14;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 15 || abs(phi_ECL_d - phi_CDC_d) == 21) {dphi_d = 15;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 16 || abs(phi_ECL_d - phi_CDC_d) == 20) {dphi_d = 16;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 17 || abs(phi_ECL_d - phi_CDC_d) == 19) {dphi_d = 17;}
  else if (abs(phi_ECL_d - phi_CDC_d) == 18) {dphi_d = 18;}

}

void TRGGRLMatchModule::sectormatching_klm(CDCTriggerTrack* _track, StoreObjPtr<KLMTrgSummary> _klmtrgsummary, double& dphi,
                                           int& phiid_klm)
{

  //-- 2D track information
  double    _r = 1.0 / _track->getOmega() ;
  double    _phi = _track->getPhi0() ;

  //-- 2D phi angle calculation (extrapolating up to superconducting coil)
  double phi_p = acos(176.0 / (2 * fabs(_r))); // adjustment angle between 0 to 0.5*M_PI
  int charge = 0;
  if (_r > 0) {charge = 1;}
  else if (_r < 0) {charge = -1;}
  else {charge = 0;}

  double phi_CDC = 0.0;
  if (charge == 1) {
    phi_CDC = _phi + phi_p - 0.5 * M_PI;
  } else if (charge == -1) {
    phi_CDC = _phi - phi_p + 0.5 * M_PI;
  } else {
    phi_CDC = _phi;
  }

  if (phi_CDC > 2 * M_PI) {phi_CDC = phi_CDC - 2 * M_PI;}
  else if (phi_CDC < 0) {phi_CDC = phi_CDC + 2 * M_PI;}

  // KLM track's sector central phi
  int _sector_mask_fw = _klmtrgsummary->getSector_mask_Forward_Barrel();
  int _sector_mask_bw = _klmtrgsummary->getSector_mask_Backward_Barrel();
  int _sector_mask = _sector_mask_fw | _sector_mask_bw;
  for (int _sector = 0; _sector < 8; _sector++) {
    if (_sector_mask & (1 << _sector)) {
      double _sector_central = 0.25 * M_PI * _sector;
      double dphi_temp;
      if (fabs(phi_CDC - _sector_central) < M_PI) { dphi_temp = fabs(phi_CDC - _sector_central); }
      else  { dphi_temp = 2 * M_PI - fabs(phi_CDC - _sector_central); }
      if (dphi_temp < dphi) {
        dphi = dphi_temp;
        phiid_klm = _sector;
      }
    }
  }

}

bool TRGGRLMatchModule::photon_cluster(TRGECLCluster* _cluster, std::vector<bool> phimap, double e_threshold)
{

  //-- cluster/TRGECL information
  double    _cluster_x = _cluster->getPositionX();
  double    _cluster_y = _cluster->getPositionY();
  double    _cluster_z = _cluster->getPositionZ();
  double    _cluster_theta = atan(_cluster_z / (sqrt(_cluster_x * _cluster_x + _cluster_y * _cluster_y)));
  _cluster_theta = 0.5 * M_PI - _cluster_theta;
  bool barrel = true;
  if (_cluster_theta < M_PI * 35.0 / 180.0 || _cluster_theta > M_PI * 126.0 / 180.0) {barrel = false;}
  double  _cluster_e = _cluster->getEnergyDep();

  // -- ECL phi angle
  double phi_ECL = 0.0;
  if (_cluster_x >= 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x);}
  else if (_cluster_x < 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
  else if (_cluster_x < 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
  else if (_cluster_x >= 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + 2 * M_PI;}

  int phi_ECL_d = 0;
  // digitization on both angle
  for (int i = 0; i < 36; i++) {
    if (phi_ECL > i * M_PI / 18 && phi_ECL < (i + 1)*M_PI / 18) {phi_ECL_d = i;}
  }

  int index = phi_ECL_d, index_p = phi_ECL_d + 1, index_m = phi_ECL_d - 1;
  if (index_p > 35) {index_p = index_p - 36;}
  if (index_m < 0) {index_m = index_m + 36;}

  if (!phimap[index] && !phimap[index_p] && !phimap[index_m] && _cluster_e >= e_threshold && barrel) {return true;}
  else if (!barrel) {return true;}
  else {return false;}

}

int TRGGRLMatchModule::N64(int x)
{
  if (x > 63) x -= 64;
  if (x < 0) x += 64;
  return x;
}

int TRGGRLMatchModule::N36(int x)
{
  if (x > 35) x -= 36;
  if (x < 0) x += 36;
  return x;
}

void TRGGRLMatchModule::fill_pattern_base2(std::vector< std::vector<int> >& patt)
{
  patt.push_back({ 0, 0, 0, 0});
  patt.push_back({ 0, -1, 0, 0});
  patt.push_back({ 0, -1, 1, 0});
  patt.push_back({ 0, -1, -1, 0});
  patt.push_back({ 0, -2, 0, 0});
  patt.push_back({ 0, -2, 1, 0});
  patt.push_back({ 0, -2, 2, 0});
  patt.push_back({ 0, -2, 3, 0});
  patt.push_back({ 0, -3, 1, 0});
  patt.push_back({ 0, -3, 2, 0});
  patt.push_back({ 0, -3, 3, 0});
  patt.push_back({ 0, -4, 2, 0});
  patt.push_back({ 0, -4, 3, 0});
  patt.push_back({ 0, 0, 0, 1});
  patt.push_back({ 0, 0, 1, 1});
  patt.push_back({ 0, -1, 0, 1});
  patt.push_back({ 0, -1, 1, 1});
  patt.push_back({ 0, -1, 2, 1});
  patt.push_back({ 0, -2, 2, 1});
  patt.push_back({ 0, -2, 3, 1});
  patt.push_back({ 0, -3, 2, 1});
  patt.push_back({ 0, -3, 3, 1});
  patt.push_back({ 0, 0, 0, -1});
  patt.push_back({ 0, 0, -1, -1});
  patt.push_back({ 0, -1, 0, -1});
  patt.push_back({ 0, -1, -1, -1});
  patt.push_back({ 0, -2, 0, -1});
  patt.push_back({ 0, -2, 1, -1});
  patt.push_back({ 0, -3, 1, -1});
  patt.push_back({ 0, -3, 2, -1});
  patt.push_back({ -1, -1, 0, 0});
  patt.push_back({ -1, -1, 1, 0});
  patt.push_back({ -1, -2, 0, 0});
  patt.push_back({ -1, -2, 1, 0});
  patt.push_back({ -1, -3, 1, 0});
  patt.push_back({ -1, -3, 2, 0});
  patt.push_back({ -1, -3, 3, 0});
  patt.push_back({ -1, -4, 2, 0});
  patt.push_back({ -1, -4, 3, 0});
  patt.push_back({ 1, 0, 1, 0});
  patt.push_back({ 1, 0, 0, 0});
  patt.push_back({ 1, 0, -1, 0});
  patt.push_back({ 1, -1, 0, 0});
  patt.push_back({ 1, -1, 1, 0});
  patt.push_back({ 1, -2, 2, 0});
  patt.push_back({ 1, -2, 3, 0});
  patt.push_back({ 1, -3, 2, 0});
  patt.push_back({ 1, -3, 3, 0});
  patt.push_back({ -1, -1, 0, 1});
  patt.push_back({ -1, -1, 1, 1});
  patt.push_back({ -1, -2, 0, 1});
  patt.push_back({ -1, -2, 1, 1});
  patt.push_back({ -1, -2, 2, 1});
  patt.push_back({ -1, -3, 1, 1});
  patt.push_back({ -1, -3, 2, 1});
  patt.push_back({ -1, -3, 3, 1});
  patt.push_back({ -1, -4, 2, 1});
  patt.push_back({ -1, -4, 3, 1});
  patt.push_back({ 1, 0, -1, -1});
  patt.push_back({ 1, 0, 0, -1});
  patt.push_back({ 1, -1, -1, -1});
  patt.push_back({ 1, -1, 0, -1});
  patt.push_back({ 1, -1, 1, -1});
  patt.push_back({ 1, -2, 1, -1});
  patt.push_back({ 1, -2, 2, -1});
  patt.push_back({ 1, -3, 1, -1});
  patt.push_back({ 1, -3, 2, -1});
  patt.push_back({ -1, -1, 1, 2});
  patt.push_back({ -1, -1, 2, 2});
  patt.push_back({ -1, -2, 1, 2});
  patt.push_back({ -1, -2, 2, 2});
  patt.push_back({ -1, -2, 3, 2});
  patt.push_back({ -1, -3, 2, 2});
  patt.push_back({ -1, -3, 3, 2});
  patt.push_back({ -1, -3, 4, 2});
  patt.push_back({ 1, 0, -1, -2});
  patt.push_back({ 1, 0, 0, -2});
  patt.push_back({ 1, -1, 1, -2});
  patt.push_back({ 1, -1, 0, -2});
  patt.push_back({ 1, -1, -1, -2});
  patt.push_back({ 1, -2, 0, -2});
  patt.push_back({ 1, -2, 1, -2});
  patt.push_back({ -2, -2, 0, 1});
  patt.push_back({ -2, -2, 1, 1});
  patt.push_back({ -2, -3, 1, 1});
  patt.push_back({ -2, -3, 2, 1});
  patt.push_back({ -2, -4, 2, 1});
  patt.push_back({ -2, -4, 3, 1});
  patt.push_back({ -2, -5, 3, 1});
  patt.push_back({ 2, 1, 0, -1});
  patt.push_back({ 2, 0, 1, -1});
  patt.push_back({ 2, 0, 0, -1});
  patt.push_back({ 2, 0, -1, -1});
  patt.push_back({ 2, -1, 1, -1});
  patt.push_back({ 2, -1, 0, -1});
  patt.push_back({ 2, -2, 2, -1});
  patt.push_back({ 2, -2, 1, -1});
  patt.push_back({ -2, -2, 1, 2});
  patt.push_back({ -2, -2, 2, 2});
  patt.push_back({ -2, -3, 1, 2});
  patt.push_back({ -2, -3, 2, 2});
  patt.push_back({ -2, -3, 3, 2});
  patt.push_back({ -2, -4, 2, 2});
  patt.push_back({ -2, -4, 3, 2});
  patt.push_back({ -2, -4, 4, 2});
  patt.push_back({ 2, 1, 0, -2});
  patt.push_back({ 2, 1, -1, -2});
  patt.push_back({ 2, 0, 1, -2});
  patt.push_back({ 2, 0, 0, -2});
  patt.push_back({ 2, 0, -1, -2});
  patt.push_back({ 2, 0, -2, -2});
  patt.push_back({ 2, -1, 2, -2});
  patt.push_back({ 2, -1, 1, -2});
  patt.push_back({ 2, -1, 0, -2});
  patt.push_back({ 2, -1, -1, -2});
  patt.push_back({ 2, -2, 0, -2});
  patt.push_back({ 2, -2, 1, -2});
  patt.push_back({ -2, -2, 1, 3});
  patt.push_back({ -2, -2, 2, 3});
  patt.push_back({ -2, -3, 2, 3});
  patt.push_back({ -2, -3, 3, 3});
  patt.push_back({ -2, -3, 4, 3});
  patt.push_back({ -2, -4, 3, 3});
  patt.push_back({ -2, -4, 4, 3});
  patt.push_back({ 2, 1, -1, -3});
  patt.push_back({ 2, 0, -1, -3});
  patt.push_back({ 2, 0, -2, -3});
  patt.push_back({ 2, -1, 0, -3});
  patt.push_back({ 2, -2, 0, -3});
  patt.push_back({ 2, -2, 1, -3});
  patt.push_back({ -2, -2, 2, 4});
  patt.push_back({ -2, -3, 3, 4});
  patt.push_back({ -2, -3, 4, 4});
  patt.push_back({ -2, -4, 4, 4});
  patt.push_back({ 2, -1, 0, 4});
  patt.push_back({ 2, -1, -1, 4});
  patt.push_back({ 2, -2, 0, 4});

}

void TRGGRLMatchModule::make_veto_map(StoreArray<CDCTriggerTrack> track2Dlist, std::vector<bool>&  map_veto)
{
  for (int i = 0; i < track2Dlist.getEntries(); i++) {
    int _w = (int)(2271.7 * track2Dlist[i]->getOmega()) ; // omega from -33 to 33
    if (_w >= 33) { _w = 33;}
    else if (_w <= -33) { _w = -33;}
    int _phi = (int)((track2Dlist[i]->getPhi0() + 2 * M_PI) / (M_PI / 32.0)); // phi_i digitized to 0 ~ 63

    int charge = 0;
    if (_w > 0) {charge = 1;}
    else if (_w < 0) {charge = -1;}
    else {charge = 0;}

    _w = abs(_w);

    int L;
    if (_w >= 0 && _w <= 8) { L = _phi; }
    else if (_w >= 9 && _w <= 15) {
      if (charge < 0) { L = _phi + 1; }
      else { L = _phi; }
    } else if (_w >= 16 && _w <= 24) {
      if (charge < 0) { L = _phi + 2; }
      else { L = _phi; }
    } else if (_w >= 25 && _w <= 27) {
      if (charge < 0) { L = _phi + 3; }
      else { L = _phi; }
    } else if (_w >= 28 && _w <= 30) {
      if (charge < 0) { L = _phi + 3; }
      else { L = _phi + 1; }
    } else if (_w >= 31 && _w <= 32) {
      if (charge < 0) { L = _phi + 4; }
      else { L = _phi + 1; }
    } else {
      if (charge < 0) { L = _phi + 5; }
      else { L = _phi + 1; }
    }

    int R;
    if (_w >= 0 && _w <= 8) { R = _phi; }
    else if (_w >= 9 && _w <= 15) {
      if (charge < 0) { R = _phi; }
      else { R = _phi - 1; }
    } else if (_w >= 16 && _w <= 24) {
      if (charge < 0) { R = _phi; }
      else { R = _phi - 2; }
    } else if (_w >= 25 && _w <= 27) {
      if (charge < 0) { R = _phi; }
      else { R = _phi - 3; }
    } else if (_w >= 28 && _w <= 30) {
      if (charge < 0) { R = _phi + 1; }
      else { R = _phi - 3; }
    } else if (_w >= 21 && _w <= 32) {
      if (charge < 0) { R = _phi + 1; }
      else { R = _phi - 4; }
    } else {
      if (charge < 0) { R = _phi + 1; }
      else { R = _phi - 5; }
    }

    // L should be > R
    for (int j = R - 1; j < L + 2; j++) {
      map_veto[N64(j)] = true;
    }
  }

}

void TRGGRLMatchModule::make_eecl_map(StoreArray<TRGECLCluster> clusterlist,
                                      std::vector<bool>& ecl_phimap, std::vector<bool>& ecl_phimap_fwd, std::vector<bool>& ecl_phimap_bwd,
                                      std::vector<bool>& ecl_sectormap_fwd, std::vector<bool>& ecl_sectormap_bwd)
{
  bool ecl_phimap_loose_fwd[36];
  bool ecl_phimap_loose_bwd[36];
  for (int i = 0; i < 36; i++) {
    ecl_phimap_loose_fwd[i] = false;
    ecl_phimap_loose_bwd[i] = false;
  }

  for (int iclst = 0; iclst < clusterlist.getEntries(); iclst++) {
    //-- cluster/TRGECL information
    double    _cluster_x = clusterlist[iclst]->getPositionX();
    double    _cluster_y = clusterlist[iclst]->getPositionY();

    // -- ECL phi angle
    double phi_ECL = 0.0;
    if (_cluster_x >= 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x);}
    else if (_cluster_x < 0 && _cluster_y >= 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
    else if (_cluster_x < 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + M_PI;}
    else if (_cluster_x >= 0 && _cluster_y < 0) {phi_ECL = atan(_cluster_y / _cluster_x) + 2 * M_PI;}

    int phi_ECL_d = 0;
    // digitization on both angle
    for (int i = 0; i < 36; i++) {
      if (phi_ECL > i * M_PI / 18 && phi_ECL < (i + 1)*M_PI / 18) {phi_ECL_d = i;}
    }

    //fill endcap only
    int _cluster_thetaid = clusterlist[iclst]->getMaxThetaId();
    if (_cluster_thetaid < 4 || _cluster_thetaid > 15) ecl_phimap[phi_ECL_d] = true;
    if (_cluster_thetaid < 4)  ecl_phimap_fwd[phi_ECL_d] = true;
    if (_cluster_thetaid > 15) ecl_phimap_bwd[phi_ECL_d] = true;
    if (_cluster_thetaid < 5)  ecl_phimap_loose_fwd[phi_ECL_d] = true;
    if (_cluster_thetaid > 14) ecl_phimap_loose_bwd[phi_ECL_d] = true;
  }

  //-- 36b into 4b
  ecl_sectormap_fwd[0] = ecl_phimap_loose_fwd[35] or ecl_phimap_loose_fwd[0] or ecl_phimap_loose_fwd[1] or ecl_phimap_loose_fwd[2] or
                         ecl_phimap_loose_fwd[3] or ecl_phimap_loose_fwd[4] or ecl_phimap_loose_fwd[5] or ecl_phimap_loose_fwd[6] or
                         ecl_phimap_loose_fwd[7] or ecl_phimap_loose_fwd[8] or ecl_phimap_loose_fwd[9];
  ecl_sectormap_fwd[1] = ecl_phimap_loose_fwd[8]  or ecl_phimap_loose_fwd[9] or ecl_phimap_loose_fwd[10] or ecl_phimap_loose_fwd[11]
                         or
                         ecl_phimap_loose_fwd[12] or ecl_phimap_loose_fwd[13] or ecl_phimap_loose_fwd[14] or ecl_phimap_loose_fwd[15] or
                         ecl_phimap_loose_fwd[16] or ecl_phimap_loose_fwd[17] or ecl_phimap_loose_fwd[18] or ecl_phimap_loose_fwd[19];
  ecl_sectormap_fwd[2] = ecl_phimap_loose_fwd[18]  or ecl_phimap_loose_fwd[19] or ecl_phimap_loose_fwd[20]
                         or ecl_phimap_loose_fwd[21] or
                         ecl_phimap_loose_fwd[22] or ecl_phimap_loose_fwd[23] or ecl_phimap_loose_fwd[24] or ecl_phimap_loose_fwd[25] or
                         ecl_phimap_loose_fwd[26] or ecl_phimap_loose_fwd[27] or ecl_phimap_loose_fwd[28];
  ecl_sectormap_fwd[3] = ecl_phimap_loose_fwd[26]  or ecl_phimap_loose_fwd[27] or ecl_phimap_loose_fwd[28]
                         or ecl_phimap_loose_fwd[29] or
                         ecl_phimap_loose_fwd[30] or ecl_phimap_loose_fwd[31] or ecl_phimap_loose_fwd[32] or ecl_phimap_loose_fwd[33] or
                         ecl_phimap_loose_fwd[34] or ecl_phimap_loose_fwd[36] or ecl_phimap_loose_fwd[0];
  //-- 36b into 4b
  ecl_sectormap_bwd[0] = ecl_phimap_loose_bwd[35] or ecl_phimap_loose_bwd[0] or ecl_phimap_loose_bwd[1] or ecl_phimap_loose_bwd[2] or
                         ecl_phimap_loose_bwd[3] or ecl_phimap_loose_bwd[4] or ecl_phimap_loose_bwd[5] or ecl_phimap_loose_bwd[6] or
                         ecl_phimap_loose_bwd[7] or ecl_phimap_loose_bwd[8] or ecl_phimap_loose_bwd[9];
  ecl_sectormap_bwd[1] = ecl_phimap_loose_bwd[8]  or ecl_phimap_loose_bwd[9] or ecl_phimap_loose_bwd[10] or ecl_phimap_loose_bwd[11]
                         or
                         ecl_phimap_loose_bwd[12] or ecl_phimap_loose_bwd[13] or ecl_phimap_loose_bwd[14] or ecl_phimap_loose_bwd[15] or
                         ecl_phimap_loose_bwd[16] or ecl_phimap_loose_bwd[17] or ecl_phimap_loose_bwd[18] or ecl_phimap_loose_bwd[19];
  ecl_sectormap_bwd[2] = ecl_phimap_loose_bwd[18]  or ecl_phimap_loose_bwd[19] or ecl_phimap_loose_bwd[20]
                         or ecl_phimap_loose_bwd[21] or
                         ecl_phimap_loose_bwd[22] or ecl_phimap_loose_bwd[23] or ecl_phimap_loose_bwd[24] or ecl_phimap_loose_bwd[25] or
                         ecl_phimap_loose_bwd[26] or ecl_phimap_loose_bwd[27] or ecl_phimap_loose_bwd[28];
  ecl_sectormap_bwd[3] = ecl_phimap_loose_bwd[26]  or ecl_phimap_loose_bwd[27] or ecl_phimap_loose_bwd[28]
                         or ecl_phimap_loose_bwd[29] or
                         ecl_phimap_loose_bwd[30] or ecl_phimap_loose_bwd[31] or ecl_phimap_loose_bwd[32] or ecl_phimap_loose_bwd[33] or
                         ecl_phimap_loose_bwd[34] or ecl_phimap_loose_bwd[36] or ecl_phimap_loose_bwd[0];

}

void TRGGRLMatchModule::make_eklm_map(StoreObjPtr<KLMTrgSummary> _klmtrgsummary,
                                      std::vector<bool>& _eklm_sectormap, std::vector<bool>& _eklm_sectormap_fwd, std::vector<bool>& _eklm_sectormap_bwd)
{

  int _sector_mask_fw = _klmtrgsummary->getSector_mask_Forward_Endcap();
  int _sector_mask_bw = _klmtrgsummary->getSector_mask_Backward_Endcap();

  for (int _sector = 0; _sector < 4; _sector++) {
    //if(_sector_mask_fw & (1<<_sector) ) _eklm_sectormap_fwd[_sector]=true;
    if (_sector_mask_bw & (1 << _sector)) _eklm_sectormap_bwd[_sector] = true;
    //if(_sector_mask &    (1<<_sector) ) _eklm_sectormap[_sector]=true;
  }
  if (_sector_mask_fw & (1 << 0)) _eklm_sectormap_fwd[1] = true;
  if (_sector_mask_fw & (1 << 1)) _eklm_sectormap_fwd[0] = true;
  if (_sector_mask_fw & (1 << 2)) _eklm_sectormap_fwd[3] = true;
  if (_sector_mask_fw & (1 << 3)) _eklm_sectormap_fwd[2] = true;

  for (int _sector = 0; _sector < 4; _sector++) {
    _eklm_sectormap[_sector] = (_eklm_sectormap_fwd[_sector] || _eklm_sectormap_bwd[_sector]);
  }
}


void TRGGRLMatchModule::short_tracking(StoreArray<CDCTriggerSegmentHit> tslist, std::vector<bool>  map_veto,
                                       std::vector<bool>  phimap_i,
                                       std::vector<bool>  ecl_phimap_fwd,
                                       std::vector<bool>  ecl_phimap_bwd,
                                       std::vector<bool>  klm_sectormap_fwd,
                                       std::vector<bool>  klm_sectormap_bwd,
                                       std::vector< std::vector<int> >& pattern_base0, std::vector< std::vector<int> >& pattern_base2,
                                       StoreArray<TRGGRLShortTrack> grlst,
                                       StoreObjPtr<TRGGRLInfo> trgInfo)
{
  std::vector<bool> SL0(64, 0);
  std::vector<bool> SL1(64, 0);
  std::vector<bool> SL2(64, 0);
  std::vector<bool> SL3(64, 0);
  std::vector<bool> SL4(64, 0);
  std::vector<bool> ST0(64, 0);
  std::vector<bool> ST0_36b(36, 0);
  std::vector<bool> ST2(64, 0);
  std::vector<int> patt_ID(64, -1);

  std::vector<bool> st_ec1(64, 0);
  std::vector<bool> st_ec1_36b(36, 0);
  std::vector<bool> st_ec1_4b(4, 0);
  std::vector<bool> st_ec2(64, 0);
  std::vector<bool> st_ec2_36b(36, 0);
  std::vector<bool> st_ec2_4b(4, 0);

//-- collecting TSF info in SL0~4
  for (int i = 0; i < tslist.getEntries(); i++) {
    int id = tslist[i]->getSegmentID();
    int sl = 0;
    if (id >= 0 * 32 && id < 5 * 32) {sl = 0; id -= 0;}
    else if (id >= 5 * 32 && id < 10 * 32) {sl = 1; id -= 5 * 32;}
    else if (id >= 10 * 32 && id < 16 * 32) {sl = 2; id -= 10 * 32;}
    else if (id >= 16 * 32 && id < 23 * 32) {sl = 3; id -= 16 * 32;}
    else if (id >= 23 * 32 && id < 31 * 32) {sl = 4; id -= 23 * 32;}
    else continue;

    if (sl == 0) {
      int X = (int)(id / 5), Y = id % 5;
      if (Y == 0 || Y == 1) { SL0[2 * X] = true; }
      else if (Y == 3 || Y == 4) { SL0[2 * X + 1] = true; }
      else  { SL0[2 * X] = true; SL0[2 * X + 1] = true; }
    } else if (sl == 1) {
      int X = (int)(id / 5), Y = id % 5;
      if (Y == 0 || Y == 1) { SL1[2 * X] = true; }
      else if (Y == 3 || Y == 4) { SL1[2 * X + 1] = true; }
      else  { SL1[2 * X] = true; SL1[2 * X + 1] = true; }
    } else if (sl == 2) {
      int X = (int)(id / 3);
      SL2[X] = true;
    } else if (sl == 3) {
      int X = (int)(id / 7), Y = id % 7;
      if (Y == 0 || Y == 1 || Y == 2) { SL3[2 * X] = true; }
      else if (Y == 4 || Y == 5 || Y == 6) { SL3[2 * X + 1] = true; }
      else  { SL3[2 * X] = true; SL3[2 * X + 1] = true; }
    } else if (sl == 4) {
      int X = (int)(id / 4);
      SL4[X] = true;
    }

  }

//-- making veto
  for (int i = 0; i < 64; i++) {
    if (map_veto[i]) {SL0[i] = false; SL1[i] = false; SL2[i] = false;}
  }
  /*
        for (int i = 0; i < 64; i++) { std::cout<<map_veto[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
        for (int i = 0; i < 64; i++) { std::cout<<SL4[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
        for (int i = 0; i < 64; i++) { std::cout<<SL3[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
        for (int i = 0; i < 64; i++) { std::cout<<SL2[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
        for (int i = 0; i < 64; i++) { std::cout<<SL1[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
        for (int i = 0; i < 64; i++) { std::cout<<SL0[63-i]; if((64-i)%10==1) std::cout<<" ";}
        std::cout<<std::endl;
  */
//-- doing short tracking

  std::vector< std::vector<int> > stlist_buf(0);

  // -- ST finding with SL2
  for (int i = 0; i < 64; i++) {

    int ID0 = 0;
    int ID1 = 0;
    int ID2 = 0;
    int ID3 = 0;
    int ID4 = 0;
    stlist_buf.push_back({0, 0, 0, 0, 0, 0});

    if (!SL2[i]) continue;
    bool SL2_already_found = false;

    for (int p = 0; p < 137; p++) {

      // following patterns will not be used.
      if (p == 4) continue;
      if (p == 5) continue;
      if (p == 17) continue;
      if (p == 26) continue;
      if (p == 38) continue;
      if (p == 41) continue;
      if (p == 42) continue;
      if (p == 47) continue;
      if (p == 50) continue;
      if (p == 60) continue;
      if (p == 63) continue;
      if (p == 64) continue;
      if (p == 74) continue;
      if (p == 93) continue;
      if (p == 94) continue;
      if (p == 95) continue;
      if (p == 96) continue;
      if (p == 104) continue;
      if (p == 113) continue;
      if (p == 114) continue;
      if (p == 115) continue;
      if (p == 123) continue;
      if (p == 134) continue;
      if (p == 135) continue;
      if (p == 136) continue;

      int x0 = pattern_base2[p][0];
      int x1 = pattern_base2[p][1];
      int x3 = pattern_base2[p][2];
      int x4 = pattern_base2[p][3];


      if (SL2[i] && SL0[N64(i + x0)] && SL1[N64(i + x1)] && SL3[N64(i + x3)] && SL4[N64(i + x4)] && !SL2_already_found) {
        ST2[i] = true;
        ID0 = N64(i + x0);
        ID1 = N64(i + x1);
        ID2 = i;
        ID3 = N64(i + x3);
        ID4 = N64(i + x4);
        SL2_already_found = true; // if it has been found in previous pattern, no need to do it again.
      }

      // if a pattern is found, no need to look for other pattern
      if (SL2_already_found) break;

    }

    if (SL2_already_found) {
      stlist_buf[i][0] = 1;
      stlist_buf[i][1] = ID0;
      stlist_buf[i][2] = ID1;
      stlist_buf[i][3] = ID2;
      stlist_buf[i][4] = ID3;
      stlist_buf[i][5] = ID4;
    }
  }
/////////
//-- ST finding with SL0
  for (int i = 0; i < 64; i++) {

    if (!SL0[i]) continue;
    bool SL0_already_found = false;

    for (int p = 0; p < 137; p++) {

      // following patterns will not be used.
      if (p == 4) continue;
      if (p == 5) continue;
      if (p == 17) continue;
      if (p == 26) continue;
      if (p == 38) continue;
      if (p == 41) continue;
      if (p == 42) continue;
      if (p == 47) continue;
      if (p == 50) continue;
      if (p == 60) continue;
      if (p == 63) continue;
      if (p == 64) continue;
      if (p == 74) continue;
      if (p == 93) continue;
      if (p == 94) continue;
      if (p == 95) continue;
      if (p == 96) continue;
      if (p == 104) continue;
      if (p == 113) continue;
      if (p == 114) continue;
      if (p == 115) continue;
      if (p == 123) continue;
      if (p == 134) continue;
      if (p == 135) continue;
      if (p == 136) continue;

      int y1 = pattern_base0[p][0];
      int y2 = pattern_base0[p][1];
      int y3 = pattern_base0[p][2];
      int y4 = pattern_base0[p][3];

      if (SL0[i] && SL1[N64(i + y1)] && SL2[N64(i + y2)] && SL3[N64(i + y3)] && SL4[N64(i + y4)] && !SL0_already_found) {
        ST0[i] = true;
        if (patt_ID[i] < 0) { patt_ID[i] = p; }
        SL0_already_found = true; // if it has been found in previous pattern, no need to do it again.
      }

      // if a pattern is found, no need to look for other pattern
      if (SL0_already_found) break;

    }

  }
/////////
//-- extrapolation
  for (int i = 0; i < 64; i++) {
    if (patt_ID[i] == -1) continue;

    int ec = 0, l = 0, r = 0;
    extrapolation(patt_ID[i], l, r, ec);
    if (ec == 1) {
      for (int e = l; e <= r; e++) { st_ec1[N64(i + e)] = true; }
    }
    if (ec == 2) {
      for (int e = l; e <= r; e++) { st_ec2[N64(i + e)] = true; }
    }

  }
//-- 64b into 36b
  for (int i = 0; i < 4; i++) {
    ST0_36b[0 + 9 * i] = ST0[0 + 16 * i] or ST0[1 + 16 * i];
    ST0_36b[1 + 9 * i] = ST0[1 + 16 * i] or ST0[2 + 16 * i] or ST0[3 + 16 * i];
    ST0_36b[2 + 9 * i] = ST0[3 + 16 * i] or ST0[4 + 16 * i] or ST0[5 + 16 * i];
    ST0_36b[3 + 9 * i] = ST0[5 + 16 * i] or ST0[6 + 16 * i] or ST0[7 + 16 * i];
    ST0_36b[4 + 9 * i] = ST0[7 + 16 * i] or ST0[8 + 16 * i];
    ST0_36b[5 + 9 * i] = ST0[8 + 16 * i] or ST0[9 + 16 * i] or ST0[10 + 16 * i];
    ST0_36b[6 + 9 * i] = ST0[10 + 16 * i] or ST0[11 + 16 * i] or ST0[12 + 16 * i];
    ST0_36b[7 + 9 * i] = ST0[12 + 16 * i] or ST0[13 + 16 * i] or ST0[14 + 16 * i];
    ST0_36b[8 + 9 * i] = ST0[14 + 16 * i] or ST0[15 + 16 * i];
    st_ec1_36b[0 + 9 * i] = st_ec1[0 + 16 * i] or st_ec1[1 + 16 * i];
    st_ec1_36b[1 + 9 * i] = st_ec1[1 + 16 * i] or st_ec1[2 + 16 * i] or st_ec1[3 + 16 * i];
    st_ec1_36b[2 + 9 * i] = st_ec1[3 + 16 * i] or st_ec1[4 + 16 * i] or st_ec1[5 + 16 * i];
    st_ec1_36b[3 + 9 * i] = st_ec1[5 + 16 * i] or st_ec1[6 + 16 * i] or st_ec1[7 + 16 * i];
    st_ec1_36b[4 + 9 * i] = st_ec1[7 + 16 * i] or st_ec1[8 + 16 * i];
    st_ec1_36b[5 + 9 * i] = st_ec1[8 + 16 * i] or st_ec1[9 + 16 * i] or st_ec1[10 + 16 * i];
    st_ec1_36b[6 + 9 * i] = st_ec1[10 + 16 * i] or st_ec1[11 + 16 * i] or st_ec1[12 + 16 * i];
    st_ec1_36b[7 + 9 * i] = st_ec1[12 + 16 * i] or st_ec1[13 + 16 * i] or st_ec1[14 + 16 * i];
    st_ec1_36b[8 + 9 * i] = st_ec1[14 + 16 * i] or st_ec1[15 + 16 * i];

    st_ec2_36b[0 + 9 * i] = st_ec2[0 + 16 * i] or st_ec2[1 + 16 * i];
    st_ec2_36b[1 + 9 * i] = st_ec2[1 + 16 * i] or st_ec2[2 + 16 * i] or st_ec2[3 + 16 * i];
    st_ec2_36b[2 + 9 * i] = st_ec2[3 + 16 * i] or st_ec2[4 + 16 * i] or st_ec2[5 + 16 * i];
    st_ec2_36b[3 + 9 * i] = st_ec2[5 + 16 * i] or st_ec2[6 + 16 * i] or st_ec2[7 + 16 * i];
    st_ec2_36b[4 + 9 * i] = st_ec2[7 + 16 * i] or st_ec2[8 + 16 * i];
    st_ec2_36b[5 + 9 * i] = st_ec2[8 + 16 * i] or st_ec2[9 + 16 * i] or st_ec2[10 + 16 * i];
    st_ec2_36b[6 + 9 * i] = st_ec2[10 + 16 * i] or st_ec2[11 + 16 * i] or st_ec2[12 + 16 * i];
    st_ec2_36b[7 + 9 * i] = st_ec2[12 + 16 * i] or st_ec2[13 + 16 * i] or st_ec2[14 + 16 * i];
    st_ec2_36b[8 + 9 * i] = st_ec2[14 + 16 * i] or st_ec2[15 + 16 * i];
  }
//-- 36b into 4b
  st_ec1_4b[0] = st_ec1_36b[35] or st_ec1_36b[0] or st_ec1_36b[1] or st_ec1_36b[2] or st_ec1_36b[3] or st_ec1_36b[4] or st_ec1_36b[5]
                 or st_ec1_36b[6] or st_ec1_36b[7] or st_ec1_36b[8] or st_ec1_36b[9];
  st_ec1_4b[1] = st_ec1_36b[8]  or st_ec1_36b[9] or st_ec1_36b[10] or st_ec1_36b[11] or st_ec1_36b[12] or st_ec1_36b[13]
                 or st_ec1_36b[14] or st_ec1_36b[15] or st_ec1_36b[16] or st_ec1_36b[17] or st_ec1_36b[18] or st_ec1_36b[19];
  st_ec1_4b[2] = st_ec1_36b[18]  or st_ec1_36b[19] or st_ec1_36b[20] or st_ec1_36b[21] or st_ec1_36b[22] or st_ec1_36b[23]
                 or st_ec1_36b[24] or st_ec1_36b[25] or st_ec1_36b[26] or st_ec1_36b[27] or st_ec1_36b[28];
  st_ec1_4b[3] = st_ec1_36b[26]  or st_ec1_36b[27] or st_ec1_36b[28] or st_ec1_36b[29] or st_ec1_36b[30] or st_ec1_36b[31]
                 or st_ec1_36b[32] or st_ec1_36b[33] or st_ec1_36b[34] or st_ec1_36b[35] or st_ec1_36b[0];
  st_ec2_4b[0] = st_ec2_36b[35] or st_ec2_36b[0] or st_ec2_36b[1] or st_ec2_36b[2] or st_ec2_36b[3] or st_ec2_36b[4] or st_ec2_36b[5]
                 or st_ec2_36b[6] or st_ec2_36b[7] or st_ec2_36b[8] or st_ec2_36b[9];
  st_ec2_4b[1] = st_ec2_36b[8]  or st_ec2_36b[9] or st_ec2_36b[10] or st_ec2_36b[11] or st_ec2_36b[12] or st_ec2_36b[13]
                 or st_ec2_36b[14] or st_ec2_36b[15] or st_ec2_36b[16] or st_ec2_36b[17] or st_ec2_36b[18] or st_ec2_36b[19];
  st_ec2_4b[2] = st_ec2_36b[18]  or st_ec2_36b[19] or st_ec2_36b[20] or st_ec2_36b[21] or st_ec2_36b[22] or st_ec2_36b[23]
                 or st_ec2_36b[24] or st_ec2_36b[25] or st_ec2_36b[26] or st_ec2_36b[27] or st_ec2_36b[28];
  st_ec2_4b[3] = st_ec2_36b[26]  or st_ec2_36b[27] or st_ec2_36b[28] or st_ec2_36b[29] or st_ec2_36b[30] or st_ec2_36b[31]
                 or st_ec2_36b[32] or st_ec2_36b[33] or st_ec2_36b[34] or st_ec2_36b[35] or st_ec2_36b[0];




//-- Summary info

  int N_ST = 0;
  int N_ST_fwd = 0;
  int N_ST_bwd = 0;
  bool s2s3 = false;
  bool s2s5 = false;
  bool s2so = false;
  bool s2s30 = false;
  bool s2f3 = false;
  bool s2f5 = false;
  bool s2fo = false;
  bool s2f30 = false;
  int secl = 0;
  int secl_fwd = 0;
  int secl_bwd = 0;
  int sklm = 0;
  int sklm_fwd = 0;
  int sklm_bwd = 0;

//-- short track counting on ST2
  for (int i = 0; i < 64; i++) {
    if (ST2[i]) {
      N_ST++;
      ST2[i] = false;
      int L = i - 1, R = i + 1;
      while (ST2[N64(L)]) {
        ST2[N64(L)] = false;
        L--;
      }
      while (ST2[N64(R)]) {
        ST2[N64(R)] = false;
        R++;
      }

      //-- Fill the store array
      L++; R--;
      int index = N64((L + R) / 2); // fill the middle one when multiple ST is found continuously in the map
      TRGGRLShortTrack* st = grlst.appendNew();
      st->set_TS_ID(0, stlist_buf[index][1]);
      st->set_TS_ID(1, stlist_buf[index][2]);
      st->set_TS_ID(2, stlist_buf[index][3]);
      st->set_TS_ID(3, stlist_buf[index][4]);
      st->set_TS_ID(4, stlist_buf[index][5]);
    }
  }
  for (int i = 0; i < 64; i++) {
    if (st_ec1[i]) N_ST_fwd++;
  }
  for (int i = 0; i < 64; i++) {
    if (st_ec2[i]) N_ST_bwd++;
  }

//-- b2b info with ST0 and phi_i map
  for (int i = 0; i < 36; i++) {
    s2s3 = (ST0_36b[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)])) or s2s3;
    s2s5 = (ST0_36b[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                            or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)])) or s2s5;
    s2so = (ST0_36b[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                            or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)]
                            or ST0_36b[N36(i + 15)] or ST0_36b[N36(i + 21)]
                            or ST0_36b[N36(i + 14)] or ST0_36b[N36(i + 22)]
                            or ST0_36b[N36(i + 13)] or ST0_36b[N36(i + 23)]
                            or ST0_36b[N36(i + 12)] or ST0_36b[N36(i + 24)]
                            or ST0_36b[N36(i + 11)] or ST0_36b[N36(i + 25)]
                            or ST0_36b[N36(i + 10)] or ST0_36b[N36(i + 26)]
                            or ST0_36b[N36(i + 9)] or ST0_36b[N36(i + 27)])) or s2so ;
    s2s30 = (ST0_36b[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                             or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)]
                             or ST0_36b[N36(i + 15)] or ST0_36b[N36(i + 21)]
                             or ST0_36b[N36(i + 14)] or ST0_36b[N36(i + 22)]
                             or ST0_36b[N36(i + 13)] or ST0_36b[N36(i + 23)]
                             or ST0_36b[N36(i + 12)] or ST0_36b[N36(i + 24)]
                             or ST0_36b[N36(i + 11)] or ST0_36b[N36(i + 25)]
                             or ST0_36b[N36(i + 10)] or ST0_36b[N36(i + 26)]
                             or ST0_36b[N36(i + 9)] or ST0_36b[N36(i + 27)]
                             or ST0_36b[N36(i + 8)] or ST0_36b[N36(i + 28)]
                             or ST0_36b[N36(i + 7)] or ST0_36b[N36(i + 29)]
                             or ST0_36b[N36(i + 6)] or ST0_36b[N36(i + 30)]
                             or ST0_36b[N36(i + 5)] or ST0_36b[N36(i + 31)]
                             or ST0_36b[N36(i + 4)] or ST0_36b[N36(i + 32)]
                             or ST0_36b[N36(i + 3)] or ST0_36b[N36(i + 33)])) or s2s30 ;


    s2f3 = (phimap_i[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)])) or s2f3;
    s2f5 = (phimap_i[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                             or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)])) or s2f5;
    s2fo = (phimap_i[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                             or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)]
                             or ST0_36b[N36(i + 15)] or ST0_36b[N36(i + 21)]
                             or ST0_36b[N36(i + 14)] or ST0_36b[N36(i + 22)]
                             or ST0_36b[N36(i + 13)] or ST0_36b[N36(i + 23)]
                             or ST0_36b[N36(i + 12)] or ST0_36b[N36(i + 24)]
                             or ST0_36b[N36(i + 11)] or ST0_36b[N36(i + 25)]
                             or ST0_36b[N36(i + 10)] or ST0_36b[N36(i + 26)]
                             or ST0_36b[N36(i + 9)] or ST0_36b[N36(i + 27)])) or s2fo ;
    s2f30 = (phimap_i[i] and (ST0_36b[N36(i + 18)] or ST0_36b[N36(i + 17)] or ST0_36b[N36(i + 19)]
                              or ST0_36b[N36(i + 16)] or ST0_36b[N36(i + 20)]
                              or ST0_36b[N36(i + 15)] or ST0_36b[N36(i + 21)]
                              or ST0_36b[N36(i + 14)] or ST0_36b[N36(i + 22)]
                              or ST0_36b[N36(i + 13)] or ST0_36b[N36(i + 23)]
                              or ST0_36b[N36(i + 12)] or ST0_36b[N36(i + 24)]
                              or ST0_36b[N36(i + 11)] or ST0_36b[N36(i + 25)]
                              or ST0_36b[N36(i + 10)] or ST0_36b[N36(i + 26)]
                              or ST0_36b[N36(i + 9)] or ST0_36b[N36(i + 27)]
                              or ST0_36b[N36(i + 8)] or ST0_36b[N36(i + 28)]
                              or ST0_36b[N36(i + 7)] or ST0_36b[N36(i + 29)]
                              or ST0_36b[N36(i + 6)] or ST0_36b[N36(i + 30)]
                              or ST0_36b[N36(i + 5)] or ST0_36b[N36(i + 31)]
                              or ST0_36b[N36(i + 4)] or ST0_36b[N36(i + 32)]
                              or ST0_36b[N36(i + 3)] or ST0_36b[N36(i + 33)])) or s2f30 ;

  }

//short-ecl matching at endcap
  for (int i = 0; i < 36; i++) {
    if (ecl_phimap_fwd[i] and st_ec1_36b[i])secl_fwd++;
  }
  for (int i = 0; i < 36; i++) {
    if (ecl_phimap_bwd[i] and st_ec2_36b[i])secl_bwd++;
  }
  secl = secl_fwd + secl_bwd;

//short-klm matching at endcap
  for (int i = 0; i < 4; i++) {
    if (klm_sectormap_fwd[i] and st_ec1_4b[i])sklm_fwd++;
  }
  for (int i = 0; i < 4; i++) {
    if (klm_sectormap_bwd[i] and st_ec2_4b[i])sklm_bwd++;
  }
  sklm = sklm_fwd + sklm_bwd;

//-- set results
  trgInfo->setNshorttrk(N_ST);
  trgInfo->setNshorttrk_fwd(N_ST_fwd);
  trgInfo->setNshorttrk_bwd(N_ST_bwd);
  trgInfo->sets2s3(s2s3);
  trgInfo->sets2s5(s2s5);
  trgInfo->sets2so(s2so);
  trgInfo->sets2s30(s2s30);
  trgInfo->sets2f3(s2f3);
  trgInfo->sets2f5(s2f5);
  trgInfo->sets2fo(s2fo);
  trgInfo->sets2f30(s2f30);
  trgInfo->setbwdsb(0);
  trgInfo->setbwdnb(0);
  trgInfo->setfwdsb(0);
  trgInfo->setfwdnb(0);
  trgInfo->setbrlfb(0);
  trgInfo->setbrlnb(0);
  trgInfo->setNsecl(secl);
  trgInfo->setNsecl_fwd(secl_fwd);
  trgInfo->setNsecl_bwd(secl_bwd);
  trgInfo->setNsklm(sklm);
  trgInfo->setNsklm_fwd(sklm_fwd);
  trgInfo->setNsklm_bwd(sklm_bwd);

  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  st_ec1[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  st_ec2[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  st_ec1_36b[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  st_ec2_36b[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  ecl_phimap_fwd[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  ecl_phimap_bwd[i] << " ";
  //}
  //std::cout << std::endl;
  //std::cout << secl << " " << secl_fwd << " " << secl_bwd << std::endl;

}


void TRGGRLMatchModule::inner_tracking(StoreArray<CDCTriggerSegmentHit> tslist,
                                       std::vector<bool>  phimap_i,
                                       std::vector<bool>  ecl_phimap,
                                       std::vector<bool>  klm_sectormap,
                                       StoreArray<TRGGRLInnerTrack> grlit,
                                       StoreObjPtr<TRGGRLInfo> trgInfo)
{
  std::vector<bool> SL0(64, 0);
  std::vector<bool> SL1(64, 0);
  std::vector<bool> SL2(64, 0);
  std::vector<bool> IT0(64, 0);
  std::vector<bool> IT0_36b(36, 0);
  std::vector<bool> IT0_4b(4, 0);

  //-- collecting TSF info in SL0~2
  for (int i = 0; i < tslist.getEntries(); i++) {
    int id = tslist[i]->getSegmentID();
    int sl = 0;
    if (id >= 0 * 32 && id < 5 * 32) {sl = 0; id -= 0;}
    else if (id >= 5 * 32 && id < 10 * 32) {sl = 1; id -= 5 * 32;}
    else if (id >= 10 * 32 && id < 16 * 32) {sl = 2; id -= 10 * 32;}
    else continue;

    if (sl == 0) {
      int X = (int)(id / 5), Y = id % 5;
      if (Y == 0 || Y == 1) { SL0[2 * X] = true; }
      else if (Y == 3 || Y == 4) { SL0[2 * X + 1] = true; }
      else  { SL0[2 * X] = true; SL0[2 * X + 1] = true; }
    } else if (sl == 1) {
      int X = (int)(id / 5), Y = id % 5;
      if (Y == 0 || Y == 1) { SL1[2 * X] = true; }
      else if (Y == 3 || Y == 4) { SL1[2 * X + 1] = true; }
      else  { SL1[2 * X] = true; SL1[2 * X + 1] = true; }
    } else if (sl == 2) {
      int X = (int)(id / 3);
      SL2[X] = true;
    }
  }


  // -- Inner Track finding with SL0
  for (int i = 0; i < 64; i++) {
    int j1 = i - 4;
    if (j1 < 0) j1 = j1 + 64;
    int j2 = i - 3;
    if (j2 < 0) j2 = j2 + 64;
    int j3 = i - 2;
    if (j3 < 0) j3 = j3 + 64;
    int j4 = i - 1;
    if (j4 < 0) j4 = j4 + 64;
    int j5 = i;
    int j6 = i + 1;
    if (j6 > 63)j6 = j6 - 64;
    int j7 = i + 2;
    if (j7 > 63)j7 = j7 - 64;
    if (
      SL0[i] &&
      (SL1[j1] || SL1[j2] || SL1[j3] || SL1[j4] || SL1[j5]) &&
      (SL2[j3] || SL2[j4] || SL2[j5] || SL2[j6] || SL2[j7])
    ) {
      IT0[i] = true;
    } else {
      IT0[i] = false;
    }
  }

  //-- 64b into 36b
  for (int i = 0; i < 4; i++) {
    IT0_36b[0 + 9 * i] = IT0[0 + 16 * i] or IT0[1 + 16 * i];
    IT0_36b[1 + 9 * i] = IT0[1 + 16 * i] or IT0[2 + 16 * i] or IT0[3 + 16 * i];
    IT0_36b[2 + 9 * i] = IT0[3 + 16 * i] or IT0[4 + 16 * i] or IT0[5 + 16 * i];
    IT0_36b[3 + 9 * i] = IT0[5 + 16 * i] or IT0[6 + 16 * i] or IT0[7 + 16 * i];
    IT0_36b[4 + 9 * i] = IT0[7 + 16 * i] or IT0[8 + 16 * i];
    IT0_36b[5 + 9 * i] = IT0[8 + 16 * i] or IT0[9 + 16 * i] or IT0[10 + 16 * i];
    IT0_36b[6 + 9 * i] = IT0[10 + 16 * i] or IT0[11 + 16 * i] or IT0[12 + 16 * i];
    IT0_36b[7 + 9 * i] = IT0[12 + 16 * i] or IT0[13 + 16 * i] or IT0[14 + 16 * i];
    IT0_36b[8 + 9 * i] = IT0[14 + 16 * i] or IT0[15 + 16 * i];
  }

  //-- 36b into 4b
  IT0_4b[0] = IT0_36b[35] or IT0_36b[0] or IT0_36b[1] or IT0_36b[2] or IT0_36b[3] or IT0_36b[4] or IT0_36b[5] or IT0_36b[6]
              or IT0_36b[7] or IT0_36b[8] or IT0_36b[9];
  IT0_4b[1] = IT0_36b[8]  or IT0_36b[9] or IT0_36b[10] or IT0_36b[11] or IT0_36b[12] or IT0_36b[13] or IT0_36b[14] or IT0_36b[15]
              or IT0_36b[16] or IT0_36b[17] or IT0_36b[18] or IT0_36b[19];
  IT0_4b[2] = IT0_36b[18]  or IT0_36b[19] or IT0_36b[20] or IT0_36b[21] or IT0_36b[22] or IT0_36b[23] or IT0_36b[24] or IT0_36b[25]
              or IT0_36b[26] or IT0_36b[27] or IT0_36b[28];
  IT0_4b[3] = IT0_36b[26]  or IT0_36b[27] or IT0_36b[28] or IT0_36b[29] or IT0_36b[30] or IT0_36b[31] or IT0_36b[32] or IT0_36b[33]
              or IT0_36b[34] or IT0_36b[36] or IT0_36b[0];

  //-- Summary info
  int N_IT = 0;
  bool i2fo = false;
  bool i2io = false;
  int iecl = 0;
  int iklm = 0;

  //-- inner track counting
  for (int i = 0; i < 64; i++) {
    if (IT0[i]) N_IT++;
    TRGGRLInnerTrack* it = grlit.appendNew();
    it->set_TS_ID(0, i);
  }

  //-- b2b info with IT0 and phi_i map
  for (int i = 0; i < 36; i++) {
    i2fo = (phimap_i[i] and (IT0_36b[N36(i + 18)] or IT0_36b[N36(i + 17)] or IT0_36b[N36(i + 19)]
                             or IT0_36b[N36(i + 16)] or IT0_36b[N36(i + 20)]
                             or IT0_36b[N36(i + 15)] or IT0_36b[N36(i + 21)]
                             or IT0_36b[N36(i + 14)] or IT0_36b[N36(i + 22)]
                             or IT0_36b[N36(i + 13)] or IT0_36b[N36(i + 23)]
                             or IT0_36b[N36(i + 12)] or IT0_36b[N36(i + 24)]
                             or IT0_36b[N36(i + 11)] or IT0_36b[N36(i + 25)]
                             or IT0_36b[N36(i + 10)] or IT0_36b[N36(i + 26)]
                             or IT0_36b[N36(i + 9)] or IT0_36b[N36(i + 27)])) or i2fo ;
  }
  //-- b2b info with IT0
  for (int i = 0; i < 36; i++) {
    i2io = (IT0_36b[i] and (IT0_36b[N36(i + 18)] or IT0_36b[N36(i + 17)] or IT0_36b[N36(i + 19)]
                            or IT0_36b[N36(i + 16)] or IT0_36b[N36(i + 20)]
                            or IT0_36b[N36(i + 15)] or IT0_36b[N36(i + 21)]
                            or IT0_36b[N36(i + 14)] or IT0_36b[N36(i + 22)]
                            or IT0_36b[N36(i + 13)] or IT0_36b[N36(i + 23)]
                            or IT0_36b[N36(i + 12)] or IT0_36b[N36(i + 24)]
                            or IT0_36b[N36(i + 11)] or IT0_36b[N36(i + 25)]
                            or IT0_36b[N36(i + 10)] or IT0_36b[N36(i + 26)]
                            or IT0_36b[N36(i + 9)] or IT0_36b[N36(i + 27)])) or i2io ;
  }
  //inner-ecl matching at endcap
  bool IT0_36b_temp[36 * 2];
  for (int i = 0; i < 36; i++) {
    IT0_36b_temp[i]   = IT0_36b[i];
    IT0_36b_temp[i + 36] = IT0_36b[i];
  }
  for (int i = 0; i < 36; i++) {
    if (ecl_phimap[i] and
        (IT0_36b_temp[i + 36] or IT0_36b_temp[i + 36 + 1] or IT0_36b_temp[i + 36 + 2] or IT0_36b_temp[i + 36 + 3]
         or IT0_36b_temp[i + 36 + 4] or
         IT0_36b_temp[i + 36 - 1] or IT0_36b_temp[i + 36 - 2] or IT0_36b_temp[i + 36 - 3] or IT0_36b_temp[i + 36 - 4])
       )iecl++;
  }

  //std::cout << "sector map " ;
  //for (int i = 0; i < 4; i++) {
  // std::cout << " " << i << " " << IT0_4b[i] << " " << klm_sectormap[i];
  //}
  //std::cout << std::endl;

  //inner-klm matching at endcap
  for (int i = 0; i < 4; i++) {
    if (klm_sectormap[i] and IT0_4b[i])iklm++;
  }
  //-- set results
  trgInfo->setNinnertrk(N_IT);
  trgInfo->seti2fo(i2fo);
  trgInfo->seti2io(i2io);
  trgInfo->setNiecl(iecl);
  trgInfo->setNiklm(iklm);

  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  SL0[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  SL1[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  SL2[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 64; i++) {
  //  std::cout <<  IT0[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  IT0_36b[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  phimap_i[i] << " ";
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout <<  ecl_phimap[i] << " ";
  //}
  //std::cout << std::endl;
  //std::cout << i2fo << " " << iecl << std::endl;

}

void TRGGRLMatchModule::matching_eecl_eklm(std::vector<bool>  _eecl_sectormap_fw,
                                           std::vector<bool>  _eecl_sectormap_bw,
                                           std::vector<bool>  _eklm_sectormap_fw,
                                           std::vector<bool>  _eklm_sectormap_bw,
                                           StoreObjPtr<TRGGRLInfo> trgInfo)
{
  int ieclklm = 0;
  for (int i = 0; i < 4; i++) {
    if (_eklm_sectormap_fw[i] && _eecl_sectormap_fw[i])ieclklm++;
    if (_eklm_sectormap_bw[i] && _eecl_sectormap_bw[i])ieclklm++;
  }

  trgInfo->setNeecleklm(ieclklm);

}

void
TRGGRLMatchModule::extrapolation(int pattern, int& l, int& r, int& ec)
{
  if (pattern == 6) {ec = 1; l = 0; r = 1;}
  if (pattern == 7) {ec = 1; l = 0; r = 1;}
  if (pattern == 8) {ec = 1; l = 0; r = 1;}
  if (pattern == 9) {ec = 1; l = 0; r = 2;}
  if (pattern == 10) {ec = 1; l = 0; r = 2;}
  if (pattern == 11) {ec = 1; l = 0; r = 1;}
  if (pattern == 12) {ec = 1; l = 0; r = 2;}
  if (pattern == 18) {ec = 1; l = 0; r = 2;}
  if (pattern == 19) {ec = 1; l = 0; r = 2;}
  if (pattern == 20) {ec = 1; l = 0; r = 4;}
  if (pattern == 21) {ec = 1; l = 0; r = 4;}
  if (pattern == 28) {ec = 1; l = -4; r = 0;}
  if (pattern == 29) {ec = 1; l = -3; r = 0;}
  if (pattern == 34) {ec = 1; l = 0; r = 1;}
  if (pattern == 35) {ec = 1; l = 0; r = 3;}
  if (pattern == 36) {ec = 1; l = 1; r = 3;}
  if (pattern == 37) {ec = 1; l = 0; r = 3;}
  if (pattern == 44) {ec = 1; l = -4; r = 0;}
  if (pattern == 45) {ec = 1; l = -2; r = 0;}
  if (pattern == 46) {ec = 1; l = -3; r = 0;}
  if (pattern == 54) {ec = 1; l = 1; r = 7;}
  if (pattern == 55) {ec = 1; l = 1; r = 6;}
  if (pattern == 56) {ec = 1; l = 1; r = 5;}
  if (pattern == 57) {ec = 1; l = 1; r = 5;}
  if (pattern == 64) {ec = 1; l = -6; r = -1;}
  if (pattern == 73) {ec = 1; l = 3; r = 13;}
  if (pattern == 81) {ec = 1; l = -10; r = -3;}
  if (pattern == 86) {ec = 1; l = 3; r = 12;}
  if (pattern == 87) {ec = 1; l = 3; r = 6;}
  if (pattern == 100) {ec = 1; l = 7; r = 20;}
  if (pattern == 101) {ec = 1; l = 5; r = 20;}
  if (pattern == 102) {ec = 1; l = 5; r = 20;}
  if (pattern == 103) {ec = 1; l = 4; r = 14;}
  if (pattern == 111) {ec = 1; l = -12; r = -5;}
  if (pattern == 112) {ec = 1; l = -18; r = -5;}
  if (pattern == 116) {ec = 1; l = -11; r = -6;}
  if (pattern == 120) {ec = 1; l = 7; r = 21;}
  if (pattern == 121) {ec = 1; l = 7; r = 14;}
  if (pattern == 122) {ec = 1; l = 7; r = 21;}
  if (pattern == 127) {ec = 1; l = -21; r = -8;}
  if (pattern == 128) {ec = 1; l = -15; r = -7;}
  if (pattern == 129) {ec = 1; l = -12; r = -7;}
  if (pattern == 132) {ec = 1; l = 10; r = 18;}
  if (pattern == 133) {ec = 1; l = 8; r = 18;}

  if (pattern == 0) {ec = 2; l = -3; r = 1;}
  if (pattern == 1) {ec = 2; l = -3; r = 1;}
  if (pattern == 3) {ec = 2; l = -3; r = 0;}
  if (pattern == 13) {ec = 2; l = 0; r = 3;}
  if (pattern == 14) {ec = 2; l = 0; r = 4;}
  if (pattern == 15) {ec = 2; l = 0; r = 5;}
  if (pattern == 22) {ec = 2; l = -4; r = -1;}
  if (pattern == 23) {ec = 2; l = -5; r = -1;}
  if (pattern == 24) {ec = 2; l = -3; r = 0;}
  if (pattern == 25) {ec = 2; l = -4; r = 0;}
  if (pattern == 30) {ec = 2; l = 1; r = 5;}
  if (pattern == 39) {ec = 2; l = -2; r = 0;}
  if (pattern == 40) {ec = 2; l = -2; r = 0;}
  if (pattern == 48) {ec = 2; l = 2; r = 6;}
  if (pattern == 49) {ec = 2; l = 3; r = 8;}
  if (pattern == 58) {ec = 2; l = -9; r = -3;}
  if (pattern == 59) {ec = 2; l = -9; r = -3;}
  if (pattern == 67) {ec = 2; l = 5; r = 11;}
  if (pattern == 75) {ec = 2; l = -13; r = -6;}
  if (pattern == 82) {ec = 2; l = 5; r = 9;}
  if (pattern == 83) {ec = 2; l = 5; r = 9;}
  if (pattern == 89) {ec = 2; l = -10; r = -4;}
  if (pattern == 92) {ec = 2; l = -10; r = -4;}
  if (pattern == 97) {ec = 2; l = 7; r = 19;}
  if (pattern == 105) {ec = 2; l = -16; r = -10;}
  if (pattern == 106) {ec = 2; l = -17; r = -7;}
  if (pattern == 109) {ec = 2; l = -17; r = -6;}
  if (pattern == 111) {ec = 2; l = -16; r = -7;}
  if (pattern == 117) {ec = 2; l = 9; r = 19;}
  if (pattern == 118) {ec = 2; l = 9; r = 19;}
  if (pattern == 124) {ec = 2; l = -17; r = -8;}
  if (pattern == 125) {ec = 2; l = -17; r = -8;}
  if (pattern == 126) {ec = 2; l = -17; r = -8;}

}


