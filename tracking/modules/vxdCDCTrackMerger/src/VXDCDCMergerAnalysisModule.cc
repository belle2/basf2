#include <tracking/modules/vxdCDCTrackMerger/VXDCDCMergerAnalysisModule.h>
#include <tracking/modules/vxdCDCTrackMerger/VXDCDCTrackMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

#include <iostream>

using namespace Belle2;

REG_MODULE(VXDCDCMergerAnalysis)

VXDCDCMergerAnalysisModule::VXDCDCMergerAnalysisModule() : Module(),
  m_CDC_wall_radius(16.25),
  m_ttree(0),
  m_root_file(0),
  m_total_pairs(0),
  m_total_matched_pairs(0),
//m_nevent(0),
  m_npair(0),
  m_ntruepair(0),
  m_ncdc_trk(0),
  m_nvxd_trk(0),
//m_trk_mrg_eff(0),
  m_match_vec(0),
  m_true_match_vec(0),
  m_true_match_mc(0),
  m_right_match_vec(0),
  m_reco_vec(0),
//m_loop_match_vec(0),
  m_chi2_vec(0),
  m_dist_vec(0),
  m_dx_vec(0),
  m_dy_vec(0),
  m_dz_vec(0),
  m_x_vec(0),
  m_y_vec(0),
  m_z_vec(0),
  m_pos_vec(0),
  m_dmom_vec(0),
  m_dmomx_vec(0),
  m_dmomy_vec(0),
  m_dmomz_vec(0),
  m_momx_vec(0),
  m_momy_vec(0),
  m_momz_vec(0),
  m_mom_vec(0),
  m_vxdmomx_vec(0),
  m_vxdmomy_vec(0),
  m_vxdmomz_vec(0),
  m_vxdmom_vec(0)
{
  setDescription("Analysis module for VXDCDCTrackMerger. This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC and creates a root file.");

  //input tracks and candidates
  //  addParam("GFTracksColName",  m_GFTracksColName,  "GFTrack collection (from GenFit)");
  addParam("VXDGFTracksColName",  m_VXDGFTracksColName,  "VXDl GFTrack collection (from GFTrackSplitter)");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection (from GFTrackSplitter)");
  addParam("VXDGFTrackCandsColName",  m_VXDGFTrackCandsColName,  "VXD GFTrackCand collection (from GFTrackSplitter)");
  addParam("CDCGFTrackCandsColName", m_CDCGFTrackCandsColName, "CDC GFTrackCand collection (from GFTrackSplitter)");
  //  addParam("TrackCandColName", m_TrackCandColName, "Track Cand collection (from TrackFinder)");
  //  addParam("UnMergedCands", m_UnMergedCands, "Merged cands Collection for EvtDisplay");

  //Chi2 Cut
  addParam("root_output_filename", m_root_output_filename, "ROOT file for tracks merger analysis",
           std::string("VXD_CDC_trackmerger.root"));
}


VXDCDCMergerAnalysisModule::~VXDCDCMergerAnalysisModule()
{
}

