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
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

void RelationApplier::initialize()
{
  Super::initialize();

  StoreArray<RecoTrack> relationFromTracks(m_param_fromRelationsStoreArrayName);
  relationFromTracks.isRequired();

  StoreArray<RecoTrack> relationToTracks(m_param_toRelationsStoreArrayName);
  relationToTracks.isRequired();

  relationFromTracks.registerRelationTo(relationToTracks);

  m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);
}

void RelationApplier::apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>&
                            relationsCDCToSVD)
{
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : relationsCDCToSVD) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* svdTrack = relation.getTo();
    cdcTrack->addRelationTo(svdTrack, m_param_writeOutDirection);
  }
}

void RelationApplier::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeOutDirection"),
                                m_param_writeOutDirectionAsString,
                                "Write out the relations with the direction of the VXD part as weight");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fromRelationStoreArrayName"), m_param_fromRelationsStoreArrayName,
                                "Create relations from this store array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toRelationStoreArrayName"), m_param_toRelationsStoreArrayName,
                                "Create relations to this store array.");
}