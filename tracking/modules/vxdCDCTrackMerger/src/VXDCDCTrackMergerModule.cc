/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Oberhof, Thomas Hauth                          *
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

#include <limits>

using namespace Belle2;

REG_MODULE(VXDCDCTrackMerger)

VXDCDCTrackMergerModule::VXDCDCTrackMergerModule() :
  Module(), m_CDC_wall_radius(16.25)
{
  setDescription(
    "This module merges tracks which are reconstructed, separately, in the silicon (PXD+VXD) and in the CDC");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("CDCRecoTrackColName", m_CDCRecoTrackColName, "CDC Reco Tracks");
  addParam("VXDRecoTrackColName", m_VXDRecoTrackColName, "VXD Reco Tracks");

  //output
  addParam("MergedRecoTrackColName", m_mergedRecoTrackColName, "Merged Reco Tracks");

  //merging parameters
  addParam("merge_radius", m_merge_radius,
           "Maximum distance between extrapolated tracks on the CDC wall",
           double(2.0));
}

void VXDCDCTrackMergerModule::initialize()
{
  m_CDCRecoTracks.isRequired(m_CDCRecoTrackColName);
  m_VXDRecoTracks.isRequired(m_VXDRecoTrackColName);

  m_mergedRecoTracks.registerInDataStore(m_mergedRecoTrackColName);

  // register all required relations to be able to use RecoTrack
  // and its relations
  RecoTrack::registerRequiredRelations(m_mergedRecoTracks);

  m_CDCRecoTracks.registerRelationTo(m_VXDRecoTracks);
  m_VXDRecoTracks.registerRelationTo(m_CDCRecoTracks);
}

