#include <tracking/modules/siCDCTrackMerger/inc/SiCDCTrackMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "GFTrackCand.h"
#include <generators/dataobjects/MCParticle.h>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(SiCDCTrackMerger)

SiCDCTrackMergerModule::SiCDCTrackMergerModule() : Module()
{
  setDescription("This module merges tracks which are reconstructed, separately, in the slicon (PXD+VXD) and the CDC sub-conductors");
  addParam("CDC_wall_radius",    m_CDC_wall_radius,    "This parameters stands for either the inner or the outer CDC wall radius", double(100.0));
  //eta grid
  addParam("Theta_min", m_theta_min, "Minimum theta in the grid", double(0));
  addParam("Theta_max", m_theta_max, "Maximum theta in the grid", double(3.15));
  addParam("N_Theta_bin", m_N_theta_bin, "Number of bin theta direction", int(6));
  addParam("theta_overlap_size", m_theta_overlap_size, "How much two theta bins overlap (in percent)", double(0.05));
  //phi grid
  m_PI = 3.14159265358979323846;
  m_phi_min = 0;//the user should not change these values, they are used as such.
  m_phi_max = 2 * m_PI;
  addParam("N_Phi_bin", m_N_phi_bin, "Number of bin phi direction", int(8));
  addParam("phi_overlap_size", m_phi_overlap_size, "How much two phi bins overlap (in percent)", double(0.05));
  //input tracks
  addParam("SiGFTracksColName",  m_SiGFTracksColName,  "Silicon GFTrack collection");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GTTrack collection");

  //root stuff
  addParam("produce_root_file", m_produce_root_file, "if yes, generate a local root file", bool(false));
  addParam("root_output_filename", m_root_output_filename, "ROOT file for tracks merger analysis", std::string("si_cdc_trackmerger.root"));
}


SiCDCTrackMergerModule::~SiCDCTrackMergerModule()
{
}


void SiCDCTrackMergerModule::initialize()
{
  //for global merging efficiency
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
  //calculate the theta and phi bin length
  m_theta_bin_length = (m_theta_max -  m_theta_min) / double(m_N_theta_bin);
  m_phi_bin_length   = (m_phi_max -  m_phi_min) / double(m_N_phi_bin);

  //initializing the silicon tracks grid
  m_si_tracks_grid = new std::vector<std::vector<std::vector<GFTrack*>*>*>();
  for (int itheta = 0; itheta < m_N_theta_bin; itheta++) {
    std::vector<std::vector<GFTrack*>*>* phi_row = new std::vector<std::vector<GFTrack*>*>();
    for (int iphi = 0; iphi < m_N_phi_bin; iphi++) {
      std::vector<GFTrack*>* thetaphi_cell = new std::vector<GFTrack*>();
      phi_row->push_back(thetaphi_cell);
    }
    m_si_tracks_grid->push_back(phi_row);
  }

  //initializing the cdc tracks grid and cdc wall crossing positons
  m_cdc_tracks_grid = new std::vector<std::vector<std::vector<GFTrack*>*>*>();
  m_cdc_tracks_position = new std::vector<std::vector<std::vector<TVector3>*>*>();
  for (int itheta = 0; itheta < m_N_theta_bin; itheta++) {
    std::vector<std::vector<GFTrack*>*>* phi_row_vec = new std::vector<std::vector<GFTrack*>*>();
    std::vector<std::vector<TVector3>*>* phi_pos_vec = new std::vector<std::vector<TVector3>*>();
    for (int iphi = 0; iphi < m_N_phi_bin; iphi++) {
      std::vector<GFTrack*>* thetaphi_cell_vec = new std::vector<GFTrack*>();
      phi_row_vec->push_back(thetaphi_cell_vec);
      std::vector<TVector3>* thetaphi_pos_vec = new std::vector<TVector3>();
      phi_pos_vec->push_back(thetaphi_pos_vec);
    }
    m_cdc_tracks_grid->push_back(phi_row_vec);
    m_cdc_tracks_position->push_back(phi_pos_vec);
  }

  //root tree variables
  if (m_produce_root_file) {
    m_root_file = new TFile(m_root_output_filename.c_str(), "RECREATE");
    m_ttree     = new TTree("t3", "Track Marger Variables");

    m_match_vec = new std::vector<int>();
    m_chi2_vec = new std::vector<float>();
    m_dist_vec = new std::vector<float>();
    m_dx_vec   = new std::vector<float>();
    m_dy_vec   = new std::vector<float>();
    m_dz_vec   = new std::vector<float>();

    m_ttree->Branch("npair",     &m_npair,                  "npair/I");
    m_ttree->Branch("ncdc_trk",  &m_ncdc_trk,               "ncdc_trk/I");
    m_ttree->Branch("nsi_trk",   &m_nsi_trk,                "nsi_trk/I");
    m_ttree->Branch("mrg_eff",   &m_trk_mrg_eff,            "mrg_eff/D");
    m_ttree->Branch("match",     "std::vector<int>",        &m_match_vec);
    m_ttree->Branch("chi2",      "std::vector<float>",      &m_chi2_vec);
    m_ttree->Branch("dist",      "std::vector<float>",      &m_dist_vec);
    m_ttree->Branch("dx",        "std::vector<float>",      &m_dx_vec);
    m_ttree->Branch("dy",        "std::vector<float>",      &m_dy_vec);
    m_ttree->Branch("dz",        "std::vector<float>",      &m_dz_vec);
  }
}