void VXDCDCMergerAnalysisModule::initialize()
{

  //StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  //StoreArray<genfit::Track>::required(m_GFTracksColName);
  //StoreArray<genfit::TrackCand>::registerPersistent(m_UnMergedCands);
  StoreArray<genfit::Track>::required(m_VXDGFTracksColName);
  StoreArray<genfit::Track>::required(m_CDCGFTracksColName);
  StoreArray<genfit::TrackCand>::required(m_VXDGFTrackCandsColName);
  StoreArray<genfit::TrackCand>::required(m_CDCGFTrackCandsColName);

  //m_VXDGFTrackCandsColName.requiredRelationTo(m_VXDGFTracksColName);
  //m_CDCGFTrackCandsColName.requiredRelationTo(m_CDCGFTracksColName);
  //StoreArray<MCParticle>

  //m_CDC_wall_radius = 16.25;
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
  //nEv=0;

  //root tree variables
  m_root_file = new TFile(m_root_output_filename.c_str(), "RECREATE");
  m_ttree     = new TTree("Stats", "Matched Tracks");

  m_true_match_mc = new std::vector<int>();
  m_true_match_vec = new std::vector<int>();
  m_match_vec = new std::vector<int>();
  m_right_match_vec = new std::vector<int>();
  m_reco_vec = new std::vector<int>();
  //m_loop_match_vec = new std::vector<int>();
  m_chi2_vec = new std::vector<float>();
  m_dist_vec = new std::vector<float>();
  m_dx_vec   = new std::vector<float>();
  m_dy_vec   = new std::vector<float>();
  m_dz_vec   = new std::vector<float>();
  m_pos_vec   = new std::vector<float>();
  m_x_vec   = new std::vector<float>();
  m_y_vec   = new std::vector<float>();
  m_z_vec   = new std::vector<float>();
  m_dmom_vec = new std::vector<float>();
  m_dmomx_vec   = new std::vector<float>();
  m_dmomy_vec   = new std::vector<float>();
  m_dmomz_vec   = new std::vector<float>();
  m_mom_vec   = new std::vector<float>();
  m_momx_vec   = new std::vector<float>();
  m_momy_vec   = new std::vector<float>();
  m_momz_vec   = new std::vector<float>();
  m_vxdmom_vec   = new std::vector<float>();
  m_vxdmomx_vec   = new std::vector<float>();
  m_vxdmomy_vec   = new std::vector<float>();
  m_vxdmomz_vec   = new std::vector<float>();

  m_ttree->Branch("MergedTracks",     &m_npair,                  "npair/I");
  m_ttree->Branch("TrueMatchedTracks",     &m_ntruepair,                  "ntruepair/I");
  m_ttree->Branch("CDCTracks",  &m_ncdc_trk,               "ncdc_trk/I");
  m_ttree->Branch("VXDTracks",   &m_nvxd_trk,                "nvxd_trk/I");
  m_ttree->Branch("MergedTag",     "std::vector<int>",        &m_match_vec);
  m_ttree->Branch("TruthTag",     "std::vector<int>",        &m_true_match_vec);
  m_ttree->Branch("GoodTag",     "std::vector<int>",        &m_right_match_vec);
  m_ttree->Branch("RecoTag",     "std::vector<int>",        &m_reco_vec);
  //m_ttree->Branch("LoopTag",     "std::vector<int>",        &m_loop_match_vec);
  m_ttree->Branch("Chi2",      "std::vector<float>",      &m_chi2_vec);
  m_ttree->Branch("PosRes",      "std::vector<float>",      &m_dist_vec);
  m_ttree->Branch("Dx",        "std::vector<float>",      &m_dx_vec);
  m_ttree->Branch("Dy",        "std::vector<float>",      &m_dy_vec);
  m_ttree->Branch("Dz",        "std::vector<float>",      &m_dz_vec);
  m_ttree->Branch("x",        "std::vector<float>",      &m_x_vec);
  m_ttree->Branch("y",        "std::vector<float>",      &m_y_vec);
  m_ttree->Branch("z",        "std::vector<float>",      &m_z_vec);
  m_ttree->Branch("Pos",        "std::vector<float>",      &m_pos_vec);
  m_ttree->Branch("MomRes",      "std::vector<float>",      &m_dmom_vec);
  m_ttree->Branch("DMomx",        "std::vector<float>",      &m_dmomx_vec);
  m_ttree->Branch("DMomy",        "std::vector<float>",      &m_dmomy_vec);
  m_ttree->Branch("DMomz",        "std::vector<float>",      &m_dmomz_vec);
  m_ttree->Branch("P",        "std::vector<float>",      &m_mom_vec);
  m_ttree->Branch("Px",        "std::vector<float>",      &m_momx_vec);
  m_ttree->Branch("Py",        "std::vector<float>",      &m_momy_vec);
  m_ttree->Branch("Pz",        "std::vector<float>",      &m_momz_vec);
  m_ttree->Branch("vxdP",        "std::vector<float>",      &m_vxdmom_vec);
  m_ttree->Branch("vxdPx",        "std::vector<float>",      &m_vxdmomx_vec);
  m_ttree->Branch("vxdPy",        "std::vector<float>",      &m_vxdmomy_vec);
  m_ttree->Branch("vxdPz",        "std::vector<float>",      &m_vxdmomz_vec);
}

