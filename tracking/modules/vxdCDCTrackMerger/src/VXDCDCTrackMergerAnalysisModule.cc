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

  //Chi2 Cut
  addParam("chi2_max", m_chi2_max, "Chi^2 cut for matching", double(100.0));

  addParam("root_output_filename", m_root_output_filename, "ROOT file for tracks merger analysis", std::string("VXD_CDC_trackmerger.root"));
}


VXDCDCTrackMergerAnalysisModule::~VXDCDCTrackMergerAnalysisModule()
{
}


void VXDCDCTrackMergerAnalysisModule::initialize()
{

  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track>::required(m_GFTracksColName);

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
  m_VXDmom_vec   = new std::vector<float>();
  m_VXDmomx_vec   = new std::vector<float>();
  m_VXDmomy_vec   = new std::vector<float>();
  m_VXDmomz_vec   = new std::vector<float>();

  m_ttree->Branch("MergedTracks",     &m_npair,                  "npair/I");
  m_ttree->Branch("TrueMatchedTracks",     &m_ntruepair,                  "ntruepair/I");
  m_ttree->Branch("CDCTracks",  &m_ncdc_trk,               "ncdc_trk/I");
  m_ttree->Branch("VXDTracks",   &m_nVXD_trk,                "nVXD_trk/I");
  m_ttree->Branch("MergedTag",     "std::vector<int>",        &m_match_vec);
  m_ttree->Branch("TruthTag",     "std::vector<int>",        &m_true_match_vec);
  m_ttree->Branch("GoodTag",     "std::vector<int>",        &m_right_match_vec);
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
  m_ttree->Branch("VXDP",        "std::vector<float>",      &m_VXDmom_vec);
  m_ttree->Branch("VXDPx",        "std::vector<float>",      &m_VXDmomx_vec);
  m_ttree->Branch("VXDPy",        "std::vector<float>",      &m_VXDmomy_vec);
  m_ttree->Branch("VXDPz",        "std::vector<float>",      &m_VXDmomz_vec);
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
  m_VXDmomx_vec->clear();
  m_VXDmomy_vec->clear();
  m_VXDmomz_vec->clear();
  m_VXDmom_vec->clear();

  //GET TRACKS;
  //get CDC tracks
  StoreArray<genfit::Track> CDCGFTracks(m_CDCGFTracksColName);
  unsigned int nCDCTracks = CDCGFTracks.getEntries();
  B2INFO("VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("VXDCDCTrackMerger: CDCGFTracksCollection is empty!");

  //get VXD tracks
  StoreArray<genfit::Track> VXDGFTracks(m_VXDGFTracksColName);
  unsigned int nVXDTracks = VXDGFTracks.getEntries();
  B2INFO("VXDCDCTrackMerger: input Number of Silicon Tracks: " << nVXDTracks);
  if (nVXDTracks == 0) B2WARNING("VXDCDCTrackMerger: VXDGFTracksCollection is empty!");

  StoreArray<genfit::Track> GFTracks(m_GFTracksColName);
  //unsigned int nTracks = GFTracks.getEntries();

  const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);

  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  TVectorD VXD_trk_state;
  TMatrixDSym VXD_trk_covmtrx(6);
  TVectorD cdc_trk_state;
  TMatrixDSym cdc_trk_covmtrx(6);
  TVector3 pos;
  TVector3 mom;
  TVector3 VXDpos;
  TVector3 VXDmom;
  double chi_2 = 100;

  unsigned int matched_track = 0;
  unsigned int truth_matched = 0;
  unsigned int vxd_match = 1000;
  unsigned int vxd_truth = 2000;

  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) {
    //extrapolate to the CDC wall
    try {
      genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdc_sop.get6DStateCov(cdc_trk_state, cdc_trk_covmtrx);
      pos = cdc_sop.getPos();
      mom = cdc_sop.getMom();
      m_x_vec->push_back(pos.X());
      m_y_vec->push_back(pos.Y());
      m_z_vec->push_back(pos.Z());
      m_pos_vec->push_back(pos.Mag());
      m_momx_vec->push_back(mom.X());
      m_momy_vec->push_back(mom.Y());
      m_momz_vec->push_back(mom.Z());
      m_mom_vec->push_back(mom.Mag());
    } catch (...) {
      B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    double CHI2_MAX = m_chi2_max;
    double CHI2_MIN = 1000000000000;
    matched_track = 0;
    truth_matched = 0;
    vxd_match = 1000;
    vxd_truth = 2000;
    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nVXDTracks; jtrack++) {
      try {
        genfit::MeasuredStateOnPlane VXD_sop = VXDGFTracks[jtrack]->getFittedState();
        VXD_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        VXD_sop.get6DStateCov(VXD_trk_state, VXD_trk_covmtrx);
      } catch (...) {
        B2WARNING("VXDTrack extrapolation to cylinder failed!");
        continue;
      }

      //Match Tracks
      TMatrixDSym inv_covmtrx = (cdc_trk_covmtrx + VXD_trk_covmtrx).Invert();
      TVectorD state_diff = cdc_trk_state - VXD_trk_state;
      state_diff *= inv_covmtrx;
      chi_2 = state_diff * (cdc_trk_state - VXD_trk_state);
      if ((chi_2 < CHI2_MAX) && (chi_2 > 0)) {
        matched_track = 1;
        CHI2_MAX = chi_2;
        vxd_match = jtrack;
        genfit::MeasuredStateOnPlane VXD_sop = VXDGFTracks[jtrack]->getFittedState();
        VXD_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        VXD_sop.get6DStateCov(VXD_trk_state, VXD_trk_covmtrx);
        VXDpos = VXD_sop.getPos();
        VXDmom = VXD_sop.getMom();
      }
      if ((chi_2 > CHI2_MAX) && (chi_2 < CHI2_MIN) && (matched_track == 0)) {
        CHI2_MIN = chi_2;
        genfit::MeasuredStateOnPlane VXD_sop = VXDGFTracks[jtrack]->getFittedState();
        VXD_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        VXD_sop.get6DStateCov(VXD_trk_state, VXD_trk_covmtrx);
        VXDpos = VXD_sop.getPos();
        VXDmom = VXD_sop.getMom();
      }

      //Recover track candidate index
      genfit::Track* GFTrk = GFTracks[itrack];
      const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (cdc_TrkCandPtr == NULL) {
        continue;
      }
      int cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();
      GFTrk = GFTracks[jtrack + nCDCTracks];
      const genfit::TrackCand* VXD_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (VXD_TrkCandPtr == NULL) {
        continue;
      }
      int VXD_mcp_index = VXD_TrkCandPtr->getMcTrackId();
      if (cdc_mcp_index == VXD_mcp_index) {
        m_true_match_vec->push_back(1);
        truth_matched = 1;
        vxd_truth = jtrack;
        n_trk_truth_pair++;
      }
      if ((truth_matched == 0) && (cdc_mcp_index != VXD_mcp_index) && (jtrack == (nVXDTracks - 1))) {
        m_true_match_vec->push_back(0);
      }
    }//loop on VXD tracks

    //Merged/Unmerged Index
    if (matched_track == 1) {
      n_trk_pair++;
      m_match_vec->push_back(1);
      m_chi2_vec->push_back(CHI2_MAX);
    } else {
      m_match_vec->push_back(0);
      m_chi2_vec->push_back(CHI2_MIN);
    }
    if (vxd_match == vxd_truth) {
      m_right_match_vec->push_back(1);
    } else
      m_right_match_vec->push_back(0);

    m_dx_vec->push_back(pos.X() - VXDpos.X());
    m_dy_vec->push_back(pos.Y() - VXDpos.Y());
    m_dz_vec->push_back(pos.Z() - VXDpos.Z());
    m_dist_vec->push_back(TMath::Sqrt((pos.X() - VXDpos.X()) * (pos.X() - VXDpos.X()) + (pos.Y() - VXDpos.Y()) * (pos.Y() - VXDpos.Y()) + (pos.Z() - VXDpos.Z()) * (pos.Z() - VXDpos.Z())));
    m_dmomx_vec->push_back(mom.X() - VXDmom.X());
    m_dmomy_vec->push_back(mom.Y() - VXDmom.Y());
    m_dmomz_vec->push_back(mom.Z() - VXDmom.Z());
    m_dmom_vec->push_back(TMath::Sqrt((mom.X() - VXDmom.X()) * (mom.X() - VXDmom.X()) + (mom.Y() - VXDmom.Y()) * (mom.Y() - VXDmom.Y()) + (mom.Z() - VXDmom.Z()) * (mom.Z() - VXDmom.Z())));
    m_VXDmomx_vec->push_back(VXDmom.X());
    m_VXDmomy_vec->push_back(VXDmom.Y());
    m_VXDmomz_vec->push_back(VXDmom.Z());
    m_VXDmom_vec->push_back(VXDmom.Mag());

  }//loop on CDC

  m_nVXD_trk = nVXDTracks;
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
