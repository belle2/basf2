/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/pxdSpacePoint/PXDTrackCombinationVarSet.h>
#include <tracking/ckf/states/CKFResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <cdc/dataobjects/CDCRecoHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool PXDTrackCombinationVarSet::extract(const BaseVXDTrackCombinationFilter::Object* result)
{
  RecoTrack* seedTrack = result->getSeed();
  if (not seedTrack) return false;

  const auto& svdHits = seedTrack->getSortedSVDHitList();
  const auto& cdcHits = seedTrack->getSortedCDCHitList();
  var<named("seed_svd_hits")>() = svdHits.size();
  if (svdHits.empty()) {
    var<named("seed_lowest_svd_layer")>() = -1;
  } else {
    var<named("seed_lowest_svd_layer")>() = svdHits.front()->getSensorID().getLayerNumber();
  }
  var<named("seed_cdc_hits")>() = cdcHits.size();
  if (cdcHits.empty()) {
    var<named("seed_lowest_cdc_layer")>() = -1;
  } else {
    var<named("seed_lowest_cdc_layer")>() = cdcHits.front()->getICLayer();
  }

  return true;
}
