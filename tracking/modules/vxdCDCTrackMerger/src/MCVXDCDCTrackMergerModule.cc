/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/MCVXDCDCTrackMergerModule.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;

REG_MODULE(MCVXDCDCTrackMerger)


MCVXDCDCTrackMergerFindlet::MCVXDCDCTrackMergerFindlet()
{
  addProcessingSignalListener(&m_storeArrayMerger);
  addProcessingSignalListener(&m_bestMatchSelector);
  addProcessingSignalListener(&m_relationAdder);
}

void MCVXDCDCTrackMergerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_storeArrayMerger.exposeParameters(moduleParamList, prefix);
  m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
  m_relationAdder.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "onlyFittedCDCTracks"), m_param_onlyFittedCDCTracks,
                                "Only use fitted CDC tracks as otherwise the comparison with the CKF is unfair.",
                                m_param_onlyFittedCDCTracks);
}

void MCVXDCDCTrackMergerFindlet::apply()
{
  // Pick up items from the data store.
  std::vector<RecoTrack*> cdcRecoTrackVector;
  std::vector<RecoTrack*> vxdRecoTrackVector;

  m_storeArrayMerger.apply(cdcRecoTrackVector, vxdRecoTrackVector);

  // Prepare the weighted relations for later
  std::vector<TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>> weightedRelations;
  weightedRelations.reserve(cdcRecoTrackVector.size() * vxdRecoTrackVector.size());

  TrackMatchLookUp cdcMCLookUp("MCRecoTracks", m_storeArrayMerger.getCDCStoreArrayName());
  TrackMatchLookUp vxdMCLookUp("MCRecoTracks", m_storeArrayMerger.getVXDStoreArrayName());
  for (auto& cdcRecoTrack : cdcRecoTrackVector) {
    if (m_param_onlyFittedCDCTracks and not cdcRecoTrack->wasFitSuccessful()) {
      continue;
    }

    const RecoTrack* cdcMCRecoTrack = cdcMCLookUp.getMatchedMCRecoTrack(*cdcRecoTrack);

    if (not cdcMCRecoTrack) {
      continue;
    }

    for (auto& vxdRecoTrack : vxdRecoTrackVector) {
      const RecoTrack* vxdMCRecoTrack = vxdMCLookUp.getMatchedMCRecoTrack(*vxdRecoTrack);
      if (not vxdMCRecoTrack) {
        continue;
      }

      if (cdcMCRecoTrack != vxdMCRecoTrack) {
        continue;
      }


      const float matchedEfficiency = vxdMCLookUp.getMatchedEfficiency(*vxdMCRecoTrack);
      weightedRelations.emplace_back(&cdcRecoTrack, matchedEfficiency, &vxdRecoTrack);
    }
  }

  // Select the ones with the best efficiency
  m_bestMatchSelector.apply(weightedRelations);

  for (auto& relation : weightedRelations) {
    relation.setWeight(-1);
  }

  // Add the already found items from the filter-based decision
  m_relationAdder.apply(weightedRelations);
}