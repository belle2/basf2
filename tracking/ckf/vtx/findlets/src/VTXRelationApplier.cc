/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/findlets/VTXRelationApplier.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

void VTXRelationApplier::initialize()
{
  Super::initialize();

  StoreArray<RecoTrack> relationFromTracks(m_param_fromRelationsStoreArrayName);
  relationFromTracks.isRequired();

  StoreArray<RecoTrack> relationToTracks(m_param_toRelationsStoreArrayName);
  relationToTracks.isRequired();

  relationFromTracks.registerRelationTo(relationToTracks);

  m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);
}

void VTXRelationApplier::apply(const std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>>&
                               relationsCDCToVTX)
{
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : relationsCDCToVTX) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* vtxTrack = relation.getTo();
    cdcTrack->addRelationTo(vtxTrack, m_param_writeOutDirection);
  }
}

void VTXRelationApplier::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeOutDirection"),
                                m_param_writeOutDirectionAsString,
                                "Write out the relations with the direction of the VTX part as weight");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fromRelationStoreArrayName"), m_param_fromRelationsStoreArrayName,
                                "Create relations from this store array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toRelationStoreArrayName"), m_param_toRelationsStoreArrayName,
                                "Create relations to this store array.");
}
