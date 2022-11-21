/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/results/PXDResultTruthVarSet.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool PXDResultTruthVarSet::extract(const CKFToPXDResult* result)
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
  var<named("truth_number_of_mc_pxd_hits")>() = 0;
  var<named("truth_number_of_mc_svd_hits")>() = 0;
  var<named("truth_number_of_mc_cdc_hits")>() = 0;

  if (not mcTrack) {
    // track is a fake.
    return true;
  }

  // Count the number of times the related MC-track is also related to the clusters.
  const unsigned int numberOfCorrectHits = m_mcUtil.getNumberOfCorrectPXDHits(mcTrack, result->getHits());

  var<named("truth_number_of_correct_hits")>() = numberOfCorrectHits;
  var<named("truth_number_of_mc_pxd_hits")>() = mcTrack->getNumberOfPXDHits();
  var<named("truth_number_of_mc_svd_hits")>() = mcTrack->getNumberOfSVDHits();
  var<named("truth_number_of_mc_cdc_hits")>() = mcTrack->getNumberOfCDCHits();

  var<named("truth")>() = static_cast<double>(numberOfCorrectHits) / static_cast<double>(mcTrack->getNumberOfPXDHits());

  return true;
}