void SiCDCTrackMergerModule::beginRun()
{
}

void SiCDCTrackMergerModule::event()
{
  //B2INFO("SiCDCTrackMerger: "<< m_SiGFTracksColName);
  //clear the silicon and cdc grids
  for (int itheta = 0; itheta < m_N_theta_bin; itheta++) {
    for (int iphi = 0; iphi < m_N_phi_bin; iphi++) {
      ((m_si_tracks_grid->at(itheta))->at(iphi))->clear();
      ((m_cdc_tracks_grid->at(itheta))->at(iphi))->clear();
    }
  }
  //clear ntuple varibles
  if (m_produce_root_file) {
    m_match_vec->clear();
    m_chi2_vec->clear();
    m_dist_vec->clear();
    m_dx_vec->clear();
    m_dy_vec->clear();
    m_dz_vec->clear();
  }
  //get silicon tracks
  StoreArray<GFTrack> si_mcGFTracks(m_SiGFTracksColName);
  unsigned int nSiTracks = si_mcGFTracks.getEntries();

  //B2INFO("SiCDCTrackMerger: input Number of Silicon Tracks: " << nSiTracks);
  if (nSiTracks == 0) B2WARNING("SiCDCTrackMerger: SiGFTracksCollection is empty!");

  //get CDC tracks
  StoreArray<GFTrack> cdc_mcGFTracks(m_CDCGFTracksColName);
  unsigned int nCDCTracks = cdc_mcGFTracks.getEntries();

  //B2INFO("SiCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("SiCDCTrackMerger: CDCGFTracksCollection is empty!");

  TVector3 position(0., 0., 0.); //position at which the track crosses the CDC wall
  TVector3 momentum(0., 0., 0.); //direction of the track at that point
  //loop on silicon tracks
  for (unsigned int itrack = 0; itrack < nSiTracks; itrack++) {
    //extrapolate the track to the CDC wall, the results are stored directly in position and momentum
    try {
      si_mcGFTracks[itrack]->getCardinalRep()->extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
    } catch (...) {
      B2WARNING("Track extrapolation to cylinder failed!");
      //retrun;
      continue;
    }
    //insert this track in the corresponding cell in the grid
    insertTrackInGrid(position.Theta(), position.Phi(), si_mcGFTracks[itrack], m_si_tracks_grid);
  }//end of for(unsigned int itrack=0; itrack<nSiTracks;


  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) {
    //extrapolate the track to the CDC wall, the results are stored directly in position and momentum
    try {
      cdc_mcGFTracks[itrack]->getCardinalRep()->extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
    } catch (...) {
      B2WARNING("Track extrapolation to cylinder failed!");
      //return;
      continue;
    }
    //insert this track in the corresponding cell in the grid
    insertTrackAndPositionInGrid(position, cdc_mcGFTracks[itrack], m_cdc_tracks_position, m_cdc_tracks_grid);
  }//end of for(unsigned int itrack=0; itrack<nCDCTracks; ...)

  std::map<GFTrack*, GFTrack*>* matched_tracks_map = new std::map<GFTrack*, GFTrack*>();
  //loop on the silicon and cdc grid cells and try to associate the tracks
  for (int itheta = 0; itheta < m_N_theta_bin; itheta++) {
    for (int iphi = 0; iphi < m_N_phi_bin; iphi++) {
      int N_cdc_Tracks = ((m_cdc_tracks_grid->at(itheta))->at(iphi))->size();
      //loop on the cdc tracks and the si tracks and find matched ones
      for (int i_cdc_trk = 0; i_cdc_trk < N_cdc_Tracks; i_cdc_trk++) {
        //create a plane tangent to the cylinder and get the track state on that plane
        TVector3 position = ((m_cdc_tracks_position->at(itheta))->at(iphi))->at(i_cdc_trk);
        TVector3 cyl_norm_direction(position.X(), position.Y(), 0);
        GFDetPlane cylinder_plane(position, cyl_norm_direction);

        GFTrack* cdc_track = ((m_cdc_tracks_grid->at(itheta))->at(iphi))->at(i_cdc_trk);
        try {
          cdc_track->getCardinalRep()->extrapolate(cylinder_plane);
        } catch (...) {
          B2WARNING("Track extrapolation to cylinder plane failed!");
          //return;
          continue;
        }

        TVectorD cdc_trk_state      = cdc_track->getCardinalRep()->getState();
        TMatrixDSym cdc_trk_covmtrx = cdc_track->getCardinalRep()->getCov();

        int matched_trk = -1;
        double CHI2_MAX = 100000000.0;
        int N_si_Tracks = ((m_si_tracks_grid->at(itheta))->at(iphi))->size();
        //loop on the si tracks and find the one that matches the actual cdc track.
        for (int i_si_trk = 0; i_si_trk < N_si_Tracks; i_si_trk++) {
          GFTrack* si_track = ((m_si_tracks_grid->at(itheta))->at(iphi))->at(i_si_trk);
          //extrapolate the si track to the same plane
          try {
            si_track->getCardinalRep()->extrapolate(cylinder_plane);
          } catch (...) {
            B2WARNING("Track extrapolation to cylinder plane failed!");
            //return;
            continue;
          }

          //get the track state on that plane
          TVectorD si_trk_state      = si_track->getCardinalRep()->getState();
          TMatrixDSym si_trk_covmtrx = si_track->getCardinalRep()->getCov();

          TMatrixDSym inv_covmtrx = (cdc_trk_covmtrx.Invert() + si_trk_covmtrx.Invert()).Invert();
          TVectorD state_diff = cdc_trk_state - si_trk_state;
          state_diff *= inv_covmtrx;
          double chi_2 = state_diff * state_diff;

          if (chi_2 < CHI2_MAX) {
            CHI2_MAX = chi_2;
            matched_trk = i_si_trk;
          }
        }//end of for(int i_si_trk=0; ...)
        if (matched_trk > -1) {
          GFTrack* si_track = ((m_si_tracks_grid->at(itheta))->at(iphi))->at(matched_trk);
          std::pair<GFTrack*, GFTrack*> track_pair(cdc_track, si_track);
          matched_tracks_map->insert(track_pair);
        }
      }//end of  for(int i_cdc_trk=0; ...)
    }//end of for(int iphi=0; ...)
  }//end of for(int itheta=0; ...)

  int n_trk_pair = 0;
  int true_match = 0;
  //fill analysis (root tree) variables. it's not possible to do it above because some track pairs belong to more than one cell and thus
  //one will get the same entry more than onece (which may screw a distribution)!
  for (std::map<GFTrack*, GFTrack*>::iterator trk_it = matched_tracks_map->begin(); trk_it != matched_tracks_map->end(); ++trk_it) {
    GFTrack* cdc_trk = trk_it->first;
    TVector3 cdc_trk_position   = cdc_trk->getCardinalRep()->getPos();
    TVectorD cdc_trk_state      = cdc_trk->getCardinalRep()->getState();
    TMatrixDSym cdc_trk_covmtrx = cdc_trk->getCardinalRep()->getCov();
    GFTrack* si_trk = trk_it->second;
    TVector3 si_trk_position    = si_trk->getCardinalRep()->getPos();
    TVectorD si_trk_state       = si_trk->getCardinalRep()->getState();
    TMatrixDSym si_trk_covmtrx  = si_trk->getCardinalRep()->getCov();

    TMatrixDSym inv_covmtrx = (cdc_trk_covmtrx.Invert() + si_trk_covmtrx.Invert()).Invert();
    TVectorD state_diff = cdc_trk_state - si_trk_state;
    state_diff *= inv_covmtrx;
    double chi_2 = state_diff * state_diff;

    const MCParticle* cdc_mcparticle = DataStore::getRelatedFromObj<MCParticle>(cdc_trk);
    if (cdc_mcparticle == NULL) {
      B2WARNING("cdc track MCParticle pointer is NULL");
      //return;
      continue;
    }
    unsigned int cdc_mcp_index = cdc_mcparticle->getIndex();
    const MCParticle* si_mcparticle = DataStore::getRelatedFromObj<MCParticle>(si_trk);
    if (si_mcparticle == NULL) {
      B2WARNING("si track MCParticle pointer is NULL");
      //return;
      continue;
    }
    unsigned int si_mcp_index = si_mcparticle->getIndex();
    if (cdc_mcp_index == si_mcp_index) {
      true_match++;
    }

    //root tree stuff
    if (m_produce_root_file) {
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
    }// end of if (m_produce_root_file)
    n_trk_pair++;
  }//end of for (std::map<GFTrack*, GFTrack*>::iterator trk_it = ...)

  if (m_produce_root_file) {
    //number of si tracks, number of cdc tracks and number of track pairs
    m_nsi_trk = nSiTracks;
    m_ncdc_trk = nCDCTracks;
    m_npair = n_trk_pair;
    //calculate the merging efficiency (in fact it's purity)
    m_trk_mrg_eff = double(true_match) / double(n_trk_pair);

    //for calculating global merging efficiency (in contast to event by event efficiecny)
    m_total_pairs         = m_total_pairs + double(n_trk_pair);
    m_total_matched_pairs = m_total_matched_pairs + true_match;
    m_ttree->Fill();
  }
  delete matched_tracks_map;
}


