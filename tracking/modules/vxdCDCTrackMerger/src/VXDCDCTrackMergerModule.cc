#include <tracking/modules/vxdCDCTrackMerger/VXDCDCTrackMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

REG_MODULE(VXDCDCTrackMerger)

VXDCDCTrackMergerModule::VXDCDCTrackMergerModule() : Module()
{
  setDescription("This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");

  //input tracks and candidates
  //addParam("GFTracksColName",  m_GFTracksColName,  "GFTrack collection");
  addParam("VXDGFTracksColName",  m_VXDGFTracksColName,  "VXDl GFTrack collection (from GFTrackSplitter)");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection (from GFTrackSplitter)");
  addParam("TrackCandColName", m_TrackCandColName, "Track Cand collection (from TrackFinder)");

  //output relationArray
  addParam("relMatchedTracks", m_relMatchedTracks, "Output RelationArray for Merged Tracks");

  //Chi2 Cut
  addParam("chi2_max", m_chi2_max, "Chi^2 cut for matching", double(100.0));
  addParam("merge_radius", m_merge_radius, "Maximum distance between extrapolated tracks on the CDC wall", double(2.0));
}


VXDCDCTrackMergerModule::~VXDCDCTrackMergerModule()
{
}


void VXDCDCTrackMergerModule::initialize()
{
  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track>::required(m_VXDGFTracksColName);
  StoreArray<genfit::Track>::required(m_CDCGFTracksColName);

  RelationArray::registerPersistent<genfit::Track, genfit::Track>(m_CDCGFTracksColName, m_VXDGFTracksColName);

  m_CDC_wall_radius     = 16.25;
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
}

void VXDCDCTrackMergerModule::beginRun()
{
}

void VXDCDCTrackMergerModule::event()
{
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

  //StoreArray<genfit::Track> GFTracks(m_GFTracksColName);
  //StoreArray<genfit::TrackCand> UnMergedCands(m_UnMergedCands);
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
  //std::map<genfit::Track*, genfit::Track*>* matched_tracks_map = new std::map<genfit::Track*, genfit::Track*>();
  RelationArray relMatchedTracks(CDCGFTracks, VXDGFTracks);

  unsigned int matched_track = 0;
  unsigned int vxd_match = 1000;
  int matches = 0;

  //loop on CDC tracks
  for (unsigned int itrack = 0; itrack < nCDCTracks; itrack++) { //extrapolate to the CDC wall from first hit
    double chi_2 = m_chi2_max;
    double CHI2_MAX = m_chi2_max;
    try {
      genfit::MeasuredStateOnPlane cdc_sop = CDCGFTracks[itrack]->getFittedState();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      pos = cdc_sop.getPos();
      mom = cdc_sop.getMom();
    } catch (...) {
      B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }
    matched_track = 0;
    vxd_match = 1000; //index for matched track candidate
    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nVXDTracks; jtrack++) {
      try {
        genfit::MeasuredStateOnPlane vxd_sop = VXDGFTracks[jtrack]->getFittedState(-1);
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (...) {
        B2WARNING("VXDTrack extrapolation to cylinder failed!");
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
          B2WARNING("Covariance matrix is singular!");
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
          matches++;
        }
      }
    }//end loop on VXD tracks

    if (matched_track == 1) {
      relMatchedTracks.add(itrack, vxd_match);
    }
    //B2INFO("VXDCDCTrackMerger: Merged Tracks: " << matches);
  }//loop on CDC

  B2INFO("VXDCDCTrackMerger: Merged Tracks: " << matches);

}

void VXDCDCTrackMergerModule::endRun()
{
}

void VXDCDCTrackMergerModule::terminate()
{
}
