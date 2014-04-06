#include <tracking/modules/vxdCDCTrackMerger/inc/VXDCDCTrackMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

REG_MODULE(VXDCDCTrackMerger)

VXDCDCTrackMergerModule::VXDCDCTrackMergerModule() : Module()
{
  setDescription("This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");
  addParam("CDC_wall_radius",    m_CDC_wall_radius,    "This parameters stands for either the inner or the outer CDC wall radius", double(100.0));
  //input tracks
  addParam("GFTracksColName",  m_GFTracksColName,  "Originary GFTrack collection");
  addParam("SiGFTracksColName",  m_SiGFTracksColName,  "Silicon GFTrack collection");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GTTrack collection");
  //addParam("MCParticlesColName", m_mcParticlesColName, "MCParticles collection");
  addParam("TrackCandColName", m_TrackCandColName, "CDC Track Cand collection");

  addParam("mergedSiGFTracksColName",  m_mergedSiGFTracksColName,  "Succesfully merged Silicon GFTrack collection");
  addParam("mergedCDCGFTracksColName", m_mergedCDCGFTracksColName, "Succesfully merged CDC GFTrack collection");
  addParam("chi2_max", m_chi2_max, "Maximum Chi^2 for matching", double(100.0));
}


VXDCDCTrackMergerModule::~VXDCDCTrackMergerModule()
{
}


void VXDCDCTrackMergerModule::initialize()
{
  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track>::required(m_GFTracksColName);

  StoreArray<genfit::Track>::registerPersistent(m_mergedCDCGFTracksColName);
  //mergedCDCGFTracksColName.registerAsPersistent();
  StoreArray<genfit::Track>::registerPersistent(m_mergedSiGFTracksColName);
  //mergedSiGFTracksColName.registerAsPersistent();

  //for global merging efficiency
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
}

void VXDCDCTrackMergerModule::beginRun()
{
}

void VXDCDCTrackMergerModule::event()
{
  //get VXD tracks
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
  const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);
  StoreArray<genfit::Track> mergedCDCGFTracks(m_mergedCDCGFTracksColName);
  StoreArray<genfit::Track> mergedSiGFTracks(m_mergedSiGFTracksColName);

  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  TVectorD si_trk_state;
  TMatrixDSym si_trk_covmtrx(6);
  TVectorD cdc_trk_state;
  TMatrixDSym cdc_trk_covmtrx(6);
  TVector3 pos;
  TVector3 mom;
  //unsigned int si_trk_it[nSiTracks];
  //unsigned int cdc_trk_it[nCDCTracks];
  int mtch = 0;
  //int si_trk_idx=0;

  std::map<genfit::Track*, genfit::Track*>* matched_tracks_map = new std::map<genfit::Track*, genfit::Track*>();
  unsigned int matched_track = 0;

  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) { //extrapolate to the CDC wall
    try {
      genfit::MeasuredStateOnPlane cdc_sop = cdc_mcGFTracks[itrack]->getFittedState();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdc_sop.get6DStateCov(cdc_trk_state, cdc_trk_covmtrx);
      pos = cdc_sop.getPos();
      mom = cdc_sop.getMom();
    } catch (...) {
      B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    double CHI2_MAX = m_chi2_max;
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
      state_diff *= inv_covmtrx;
      double chi_2 = state_diff * (cdc_trk_state - si_trk_state);
      if ((chi_2 < CHI2_MAX) && (chi_2 > 0)) {
        mtch = 1;
        CHI2_MAX = chi_2;
        matched_track = jtrack;
      }

    } //save matched tracks
    if (mtch == 1) {
      std::pair<genfit::Track*, genfit::Track*> matched_track_pair(cdc_mcGFTracks[itrack], si_mcGFTracks[matched_track]);
      matched_tracks_map->insert(matched_track_pair);
      mergedCDCGFTracks.appendNew(*(cdc_mcGFTracks[itrack]));
      mergedSiGFTracks.appendNew(*(si_mcGFTracks[matched_track]));
    }
  }
  delete matched_tracks_map;
}

void VXDCDCTrackMergerModule::endRun()
{
}

void VXDCDCTrackMergerModule::terminate()
{
}