void SiCDCTrackMergerModule::insertTrackInGrid(double track_theta, double track_phi, GFTrack* gftrack,
                                               std::vector<std::vector<std::vector<GFTrack*>*>*>* tracks_grid)
{
  double theta_bin = track_theta / m_theta_bin_length;
  //if track_theta is on the border of a cell, make two indices
  unsigned int theta_bin_index_1 = int(theta_bin + m_theta_overlap_size);
  unsigned int theta_bin_index_2 = int(theta_bin - m_theta_overlap_size);

  //transform phi from the interval [-pi, pi] to the interval [0, 2pi] to get positive indices
  track_phi = track_phi + m_PI;

  //if track_phi is on the border of a cell, make two indices
  double track_phi_1 = track_phi + m_phi_overlap_size * m_phi_bin_length;
  if (track_phi_1 > m_phi_max) track_phi_1 = track_phi_1 - m_phi_max;
  unsigned int phi_bin_index_1 =  int(track_phi_1 / m_phi_bin_length);
  double track_phi_2 = track_phi - m_phi_overlap_size * m_phi_bin_length;
  if (track_phi_2 < m_phi_min) track_phi_2 = track_phi_2 + m_phi_max; //track_phi_2 < 0.
  unsigned int phi_bin_index_2 =  track_phi_2 / m_phi_bin_length;

  if (theta_bin_index_1 == theta_bin_index_2 && phi_bin_index_1 == phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
  } else if (theta_bin_index_1 == theta_bin_index_2 && phi_bin_index_1 != phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(gftrack);
  } else if (theta_bin_index_1 != theta_bin_index_2 && phi_bin_index_1 == phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(gftrack);
  } else {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_2))->push_back(gftrack);
  }
}

