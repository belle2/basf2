/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Oberhof                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("VXDGFTracksColName", m_VXDGFTracksColName, "VXD GFTrack collection");
  addParam("VXDGFTrackCandsColName", m_VXDGFTrackCandsColName, "VXD GFTrackCand collection", std::string(""));

  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC GFTrack collection");
  addParam("CDCGFTrackCandsColName", m_CDCGFTrackCandsColName, "CDC GFTrackCand collection", std::string(""));

  addParam("MergedGFTrackCandsColName", m_mergedGFTrackCandsColName,
           "Merged GFTrackCand collection", std::string(""));

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

  m_VXDGFTracks.isRequired(m_VXDGFTracksColName);
  m_VXDGFTrackCands.isOptional(m_VXDGFTrackCandsColName);
  m_VXDGFTrackCands.optionalRelationTo(m_VXDGFTracks);

  m_CDCGFTracks.isRequired(m_CDCGFTracksColName);
  m_CDCGFTrackCands.isOptional(m_CDCGFTrackCandsColName);
  m_CDCGFTrackCands.optionalRelationTo(m_CDCGFTracks);

  // Let us register the results of our work
  m_CDCGFTracks.registerRelationTo(m_VXDGFTracks);

  m_TrackCands.registerInDataStore(m_mergedGFTrackCandsColName);

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
  B2DEBUG(9, "VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("VXDCDCTrackMerger: CDCGFTracksCollection is empty!");

  //get VXD tracks
  StoreArray<genfit::Track> VXDGFTracks(m_VXDGFTracksColName);
  unsigned int nVXDTracks = VXDGFTracks.getEntries();
  B2DEBUG(9, "VXDCDCTrackMerger: input Number of Silicon Tracks: " << nVXDTracks);
  if (nVXDTracks == 0) B2WARNING("VXDCDCTrackMerger: VXDGFTracksCollection is empty!");

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

    }//end loop on VXD tracks

    if ((matched_track == 1) || (recovered_track == 1)) {
      if ((m_recover == 1) && (recovered_track == 1))
        rel_weight = 2;
      else
        rel_weight = 1;
      relMatchedTracks.add(itrack, vxd_match, rel_weight);

    }


  }//loop on CDC

  collectMergedTrackCands();

}

void VXDCDCTrackMergerModule::endRun()
{
}

void VXDCDCTrackMergerModule::terminate()
{
}

void VXDCDCTrackMergerModule::collectMergedTrackCands(void)
{
  using namespace genfit;
  // Reminder: GF stands for Genfit
  // genfit::TrackCand is a sorted collection of hits
  // genfit::Track is the result of a fit og a TrackCand


  RelationIndex<TrackCand, Track>
  VXDTrackCandToTrack(m_VXDGFTrackCands, m_VXDGFTracks);

  // get CDC TrackCands, Tracks and relation from TrackCands to Tracks
  RelationIndex<TrackCand, Track>
  CDCTrackCandToTrack(m_CDCGFTrackCands, m_CDCGFTracks);

  // get relation from CDC to VXD track
  RelationIndex<Track, Track> relMatchedTracks(m_CDCGFTracks, m_VXDGFTracks);

  // create the TrackCand output list

  m_TrackCands.create();

  int nVXDonly(0);
  int nCDConly(0);
  int nMerged(0);
  // first: add VXD tracks (merged with CDC or unmerged)
  for (int i = 0 ; i < m_VXDGFTrackCands.getEntries() ; i++) {

    auto VXDGFTrackCand = m_VXDGFTrackCands[i];

    auto combinedTrackCand = m_TrackCands.appendNew(*VXDGFTrackCand);

    auto relation1 = VXDTrackCandToTrack.getFirstElementFrom(VXDGFTrackCand);
    if (! relation1) {
      B2WARNING("No relation from VXD genfit::TrackCand to VXD genfit::Track");
      continue;
    }
    const genfit::Track* VXDGFTrack = relation1 -> to ;  // this is the VXD fitted track

    auto relation2 = relMatchedTracks.getFirstElementTo(VXDGFTrack);
    if (! relation2) { // VXD track without a CDC match
      nVXDonly ++;
      continue;
    }

    const genfit::Track* CDCGFTrack = relation2 -> from ;  // this is the CDC track

    auto relation3 = CDCTrackCandToTrack.getFirstElementTo(CDCGFTrack);
    if (! relation3) {
      B2WARNING("No relation from CDC genfit::TrackCand to CDC genfit::Track");
      continue;
    }
    const genfit::TrackCand* CDCGFTrackCand = relation3 -> from ;  // this is the CDC pattern

    int nCdcCandHits = CDCGFTrackCand->getNHits();
    for (int i = 0 ; i < nCdcCandHits; ++i)
      combinedTrackCand->addHit(CDCGFTrackCand->getHit(i)->clone());
    nMerged ++;

  }


  // second: add CDC tracks that are not merged from PXD tracks

  for (int i = 0 ; i < m_CDCGFTracks.getEntries() ; i++) {

    auto CDCGFTrack = m_CDCGFTracks[i];

    if (relMatchedTracks.getFirstElementFrom(CDCGFTrack))
      continue; // TrackCand already merged

    auto relation = CDCTrackCandToTrack.getFirstElementTo(CDCGFTrack);
    if (! relation) {
      B2WARNING("No relation from CDC genfit::TrackCand to CDC genfit::Track");
      continue;
    }
    const genfit::TrackCand* CDCGFTrackCand = relation -> from ;  // this is the CDC pattern

    m_TrackCands.appendNew(*CDCGFTrackCand);
    nCDConly++;
  }

  B2DEBUG(9, "VXDCDCTrackMerger: VXD only tracks: " << nVXDonly);
  B2DEBUG(9, "VXDCDCTrackMerger: CDC only tracks: " << nCDConly);
  B2DEBUG(9, "VXDCDCTrackMerger: merged tracks  : " << nMerged);
}
