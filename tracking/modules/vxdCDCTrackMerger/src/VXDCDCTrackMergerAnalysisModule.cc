#include <tracking/modules/vxdCDCTrackMerger/inc/VXDCDCTrackMergerAnalysisModule.h>
#include <tracking/modules/vxdCDCTrackMerger/inc/VXDCDCTrackMergerModule.h>
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

REG_MODULE(VXDCDCTrackMergerAnalysis)

VXDCDCTrackMergerAnalysisModule::VXDCDCTrackMergerAnalysisModule() : Module()
{
  setDescription("Analysis module for VXDCDCTrackMerger. This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC and creates a root file.");

  //input tracks and candidates
  addParam("GFTracksColName",  m_GFTracksColName,  "GFTrack collection (from GenFit)");
  addParam("VXDGFTracksColName",  m_VXDGFTracksColName,  "VXDl GFTrack collection (from GFTrackSplitter)");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection (from GFTrackSplitter)");
  addParam("TrackCandColName", m_TrackCandColName, "Track Cand collection (from TrackFinder)");
  addParam("UnMergedCands", m_UnMergedCands, "Merged cands Collection for EvtDisplay");

  //Chi2 Cut
  addParam("chi2_max", m_chi2_max, "Chi^2 cut for matching", double(100.0));
  addParam("merge_radius", m_merge_radius, "Maximum distance between extrapolated tracks on the CDC wall", double(2.0));

  addParam("root_output_filename", m_root_output_filename, "ROOT file for tracks merger analysis", std::string("VXD_CDC_trackmerger.root"));
}


VXDCDCTrackMergerAnalysisModule::~VXDCDCTrackMergerAnalysisModule()
{
}


void VXDCDCTrackMergerAnalysisModule::initialize()
{

  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track>::required(m_GFTracksColName);
  StoreArray<genfit::TrackCand>::registerPersistent(m_UnMergedCands);

  m_CDC_wall_radius = 16.25;
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
  //nEv=0;

  //root tree variables
  m_root_file = new TFile(m_root_output_filename.c_str(), "RECREATE");
  m_ttree     = new TTree("Stats", "Matched Tracks");

  m_true_match_vec = new std::vector<int>();
  m_match_vec = new std::vector<int>();
  m_right_match_vec = new std::vector<int>();
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

void VXDCDCTrackMergerAnalysisModule::beginRun()
{
}

void VXDCDCTrackMergerAnalysisModule::event()
{

  int n_trk_pair = 0;
  int n_trk_truth_pair = 0;

  m_true_match_vec->clear();
  m_right_match_vec->clear();
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
  //B2INFO("VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  //if (nCDCTracks == 0) B2WARNING("VXDCDCTrackMerger: CDCGFTracksCollection is empty!");

  //get VXD tracks
  StoreArray<genfit::Track> VXDGFTracks(m_VXDGFTracksColName);
  unsigned int nVXDTracks = VXDGFTracks.getEntries();
  //B2INFO("VXDCDCTrackMerger: input Number of Silicon Tracks: " << nVXDTracks);
  //if (nVXDTracks == 0) B2WARNING("VXDCDCTrackMerger: VXDGFTracksCollection is empty!");

  StoreArray<genfit::Track> GFTracks(m_GFTracksColName);
  StoreArray<genfit::TrackCand> UnMergedCands(m_UnMergedCands);
  const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);

  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  TVectorD vxd_trk_state;
  TVectorD cdc_trk_state;
  TMatrixDSym vxd_trk_covmtrx(6);
  TMatrixDSym cdc_trk_covmtrx(6);
  TVector3 pos;
  TVector3 mom;
  TVector3 vxdpos;
  TVector3 vxdmom;
  TVector3 vxd_xmin;
  TVector3 vxd_pmin;
  double merge_radius = m_merge_radius;

  unsigned int matched_track = 0;
  unsigned int truth_matched = 0;
  unsigned int vxd_match = 1000;
  unsigned int vxd_truth = 2000;
  int vxd_mcp_index = 3000;
  int cdc_mcp_index = 4000;


  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) { //extrapolate to the CDC wall from first hit
    cdc_mcp_index = 4000;
    double chi_2 = m_chi2_max;
    double CHI2_MAX = m_chi2_max;
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
    truth_matched = 0;
    vxd_match = 1000; //index for matched track candidate
    vxd_truth = 2000; //index for true matched candidate
    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nVXDTracks; jtrack++) {
      vxd_mcp_index = 3000;
      try {
        genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (...) {
        //B2WARNING("VXDTrack extrapolation to cylinder failed!");
        continue;
      }

      // Extrapolate tracks to same plane & Match Tracks
      if (TMath::Sqrt((pos - vxdpos) * (pos - vxdpos)) < merge_radius) {
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
          B2WARNING("Matrix is singular!");
          continue;
        }
        TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
        TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
        state_diff *= inv_covmtrx;
        chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());

        if ((chi_2 < CHI2_MAX) && (chi_2 > 0)) {
          matched_track = 1;
          CHI2_MAX = chi_2;
          vxd_match = jtrack;
          vxd_xmin = vxd_sop.getPos();
          vxd_pmin = vxd_sop.getMom();
        }
      }//If on radius

      //Recover track candidate index
      genfit::Track* GFTrk = GFTracks[itrack];
      const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (cdc_TrkCandPtr == NULL) {
        continue;
      }
      cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();
      GFTrk = GFTracks[jtrack + nCDCTracks];
      const genfit::TrackCand* vxd_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (vxd_TrkCandPtr == NULL) {
        continue;
      }
      vxd_mcp_index = vxd_TrkCandPtr->getMcTrackId();

      if (cdc_mcp_index == vxd_mcp_index) { //Calculate for true matched tracks
        m_true_match_vec->push_back(1);
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
    if (matched_track == 1) {
      n_trk_pair++;
      m_match_vec->push_back(1);
      std::cout << "Merged candidate id: " << cdc_mcp_index << std::endl; //Temp for display purposes
      if (vxd_match == vxd_truth)
        m_right_match_vec->push_back(1);
      else
        m_right_match_vec->push_back(0);
    }
    if ((matched_track == 0) && (truth_matched == 1)) {
      m_match_vec->push_back(0);
      m_right_match_vec->push_back(0);
      std::cout << "Unmerged candidate id: " << cdc_mcp_index << std::endl; //Temp for display purposes
      //m_true_match_vec->push_back(1);
      const genfit::TrackCand* UnMergedTrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>((GFTracks[itrack]), m_TrackCandColName);
      UnMergedCands.appendNew(*UnMergedTrkCandPtr);
    }
    if ((matched_track == 1) && (truth_matched == 0)) {
      m_true_match_vec->push_back(0);
    }

    if ((matched_track == 1) || (truth_matched == 1)) { //Save info for merged tracks or a true-matcheds
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

void VXDCDCTrackMergerAnalysisModule::endRun()
{
}

void VXDCDCTrackMergerAnalysisModule::terminate()
{
  m_root_file->cd();
  m_ttree->Write();
  m_root_file->Close();
}
