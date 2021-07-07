/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/results/VTXResultTruthVarSet.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool VTXResultTruthVarSet::extract(const CKFToVTXResult* result)
{
  const RecoTrack* seedTrack = result->getSeed();
  if (not seedTrack) return false;

  var<named("truth_event_number")>() = m_eventMetaData->getEvent();
  var<named("truth_seed_number")>() = seedTrack->getArrayIndex();

  const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
  const RecoTrack* mcTrack = mcCDCMatchLookUp.getRelatedMCRecoTrack(*seedTrack);

  // Default to false
  var<named("truth_number_of_correct_hits")>() = 0;
  var<named("truth")>() = 0;
  var<named("truth_vtx_cdc_relation")>() = NAN;
  var<named("truth_number_of_mc_vtx_hits")>() = 0;
  var<named("truth_number_of_mc_cdc_hits")>() = 0;

  if (not mcTrack) {
    // track is a fake.
    return true;
  }

  // Count the number of times the related MC-track is also related to the clusters.
  const unsigned int numberOfCorrectHits = m_mcUtil.getNumberOfCorrectVTXHits(mcTrack, result->getHits());

  var<named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
  var<named("truth_number_of_mc_vtx_hits")>() = mcTrack->getNumberOfVTXHits();
  var<named("truth_number_of_mc_cdc_hits")>() = mcTrack->getNumberOfCDCHits();

  var<named("truth")>() = static_cast<double>(numberOfCorrectHits) / static_cast<double>(mcTrack->getNumberOfVTXHits());

  const RecoTrack* relatedVTXRecoTrack = result->getRelatedVTXRecoTrack();
  if (relatedVTXRecoTrack) {
    const std::string& vtxTrackStoreArrayName = relatedVTXRecoTrack->getArrayName();

    TrackMatchLookUp mcVTXMatchLookUp("MCRecoTracks", vtxTrackStoreArrayName);
    const RecoTrack* vtxMCTrack = mcVTXMatchLookUp.getRelatedMCRecoTrack(*relatedVTXRecoTrack);

    // can not be a fake anyways
    var<named("truth_vtx_cdc_relation")>() = vtxMCTrack == mcTrack ? 1 : NAN;
  }

  return true;
}
