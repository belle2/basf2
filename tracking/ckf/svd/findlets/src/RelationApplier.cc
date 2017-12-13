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

#include <framework/core/ModuleParamList.icc.h>

using namespace Belle2;

void RelationApplier::initialize()
{
  Super::initialize();

  StoreArray<RecoTrack> relationFromTracks(m_param_fromRelationsStoreArrayName);
  relationFromTracks.isRequired();

  StoreArray<RecoTrack> relationToTracks(m_param_toRelationsStoreArrayName);
  relationToTracks.isRequired();

  relationFromTracks.registerRelationTo(relationToTracks);
}

void RelationApplier::apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>&
                            relationsCDCToSVD)
{
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : relationsCDCToSVD) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* svdTrack = relation.getTo();
    if (m_param_reverseStoredRelations) {
      cdcTrack->addRelationTo(svdTrack, -1);
    } else {
      cdcTrack->addRelationTo(svdTrack);
    }
  }
}

void RelationApplier::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter("reverseStoredRelations", m_param_reverseStoredRelations,
                                "Write out the relations with a -1 as weight, indicating the reversal of the CDC track.",
                                m_param_reverseStoredRelations);

  moduleParamList->addParameter("fromRelationStoreArrayName", m_param_fromRelationsStoreArrayName,
                                "Create relations from this store array.",
                                m_param_fromRelationsStoreArrayName);

  moduleParamList->addParameter("toRelationStoreArrayName", m_param_toRelationsStoreArrayName,
                                "Create relations to this store array.",
                                m_param_toRelationsStoreArrayName);
}