void VXDCDCTrackMergerModule::event()
{
  //get CDC tracks
  unsigned int nCDCTracks = m_CDCRecoTracks.getEntries();
  B2DEBUG(9, "VXDCDCTrackMerger: input Number of CDC Tracks: " << nCDCTracks);

  //get VXD tracks
  unsigned int nVXDTracks = m_VXDRecoTracks.getEntries();
  B2DEBUG(9,
          "VXDCDCTrackMerger: input Number of VXD Tracks: " << nVXDTracks);

  // position and momentum used for extrapolations to the CDC Wall
  TVector3 position(0., 0., 0.);
  TVector3 momentum(0., 0., 1.);
  // position and momentum of the track extrapolated from the CDC fit to the CDC Wall
  TVector3 cdcpos;
  TVector3 cdcmom;
  // position and momentum of the track extrapolated from the VXD fit to the CDC Wall
  TVector3 vxdpos;
  TVector3 vxdmom;

  int currentCdcTrack = -1;

  // list holding the indices of all matched VXD and CDC tracks
  MatchPairList vxdCdcMatched;

  for (auto& cdcTrack : m_CDCRecoTracks) {
    currentCdcTrack++;

    // skip CDC Tracks which were not properly fitted
    if (!cdcTrack.wasFitSuccessful())
      continue;

    bool matched_track = false;

    // start search with worst chi^2 imaginable
    double this_chi_2 = std::numeric_limits<double>::max();
    double chi_2_max_this_cdc_track = std::numeric_limits<double>::max();
    try {
      genfit::MeasuredStateOnPlane cdc_sop =
        cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
      cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      cdcpos = cdc_sop.getPos();
      cdcmom = cdc_sop.getMom();
    } catch (...) {
      B2DEBUG(9, "CDCTrack extrapolation to cylinder failed!");
      continue;
    }

    // TODO: this can be done indepentend of each other ....
    int currentVxdTrack = -1;
    int bestMatchedVxdTrack = 0;
    for (auto& vxdTrack : m_VXDRecoTracks) {
      currentVxdTrack++;

      // skip VXD Tracks which were not properly fitted
      if (!vxdTrack.wasFitSuccessful()) {
        continue;
      }

      try {
        genfit::MeasuredStateOnPlane vxd_sop =
          vxdTrack.getMeasuredStateOnPlaneFromLastHit();
        vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
        vxdpos = vxd_sop.getPos();
        vxdmom = vxd_sop.getMom();
      } catch (genfit::Exception const&) {
        // extrapolation not possible, skip this track
        B2DEBUG(9, "VXDTrack extrapolation to cylinder failed!");
        continue;
      }

      // Extrapolate tracks to same plane & Match Tracks
      if (TMath::Sqrt((cdcpos - vxdpos) * (cdcpos - vxdpos)) < m_merge_radius) {
        genfit::MeasuredStateOnPlane vxd_sop;
        genfit::MeasuredStateOnPlane cdc_sop;
        try {
          vxd_sop = vxdTrack.getMeasuredStateOnPlaneFromLastHit();
          cdc_sop = cdcTrack.getMeasuredStateOnPlaneFromFirstHit();
          cdc_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
          vxd_sop.extrapolateToPlane(cdc_sop.getPlane());
        } catch (genfit::Exception const&) {
          // extrapolation not possible, skip this track
          B2DEBUG(9, "VXDTrack extrapolation to plane failed!");
          continue;
        }

        try {
          TMatrixDSym inv_covmtrx = (vxd_sop.getCov() + cdc_sop.getCov()).Invert();
          TVectorD state_diff = cdc_sop.getState() - vxd_sop.getState();
          state_diff *= inv_covmtrx;
          this_chi_2 = state_diff * (cdc_sop.getState() - vxd_sop.getState());
        } catch (...) {
          B2WARNING("Matrix is singular!");
          continue;
        }

        if ((this_chi_2 < chi_2_max_this_cdc_track) && (this_chi_2 > 0)) {
          matched_track = true;
          chi_2_max_this_cdc_track = this_chi_2;
          bestMatchedVxdTrack = currentVxdTrack;
        }
      }    //If on radius
    }    //end loop on VXD tracks

    if (matched_track) {
      vxdCdcMatched.push_back(std::make_pair(bestMatchedVxdTrack, currentCdcTrack));
    }
  }    //loop on CDC

  // add merged tracks to output collection
  for (auto match : vxdCdcMatched) {
    auto vxdTrack = m_VXDRecoTracks[match.first];
    auto cdcTrack = m_CDCRecoTracks[match.second];
    auto mergedRecoTrack = m_mergedRecoTracks.appendNew(vxdTrack->getPositionSeed(), vxdTrack->getMomentumSeed(),
                                                        vxdTrack->getChargeSeed());
    mergedRecoTrack->addHitsFromRecoTrack(vxdTrack);
    mergedRecoTrack->addHitsFromRecoTrack(cdcTrack, mergedRecoTrack->getNumberOfTotalHits());

    m_VXDRecoTracks[match.first]->addRelationTo(m_CDCRecoTracks[match.second]);
  }

  auto mergedTracksCount = m_mergedRecoTracks.getEntries();
  auto singleVxdTracksCount = addUnmatchedTracks(m_VXDRecoTracks, vxdCdcMatched, true);
  auto singleCdcTracksCount = addUnmatchedTracks(m_CDCRecoTracks, vxdCdcMatched, false);

  B2DEBUG(9, "VXDCDCTrackMerger: Merged Tracks: " << mergedTracksCount << " VXD only: " << singleVxdTracksCount << " CDC only: " <<
          singleCdcTracksCount);
}

size_t VXDCDCTrackMergerModule::addUnmatchedTracks(StoreArray<RecoTrack>& singleRecoTracks,
                                                   MatchPairList const& matchedList, bool useFirstIndex)
{
  int currentTrack = -1;
  size_t addCount = 0;
  for (auto& singleTrack : singleRecoTracks) {
    currentTrack++;
    // are there any matches which used this tracks index ?
    if (std::count_if(matchedList.begin(), matchedList.end(),
    [currentTrack, useFirstIndex](MatchPairType const & mp) {
    if (useFirstIndex)
        return currentTrack == mp.first;
      else
        return currentTrack == mp.second;
    }) == 0) {

      // make sure the track could be properly fit
      if (!singleTrack.wasFitSuccessful())
        continue;
      // this track has not been used by any matchet combinations, add it
      auto mergedRecoTrack = m_mergedRecoTracks.appendNew(singleTrack.getPositionSeed(), singleTrack.getMomentumSeed(),
                                                          singleTrack.getChargeSeed());
      mergedRecoTrack->addHitsFromRecoTrack(&singleTrack);
      addCount++;
    }

  }

  return addCount;
}

