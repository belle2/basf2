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

VXDCDCTrackMergerModule::VXDCDCTrackMergerModule() :
  Module(),
  m_CDC_wall_radius(16.25),
  m_total_pairs(0),
  m_total_matched_pairs(0)
{
  setDescription("This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");

  //input
  //  addParam("GFTracksColName",  m_GFTracksColName,  "GFTrack collection");
  addParam("VXDGFTracksColName",  m_VXDGFTracksColName,  "VXDl GFTrack collection (from GFTrackSplitter)");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection (from GFTrackSplitter)");
  //  addParam("TrackCandColName", m_TrackCandColName, "Track Cand collection (from TrackFinder)");

  //output
  addParam("relMatchedTracks", m_relMatchedTracks, "Output RelationArray for Merged Tracks");

  //merging parameters
  addParam("chi2_max", m_chi2_max, "Chi^2 cut for matching", double(100.0));
  addParam("recover", m_recover, "Flag for recovery mode, default is 1", bool(1));
  addParam("merge_radius", m_merge_radius, "Maximum distance between extrapolated tracks on the CDC wall", double(2.0));
}



VXDCDCTrackMergerModule::~VXDCDCTrackMergerModule()
{
}


void VXDCDCTrackMergerModule::initialize()
{
  //  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<genfit::Track> vxdTracks(m_VXDGFTracksColName);
  StoreArray<genfit::Track> cdcTracks(m_CDCGFTracksColName);
  //  StoreArray<genfit::Track>::required(m_GFTracksColName);

  vxdTracks.isRequired();
  cdcTracks.isRequired();
  cdcTracks.registerRelationTo(vxdTracks);

  m_CDC_wall_radius     = 16.25;
  m_total_pairs         = 0;
  m_total_matched_pairs = 0;
}



void VXDCDCTrackMergerModule::beginRun()
{
}



void VXDCDCTrackMergerModule::event()
{
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

  //  StoreArray<genfit::Track> GFTracks(m_GFTracksColName);
  //StoreArray<genfit::TrackCand> UnMergedCands(m_UnMergedCands);
  //  const StoreArray<genfit::TrackCand> TrackCand(m_TrackCandColName);

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
  RelationArray relMatchedTracks(CDCGFTracks, VXDGFTracks);

  unsigned int matched_track = 0;
  unsigned int recovered_track = 0;
  //unsigned int truth_matched = 0;
  //unsigned int truth_flag = 0;
  unsigned int vxd_match = 1000;
  float rel_weight = 1;

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
      //B2WARNING("CDCTrack extrapolation to cylinder failed!");
      continue;
    }
    matched_track = 0;
    recovered_track = 0;
    //truth_matched = 0;
    vxd_match = 1000; //index for matched track candidate
    //vxd_truth = 2000; //index for true matched candidate
    //loop on VXD Tracks
    for (unsigned int jtrack = 0; jtrack < nVXDTracks; jtrack++) {
      //vxd_mcp_index = 3000;
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
        } else if ((matched_track == 0) && (chi_2 > 0) && (TMath::Sqrt((pos - vxdpos) * (pos - vxdpos)) < merge_radius)) {
          recovered_track = 1;
          //CHI2_MAX = chi_2;
          vxd_match = jtrack;
          vxd_xmin = vxd_sop.getPos();
          vxd_pmin = vxd_sop.getMom();
        }
      }//If on radius

      /*
      //Recover track candidate index
      genfit::Track* GFTrk = GFTracks[itrack];
      const genfit::TrackCand* cdc_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (cdc_TrkCandPtr == NULL) {
        continue;
      }
      //cdc_mcp_index = cdc_TrkCandPtr->getMcTrackId();
      GFTrk = GFTracks[jtrack + nCDCTracks];
      const genfit::TrackCand* vxd_TrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(GFTrk, m_TrackCandColName);
      if (vxd_TrkCandPtr == NULL) {
        continue;
      }*/
      //vxd_mcp_index = vxd_TrkCandPtr->getMcTrackId();

      /*LE
      for (std::vector<int>::iterator it = m_true_match_mc->begin(); it != m_true_match_mc->end(); ++it) {
        if ((*it) == cdc_mcp_index) {
          //std::cout << "Already matched guy: " << *it << " of " << m_true_match_mc->size() << std::endl;
          truth_flag = 1;
        }
      }
      */
      /*LE
      if ((cdc_mcp_index == vxd_mcp_index) && truth_flag == 0) { //Calculate for true matched tracks
        m_true_match_vec->push_back(1);
        m_true_match_mc->push_back(vxd_mcp_index);
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
      */
    }//end loop on VXD tracks

    if ((matched_track == 1) || (recovered_track == 1)) {
      if ((m_recover == 1) && (recovered_track == 1))
        rel_weight = 2;
      else
        rel_weight = 1;
      relMatchedTracks.add(itrack, vxd_match, rel_weight);
    }
  }//loop on CDC

  //B2INFO("VXDCDCTrackMerger: Merged Tracks: " << matches);

}

void VXDCDCTrackMergerModule::endRun()
{
}

void VXDCDCTrackMergerModule::terminate()
{
}
