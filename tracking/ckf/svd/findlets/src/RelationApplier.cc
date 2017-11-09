/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/findlets/RelationApplier.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

void RelationApplier::apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>&
                            relationsCDCToSVD)
{
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : relationsCDCToSVD) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* svdTrack = relation.getTo();
    cdcTrack->addRelationTo(svdTrack);
  }
}