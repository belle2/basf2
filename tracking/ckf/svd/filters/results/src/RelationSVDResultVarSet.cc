/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/results/RelationSVDResultVarSet.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParamList.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool RelationSVDResultVarSet::extract(const CKFToSVDResult* result)
{
  const RecoTrack* svdTrack = result->getRelatedSVDRecoTrack();
  B2ASSERT("Should have a related SVD track at this stage;", svdTrack);

  const auto& svdHits = svdTrack->getSVDHitList();
  B2ASSERT("SVD hits must be present", not svdHits.empty());

  const auto sortBySVDLayer = [](const SVDCluster * lhs, const SVDCluster * rhs) {
    return lhs->getSensorID().getLayerNumber() < rhs->getSensorID().getLayerNumber();
  };

  const auto& firstSVDHitIterator = std::max_element(svdHits.begin(), svdHits.end(), sortBySVDLayer);
  var<named("svd_highest_layer")>() = (*firstSVDHitIterator)->getSensorID().getLayerNumber();

  const RecoTrack* relatedSVDRecoTrack = result->getRelatedSVDRecoTrack();
  if (relatedSVDRecoTrack) {
    var<named("number_of_hits_related_svd_track")>() = relatedSVDRecoTrack->getNumberOfSVDHits();
  } else {
    var<named("number_of_hits_related_svd_track")>() = -1;
  }
  return true;
}