void VXDCDCMergerAnalysisModule::beginRun()
{
}

void VXDCDCMergerAnalysisModule::event()
{

  int n_trk_pair = 0;
  int n_trk_truth_pair = 0;

  m_true_match_vec->clear();
  m_true_match_mc->clear();
  m_right_match_vec->clear();
  m_reco_vec->clear();
  m_match_vec->clear();
  //m_loop_match_vec->clear();
  m_chi2_vec->clear();
  m_dist_vec->clear();
  m_dx_vec->clear();
  m_dy_vec->clear();
  m_dz_vec->clear();
  m_x_vec->clear();
  m_y_vec->clear();
  m_z_vec->clear();
  m_pos_vec->clear();
  m_dmom_vec->clear();
  m_dmomx_vec->clear();
  m_dmomy_vec->clear();
  m_dmomz_vec->clear();
  m_momx_vec->clear();
  m_momy_vec->clear();
  m_momz_vec->clear();
  m_mom_vec->clear();
  m_vxdmomx_vec->clear();
  m_vxdmomy_vec->clear();
  m_vxdmomz_vec->clear();
  m_vxdmom_vec->clear();

  //GET TRACKS;
  //get CDC tracks
  StoreArray<genfit::Track> CDCGFTracks(m_CDCGFTracksColName);
  unsigned int nCDCTracks = CDCGFTracks.getEntries();
  B2INFO("VXDCDCMerger: input Number of CDC Tracks: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("VXDCDCMerger: CDCGFTracksCollection is empty!");

  //get VXD tracks
  StoreArray<genfit::Track> VXDGFTracks(m_VXDGFTracksColName);
  unsigned int nVXDTracks = VXDGFTracks.getEntries();
  B2INFO("VXDCDCMerger: input Number of Silicon Tracks: " << nVXDTracks);
  if (nVXDTracks == 0) B2WARNING("VXDCDCMerger: VXDGFTracksCollection is empty!");

  StoreArray<genfit::TrackCand> VXDGFTrackCands(m_VXDGFTrackCandsColName);
  StoreArray<genfit::TrackCand> CDCGFTrackCands(m_CDCGFTrackCandsColName);

  //StoreArray<genfit::Track> GFTracks(m_GFTracksColName);
  //StoreArray<genfit::TrackCand> UnMergedCands(m_UnMergedCands);
  //const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);

  //StoreArray<MCParticle> mcParticles;
  //RelationArray ECLClusterToMC(clusters, mcParticles);
  RelationArray CDCToVXDTracks(CDCGFTracks, VXDGFTracks);;
  //RelationArray MCToVXDTrackCands(mcParticles, VXDGFTrackCands);;
  //RelationArray MCToCDCTrackCands(mcParticles, CDCGFTrackCands);;

  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  TVectorD vxd_trk_state;
  TVectorD cdc_trk_state;
  TMatrixDSym vxd_trk_covmtrx(6);
  TMatrixDSym cdc_trk_covmtrx(6);
  TVector3 pos(-1000., -1000., -1000.);
  TVector3 mom(-10., -10., -10.);
  TVector3 vxdpos(-1000., -1000., -1000.);
  TVector3 vxdmom(-10., -10., -10.);
  TVector3 vxd_xmin(-1000., -1000., -1000.);
  TVector3 vxd_pmin(-10., -10., -10.);
  //double merge_radius = m_merge_radius;

  unsigned int matched_track = 0;
  unsigned int recovered_track = 0;
  unsigned int truth_matched = 0;
  unsigned int truth_flag = 0;
  unsigned int vxd_match = 1000;
  unsigned int vxd_track = 1000;
  unsigned int vxd_truth = 2000;
  unsigned int cdc_match = 1001;
  //unsigned int cdc_truth = 2001;
  int vxd_mcp_index = 3000;
  int cdc_mcp_index = 4000;
  int match_weight = 0;
  float CHI2_MAX = 0;

  //LOOP ON TRACKS//
  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) { //extrapolate to the CDC wall from first hit
    cdc_mcp_index = 4000;
    truth_flag = 0;
    double chi_2 = m_chi2_max;
    //double CHI2_MAX = m_chi2_max;
    try {
      genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      pos = cdc_sop.getPos();
      mom = cdc_sop.getMom();
    } catch (...) {
      //B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }
    matched_track = 0;
    recovered_track = 0;
    truth_matched = 0;
    vxd_match = 1000; //index for matched track candidate
    vxd_track = 1000;
    vxd_truth = 2000; //index for true matched candidate
    cdc_match = 1001;
    //cdc_truth = 2001;

    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nVXDTracks; jtrack++) {
      vxd_mcp_index = 3000;
      int ex_ok = 0;
      try {
        genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
        genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
        cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        //vxd_xmin = vxd_sop.getPos();
        //vxd_pmin = vxd_sop.getMom();
        ex_ok = 1;
      } catch (...) {
        //B2WARNING("VXDTrack extrapolation to cylinder failed!");
        continue;
      }
      if (ex_ok == 1) {
        try {
          genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
          vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxdpos = vxd_sop.getPos();
          vxdmom = vxd_sop.getMom();
          genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
          state_diff *= inv_covmtrx;
          chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());
          vxd_xmin = vxd_sop.getPos();
          vxd_pmin = vxd_sop.getMom();
        } catch (...) {
          B2WARNING("Matrix is singular!");
          continue;
        }
      }

      //Check if tracks were matched or recovered
      for (int itr = 0; itr < CDCToVXDTracks.getEntries(); itr++) {
        cdc_match = CDCToVXDTracks[itr].getFromIndex();
        vxd_track = CDCToVXDTracks[itr].getToIndex();
        if ((cdc_match == itrack) && (vxd_track == jtrack)) {
          match_weight = CDCToVXDTracks[itr].getWeight();
          vxd_match = vxd_track;
          if (match_weight == 1) {
            matched_track = 1;
          }
          if (match_weight == 2) {
            recovered_track = 1;
          }
        }
      }

      //RECOVER MC INFO
      const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(CDCGFTracks[itrack],
                                                m_CDCGFTrackCandsColName);
      if (cdc_TrkCandPtr == NULL) {
        //std::cout << "No CDC cand" << std::endl;
        continue;
      }
      cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();
      std::cout << cdc_mcp_index << std::endl;

      const genfit::TrackCand* vxd_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(VXDGFTracks[jtrack],
                                                m_VXDGFTrackCandsColName);
      if (vxd_TrkCandPtr == NULL) {
        //std::cout << "No VXD cand" << std::endl;
        continue;
      }
      vxd_mcp_index = vxd_TrkCandPtr->getMcTrackId();

      for (std::vector<int>::iterator it = m_true_match_mc->begin(); it != m_true_match_mc->end(); ++it) {
        if ((*it) == cdc_mcp_index) {
          truth_flag = 1;
        }
      }

      if ((cdc_mcp_index == vxd_mcp_index) && truth_flag == 0) { //Calculate for true matched tracks
        m_true_match_vec->push_back(1);
        m_true_match_mc->push_back(cdc_mcp_index);
        truth_matched = 1;
        vxd_truth = jtrack;
        n_trk_truth_pair++;
        if (matched_track == 0) {
          try {
            genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
            genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
            cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
            vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
          } catch (...) {
            B2WARNING("VXDTrack extrapolation to plane failed!");
            continue;
          }
          genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
          genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
          try {
            TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          } catch (...) {
            B2WARNING("Matrix singular!");
            continue;
          }
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
          state_diff *= inv_covmtrx;
          chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());
          CHI2_MAX = chi_2;
          vxd_xmin = vxd_sop.getPos();
          vxd_pmin = vxd_sop.getMom();
        }
        if ((truth_matched == 0) && (cdc_mcp_index != vxd_mcp_index) && (jtrack == (nVXDTracks - 1))) {
          m_true_match_vec->push_back(0);
        }
      }
    }//end loop on VXD tracks

    //Store Indexes
    if ((matched_track == 1) || (recovered_track == 1)) {
      n_trk_pair++;
      m_match_vec->push_back(1);
      if (matched_track == 1) {
        m_reco_vec->push_back(0);
        if (vxd_match == vxd_truth)
          m_right_match_vec->push_back(1);
        else
          m_right_match_vec->push_back(0);
      }
      if (recovered_track == 1) {
        m_reco_vec->push_back(1);
        if (vxd_match == vxd_truth)
          m_right_match_vec->push_back(1);
        else
          m_right_match_vec->push_back(0);
      }
    }

    if (((matched_track == 0) && (recovered_track == 0)) && (truth_matched == 1)) {
      m_match_vec->push_back(0);
      m_reco_vec->push_back(0);
      m_right_match_vec->push_back(0);
    }

    if (((matched_track == 1) || (recovered_track == 1)) && (truth_matched == 0)) {
      m_true_match_vec->push_back(0);
    }

    if ((matched_track == 1) || (recovered_track == 1) || (truth_matched == 1)) { //Save info for merged tracks or a true-matcheds
      //Chi2
      m_chi2_vec->push_back(CHI2_MAX);
      //Residuals
      m_dx_vec->push_back(pos.X() - vxd_xmin.X());
      m_dy_vec->push_back(pos.Y() - vxd_xmin.Y());
      m_dz_vec->push_back(pos.Z() - vxd_xmin.Z());
      m_dist_vec->push_back(TMath::Sqrt((pos - vxd_xmin) * (pos - vxd_xmin)));
      m_dmomx_vec->push_back(mom.X() - vxd_pmin.X());
      m_dmomy_vec->push_back(mom.Y() - vxd_pmin.Y());
      m_dmomz_vec->push_back(mom.Z() - vxd_pmin.Z());
      m_dmom_vec->push_back(TMath::Sqrt((mom - vxd_pmin) * (mom - vxd_pmin)));
      //VXD info
      m_vxdmomx_vec->push_back(vxd_pmin.X());
      m_vxdmomy_vec->push_back(vxd_pmin.Y());
      m_vxdmomz_vec->push_back(vxd_pmin.Z());
      m_vxdmom_vec->push_back(vxd_pmin.Mag());
      //CDC info
      m_x_vec->push_back(pos.X());
      m_y_vec->push_back(pos.Y());
      m_z_vec->push_back(pos.Z());
      m_pos_vec->push_back(pos.Mag());
      m_momx_vec->push_back(mom.X());
      m_momy_vec->push_back(mom.Y());
      m_momz_vec->push_back(mom.Z());
      m_mom_vec->push_back(mom.Mag());
    }

  }//loop on CDC

  m_nvxd_trk = nVXDTracks;
  m_ncdc_trk = nCDCTracks;
  m_npair = n_trk_pair;
  m_ntruepair = n_trk_truth_pair;
  m_ttree->Fill();
}

void VXDCDCMergerAnalysisModule::endRun()
{
}

void VXDCDCMergerAnalysisModule::terminate()
{
  m_root_file->cd();
  m_ttree->Write();
  m_root_file->Close();
}
