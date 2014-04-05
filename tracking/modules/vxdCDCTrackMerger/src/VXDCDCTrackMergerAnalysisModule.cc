#include <tracking/modules/vxdCDCTrackMerger/inc/VXDCDCTrackMergerAnalysisModule.h>
#include <tracking/modules/vxdCDCTrackMerger/inc/VXDCDCTrackMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"
//#include "genfit/AbsTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(VXDCDCTrackMergerAnalysis)

VXDCDCTrackMergerAnalysisModule::VXDCDCTrackMergerAnalysisModule() : Module()
{
  setDescription("Analysis module for VXDCDCTrackMerger. This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC and creates a root file.");
  addParam("CDC_wall_radius",    m_CDC_wall_radius,    "This parameters stands for either the inner or the outer CDC wall radius", double(100.0));
  //input tracks
  addParam("GFTracksColName",  m_GFTracksColName,  "Originary GFTrack collection");
  addParam("SiGFTracksColName",  m_SiGFTracksColName,  "Silicon GFTrack collection");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection");
  //addParam("MCParticlesColName", m_mcParticlesColName, "MCParticles collection");
  addParam("TrackCandColName", m_TrackCandColName, "CDC Track Cand collection");

  //root stuff
  //addParam("produce_root_file", m_produce_root_file, "if yes, generate a local root file", bool(false));
  addParam("root_output_filename", m_root_output_filename, "ROOT file for tracks merger analysis", std::string("VXD_CDC_trackmerger.root"));
}


VXDCDCTrackMergerAnalysisModule::~VXDCDCTrackMergerAnalysisModule()
{
}


void VXDCDCTrackMergerAnalysisModule::initialize()
{

  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track>::required(m_GFTracksColName);

  //std::map<genfit::Track*, genfit::Track*>* matched_tracks_map = new std::map<genfit::Track*, genfit::Track*>();

  //for global merging efficiency
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;

  //root tree variables
  //if (m_produce_root_file) {
  m_root_file = new TFile(m_root_output_filename.c_str(), "RECREATE");
  m_ttree     = new TTree("MatchedTrks", "Matched Tracks");

  m_pre_match_vec = new std::vector<int>();
  m_match_vec = new std::vector<int>();
  m_chi2_vec = new std::vector<float>();
  m_dist_vec = new std::vector<float>();
  m_dx_vec   = new std::vector<float>();
  m_dy_vec   = new std::vector<float>();
  m_dz_vec   = new std::vector<float>();
  m_x_vec   = new std::vector<float>();
  m_y_vec   = new std::vector<float>();
  m_z_vec   = new std::vector<float>();
  m_dmom_vec = new std::vector<float>();
  m_dmomx_vec   = new std::vector<float>();
  m_dmomy_vec   = new std::vector<float>();
  m_dmomz_vec   = new std::vector<float>();
  m_momx_vec   = new std::vector<float>();
  m_momy_vec   = new std::vector<float>();
  m_momz_vec   = new std::vector<float>();

  m_ttree->Branch("npair",     &m_npair,                  "npair/I");
  m_ttree->Branch("ncdc_trk",  &m_ncdc_trk,               "ncdc_trk/I");
  m_ttree->Branch("nsi_trk",   &m_nsi_trk,                "nsi_trk/I");
  m_ttree->Branch("mrg_eff",   &m_trk_mrg_eff,            "mrg_eff/D");
  m_ttree->Branch("match",     "std::vector<int>",        &m_match_vec);
  m_ttree->Branch("pre_match",     "std::vector<int>",        &m_pre_match_vec);
  m_ttree->Branch("chi2",      "std::vector<float>",      &m_chi2_vec);
  m_ttree->Branch("dist",      "std::vector<float>",      &m_dist_vec);
  m_ttree->Branch("dx",        "std::vector<float>",      &m_dx_vec);
  m_ttree->Branch("dy",        "std::vector<float>",      &m_dy_vec);
  m_ttree->Branch("dz",        "std::vector<float>",      &m_dz_vec);
  m_ttree->Branch("x",        "std::vector<float>",      &m_x_vec);
  m_ttree->Branch("y",        "std::vector<float>",      &m_y_vec);
  m_ttree->Branch("z",        "std::vector<float>",      &m_z_vec);
  m_ttree->Branch("dmom",      "std::vector<float>",      &m_dmom_vec);
  m_ttree->Branch("dmomx",        "std::vector<float>",      &m_dmomx_vec);
  m_ttree->Branch("dmomy",        "std::vector<float>",      &m_dmomy_vec);
  m_ttree->Branch("dmomz",        "std::vector<float>",      &m_dmomz_vec);
  m_ttree->Branch("momx",        "std::vector<float>",      &m_momx_vec);
  m_ttree->Branch("momy",        "std::vector<float>",      &m_momy_vec);
  m_ttree->Branch("momz",        "std::vector<float>",      &m_momz_vec);
  //}
}

