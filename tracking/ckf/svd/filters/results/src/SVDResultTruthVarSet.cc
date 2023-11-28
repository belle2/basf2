/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/SVDResultTruthVarSet.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SVDResultTruthVarSet::extract(const CKFToSVDResult* result)
{
  const RecoTrack* seedTrack = result->getSeed();
  if (not seedTrack) return false;

  var<named("truth_event_number")>() = m_eventMetaData->getEvent();
  var<named("truth_seed_number")>() = seedTrack->getArrayIndex();

  const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getRelatedMCRecoTrack(*seedTrack);

  // Default to false
  var<named("truth_number_of_correct_hits")>() = 0;
  var<named("truth")>() = 0;
  var<named("truth_svd_cdc_relation")>() = NAN;
  var<named("truth_number_of_mc_pxd_hits")>() = 0;
  var<named("truth_number_of_mc_svd_hits")>() = 0;
  var<named("truth_number_of_mc_cdc_hits")>() = 0;

  if (not cdcMCTrack) {
    // track is a fake.
    return true;
  }

  // Count the number of times the related MC-track is also related to the clusters.
  const unsigned int numberOfCorrectHits = m_mcUtil.getNumberOfCorrectHits(cdcMCTrack, result->getHits());

  var<named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
  var<named("truth_number_of_mc_svd_hits")>() = cdcMCTrack->getNumberOfSVDHits();
  var<named("truth_number_of_mc_pxd_hits")>() = cdcMCTrack->getNumberOfPXDHits();
  var<named("truth_number_of_mc_cdc_hits")>() = cdcMCTrack->getNumberOfCDCHits();

  var<named("truth")>() = static_cast<double>(numberOfCorrectHits) / (static_cast<double>(cdcMCTrack->getNumberOfSVDHits()) / 2);

  const RecoTrack* relatedSVDRecoTrack = result->getRelatedSVDRecoTrack();
  if (relatedSVDRecoTrack) {
    const std::string& svdTrackStoreArrayName = relatedSVDRecoTrack->getArrayName();

    TrackMatchLookUp mcSVDMatchLookUp("MCRecoTracks", svdTrackStoreArrayName);
    const RecoTrack* svdMCTrack = mcSVDMatchLookUp.getRelatedMCRecoTrack(*relatedSVDRecoTrack);

    // can not be a fake anyways
    var<named("truth_svd_cdc_relation")>() = svdMCTrack == cdcMCTrack ? 1 : NAN;
  }

  return true;
}