void SiCDCTrackMergerModule::insertTrackAndPositionInGrid(TVector3 position, GFTrack* gftrack,
                                                          std::vector<std::vector<std::vector<TVector3>*>*>* positions_grid,
                                                          std::vector<std::vector<std::vector<GFTrack*>*>*>* tracks_grid)
{
  double track_theta = position.Theta();
  double track_phi   = position.Phi();
  double theta_bin = track_theta / m_theta_bin_length;
  unsigned int theta_bin_index_1 = int(theta_bin + m_theta_overlap_size);
  unsigned int theta_bin_index_2 = int(theta_bin - m_theta_overlap_size);

  //transform phi from the interval [-pi, pi] to the interval [0, 2pi] to get positive indices
  track_phi = track_phi + m_PI;

  //if track_phi is on the border of a cell, make two indices
  double track_phi_1 = track_phi + m_phi_overlap_size * m_phi_bin_length;
  if (track_phi_1 > m_phi_max) track_phi_1 = track_phi_1 - m_phi_max;
  unsigned int phi_bin_index_1 =  int(track_phi_1 / m_phi_bin_length);
  double track_phi_2 = track_phi - m_phi_overlap_size * m_phi_bin_length;
  if (track_phi_2 < m_phi_min) track_phi_2 = track_phi_2 + m_phi_max; //track_phi_2 < 0.
  unsigned int phi_bin_index_2 =  track_phi_2 / m_phi_bin_length;

  if (theta_bin_index_1 == theta_bin_index_2 && phi_bin_index_1 == phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(position);
  } else if (theta_bin_index_1 == theta_bin_index_2 && phi_bin_index_1 != phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(gftrack);

    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(position);
    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(position);

  } else if (theta_bin_index_1 != theta_bin_index_2 && phi_bin_index_1 == phi_bin_index_2) {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(gftrack);

    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(position);
    ((positions_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(position);
  } else {
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(gftrack);
    ((tracks_grid->at(theta_bin_index_2))->at(phi_bin_index_2))->push_back(gftrack);

    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_1))->push_back(position);
    ((positions_grid->at(theta_bin_index_1))->at(phi_bin_index_2))->push_back(position);
    ((positions_grid->at(theta_bin_index_2))->at(phi_bin_index_1))->push_back(position);
    ((positions_grid->at(theta_bin_index_2))->at(phi_bin_index_2))->push_back(position);
  }
}
void SiCDCTrackMergerModule::endRun()
{
}

void SiCDCTrackMergerModule::terminate()
{

  //root stuff
  if (m_produce_root_file) {
    //calculate the track merging efficiecny considering the whole number of simulated events
    TVectorT<double>* global_trkmrg_eff = new  TVectorT<double>(1);
    (*global_trkmrg_eff)[0] =  m_total_matched_pairs / m_total_pairs;
    m_ttree->GetUserInfo()->Add(global_trkmrg_eff);
    m_root_file->cd();
    m_ttree->Write();
    m_root_file->Close();
  }
}