void VXDCDCTrackMergerAnalysisModule::beginRun()
{
}

void VXDCDCTrackMergerAnalysisModule::event()
{
  //StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  //mcParticles.create();

  //clear ntuple varibles
  //if (m_produce_root_file) {
  m_pre_match_vec->clear();
  m_match_vec->clear();
  m_chi2_vec->clear();
  m_dist_vec->clear();
  m_dx_vec->clear();
  m_dy_vec->clear();
  m_dz_vec->clear();
  m_x_vec->clear();
  m_y_vec->clear();
  m_z_vec->clear();
  m_dmom_vec->clear();
  m_dmomx_vec->clear();
  m_dmomy_vec->clear();
  m_dmomz_vec->clear();
  m_momx_vec->clear();
  m_momy_vec->clear();
  m_momz_vec->clear();
  //}

  //get silicon tracks
  StoreArray<genfit::Track> si_mcGFTracks(m_SiGFTracksColName);
  unsigned int nSiTracks = si_mcGFTracks.getEntries();

  B2INFO("VXDCDCTrackMerger: input Number of Silicon Tracks: " << nSiTracks);
  if (nSiTracks == 0) B2WARNING("SiCDCTrackMerger: SiGFTracksCollection is empty!");

  //get CDC tracks
  StoreArray<genfit::Track> cdc_mcGFTracks(m_CDCGFTracksColName);
  unsigned int nCDCTracks = cdc_mcGFTracks.getEntries();

  B2INFO("VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("VXDCDCTrackMerger: CDCGFTracksCollection is empty!");

  StoreArray<genfit::Track> mcGFTracks(m_GFTracksColName);
  //unsigned int nTracks = mcGFTracks.getEntries();

  const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);

  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  TVectorD si_trk_state;
  TMatrixDSym si_trk_covmtrx(6);
  TVectorD cdc_trk_state;
  TMatrixDSym cdc_trk_covmtrx(6);
  TVector3 pos;
  TVector3 mom;
  unsigned int si_trk_it[nSiTracks];
  unsigned int cdc_trk_it[nCDCTracks];
  int mtch = 0;
  int si_trk_idx = 0;
  //int cdc_trk_idx=0;

  std::map<genfit::Track*, genfit::Track*>* matched_tracks_map = new std::map<genfit::Track*, genfit::Track*>();
  unsigned int matched_track = 0;
  //std::map<genfit::Track*, genfit::Track*>* unmatched_tracks_map = new std::map<genfit::Track*, genfit::Track*>();

  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) { //extrapolate to the CDC wall
    try {
      genfit::MeasuredStateOnPlane cdc_sop = cdc_mcGFTracks[itrack]->getFittedState();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdc_sop.get6DStateCov(cdc_trk_state, cdc_trk_covmtrx);
      //cdc_trk_covmtrx=cdc_sop.get6DCov();
      pos = cdc_sop.getPos();
      mom = cdc_sop.getMom();
      //sop.getPosMomCov(pos, mom, cov);
      //int id1=cdc_mcGFTracks[itrack]->getMcTrackId();
    } catch (...) {
      B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    double CHI2_MAX = 100;
    mtch = 0;

    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nSiTracks; jtrack++) {
      try {
        genfit::MeasuredStateOnPlane si_sop = si_mcGFTracks[jtrack]->getFittedState();
        si_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        si_sop.get6DStateCov(si_trk_state, si_trk_covmtrx);

      } catch (...) {
        B2WARNING("SiTrack extrapolation to cylinder failed!");
        continue;
      }
      //Match Tracks
      TMatrixDSym inv_covmtrx = (cdc_trk_covmtrx + si_trk_covmtrx).Invert(); //.Invert() ndr
      TVectorD state_diff = cdc_trk_state - si_trk_state;
      //double raw_diff = (cdc_trk_state-si_trk_state)*(cdc_trk_state-si_trk_state);
      state_diff *= inv_covmtrx;
      double chi_2 = state_diff * (cdc_trk_state - si_trk_state);
      if ((chi_2 < CHI2_MAX) && (chi_2 > 0)) {
        mtch = 1;
        CHI2_MAX = chi_2;
        matched_track = jtrack;
      }

      genfit::Track* GFTrk = mcGFTracks[itrack];
      const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (cdc_TrkCandPtr == NULL) {
        //  B2WARNING("CDC track candidate pointer is NULL (VXDCDCTrackMerger)");
        //  n_cdc++;
        continue;
      }
      int cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();

      GFTrk = mcGFTracks[jtrack + nCDCTracks];
      const genfit::TrackCand* si_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (si_TrkCandPtr == NULL) {
        //  B2WARNING("Si track candidate pointer is NULL (VXDCDCTrackMerger)");
        //  n_cdc++;
        continue;
      }
      int si_mcp_index = si_TrkCandPtr->getMcTrackId();
      //if (cdc_mcp_index == si_mcp_index) {
      //  true_match++;
      //}

      if (cdc_mcp_index == si_mcp_index) {
        m_pre_match_vec->push_back(1);
      } //else {
      //m_pre_match_vec->push_back(-1);
      //}


    } //save matched tracks
    if (mtch == 1) {
      std::pair<genfit::Track*, genfit::Track*> matched_track_pair(cdc_mcGFTracks[itrack], si_mcGFTracks[matched_track]);
      matched_tracks_map->insert(matched_track_pair);
      si_trk_it[si_trk_idx] = matched_track + nCDCTracks;
      cdc_trk_it[si_trk_idx] = itrack;
      si_trk_idx++;
    }
  }

  //Analysis part, has to be put in a different module

  int n_trk_pair = 0;
  int true_match = 0;
  unsigned int n_cdc = 0;
  //fill analysis (root tree) variables for MATCHED TRACKS
  for (std::map<genfit::Track*, genfit::Track*>::iterator trk_it = matched_tracks_map->begin(); trk_it != matched_tracks_map->end(); ++trk_it) {
    genfit::Track* cdc_trk = trk_it->first;
    genfit::MeasuredStateOnPlane cdc_sop = cdc_trk->getFittedState();
    cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
    cdc_sop.get6DStateCov(cdc_trk_state, cdc_trk_covmtrx);
    //cdc_trk_covmtrx=cdc_sop.get6DCov();
    TVector3 cdc_trk_position = cdc_sop.getPos();
    TVector3 cdc_trk_momentum = cdc_sop.getMom();
    //TVector3 cdc_trk_position   = cdc_trk->getCardinalRep()->getPos();
    //TVectorD cdc_trk_state      = cdc_trk->getCardinalRep()->getState();
    //TMatrixDSym cdc_trk_covmtrx = cdc_trk->getCardinalRep()->getCov();
    genfit::Track* si_trk = trk_it->second;
    genfit::MeasuredStateOnPlane si_sop = si_trk->getFittedState();
    si_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
    si_sop.get6DStateCov(si_trk_state, si_trk_covmtrx);
    TVector3 si_trk_position = si_sop.getPos();
    TVector3 si_trk_momentum = si_sop.getMom();

    TMatrixDSym inv_covmtrx = (cdc_trk_covmtrx + si_trk_covmtrx).Invert(); //.Invert() ndr
    TVectorD state_diff = cdc_trk_state - si_trk_state;
    //double raw_diff = (cdc_trk_state-si_trk_state)*(cdc_trk_state-si_trk_state);
    state_diff *= inv_covmtrx;
    double chi_2 = state_diff * (cdc_trk_state - si_trk_state);

    genfit::Track* GFTrk = mcGFTracks[cdc_trk_it[n_cdc]];
    const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
    if (cdc_TrkCandPtr == NULL) {
      B2WARNING("CDC track candidate pointer is NULL (VXDCDCTrackMerger)");
      n_cdc++;
      continue;
    }
    int cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();

    GFTrk = mcGFTracks[si_trk_it[n_cdc]];
    const genfit::TrackCand* si_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
    if (si_TrkCandPtr == NULL) {
      B2WARNING("Si track candidate pointer is NULL (VXDCDCTrackMerger)");
      n_cdc++;
      continue;
    }
    int si_mcp_index = si_TrkCandPtr->getMcTrackId();
    //const MCParticle* si_mcparticle = DataStore::getRelatedFromObj<MCParticle>(si_trk);
    //unsigned int si_mcp_index = si_mcparticle->getIndex();
    if (cdc_mcp_index == si_mcp_index) {
      true_match++;
    }
    n_cdc++;

    //root tree stuff
    //if (m_produce_root_file) {
    //matching status variable
    if (cdc_mcp_index == si_mcp_index) {
      m_match_vec->push_back(1);
    } else {
      m_match_vec->push_back(0);
    }
    //chi-square and residuals
    m_chi2_vec->push_back(chi_2);
    m_dist_vec->push_back((cdc_trk_position - si_trk_position).Mag());
    m_dx_vec->push_back(cdc_trk_position.X() - si_trk_position.X());
    m_dy_vec->push_back(cdc_trk_position.Y() - si_trk_position.Y());
    m_dz_vec->push_back(cdc_trk_position.Z() - si_trk_position.Z());
    m_x_vec->push_back((cdc_trk_position.X() + si_trk_position.X()) / 2);
    m_y_vec->push_back((cdc_trk_position.Y() + si_trk_position.Y()) / 2);
    m_z_vec->push_back((cdc_trk_position.Z() + si_trk_position.Z()) / 2);
    m_dmom_vec->push_back((cdc_trk_momentum - si_trk_momentum).Mag());
    m_dmomx_vec->push_back(cdc_trk_momentum.X() - si_trk_momentum.X());
    m_dmomy_vec->push_back(cdc_trk_momentum.Y() - si_trk_momentum.Y());
    m_dmomz_vec->push_back(cdc_trk_momentum.Z() - si_trk_momentum.Z());
    m_momx_vec->push_back((cdc_trk_momentum.X() + si_trk_momentum.X()) / 2);
    m_momy_vec->push_back((cdc_trk_momentum.Y() + si_trk_momentum.Y()) / 2);
    m_momz_vec->push_back((cdc_trk_momentum.Z() + si_trk_momentum.Z()) / 2);
    //}// end of if (m_produce_root_file)
    n_trk_pair++;
  }

  //if (m_produce_root_file) {
  //number of si tracks, number of cdc tracks before merging and number of track pairs after merging
  m_nsi_trk = nSiTracks;
  m_ncdc_trk = nCDCTracks;
  m_npair = n_trk_pair;
  //that's the number of correctly merged tracks over all merged tracks per event
  m_trk_mrg_eff = double(true_match) / double(n_trk_pair);

  //for calculating global merging efficiency (in contast to event by event efficiency)
  m_total_pairs         = m_total_pairs + double(n_trk_pair);
  m_total_matched_pairs = m_total_matched_pairs + true_match;
  m_ttree->Fill();
  //}
  delete matched_tracks_map;

}

void VXDCDCTrackMergerAnalysisModule::endRun()
{
}

void VXDCDCTrackMergerAnalysisModule::terminate()
{

  //root stuff
  //if (m_produce_root_file) {
  //calculate the track merging efficiecny considering the whole number of simulated events
  //TVectorT<double>* global_trkmrg_eff = new  TVectorT<double>(1);
  //(*global_trkmrg_eff)[0] =  m_total_matched_pairs / m_total_pairs;
  //m_ttree->GetUserInfo()->Add(global_trkmrg_eff);
  m_root_file->cd();
  m_ttree->Write();
  m_root_file->Close();
  //}
}
