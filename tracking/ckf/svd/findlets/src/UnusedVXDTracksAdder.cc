/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/findlets/UnusedVXDTracksAdder.h>
#include <framework/core/ModuleParamList.h>

#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

void UnusedVXDTracksAdder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "temporaryVXDTracksStoreArrayName"),
                                m_param_temporaryVXDTracksStoreArrayName,
                                "The name of the store array produced by VXDTF2",
                                m_param_temporaryVXDTracksStoreArrayName);
}

void UnusedVXDTracksAdder::initialize()
{
  Super::initialize();

  m_vxdRecoTracks.isRequired(m_param_temporaryVXDTracksStoreArrayName);
}

void UnusedVXDTracksAdder::beginEvent()
{
  Super::beginEvent();

  m_usedVXDRecoTracks.clear();
}

void UnusedVXDTracksAdder::apply(std::vector<CKFToSVDResult>& results)
{
  // Also add the VXDRecoTracks, which do not have any partner
  for (const RecoTrack& recoTrack : m_vxdRecoTracks) {
    m_usedVXDRecoTracks.emplace(&recoTrack, false);
  }

  for (const CKFToSVDResult& result : results) {
    const std::vector<const SpacePoint*>& spacePoints = result.getHits();
    if (spacePoints.empty()) {
      continue;
    }
    const SVDCluster* relatedSVDCluster = spacePoints.front()->getRelated<SVDCluster>();
    const RecoTrack* recoTrack = relatedSVDCluster->getRelated<RecoTrack>(m_param_temporaryVXDTracksStoreArrayName);
    m_usedVXDRecoTracks[recoTrack] = true;
  }

  for (const auto& pair : m_usedVXDRecoTracks) {
    const bool used = pair.second;
    if (not used) {
      const RecoTrack* recoTrack = pair.first;
      SpacePointTrackCand* spacePointTrackCand = recoTrack->getRelated<SpacePointTrackCand>("SPTrackCands");
      std::vector<const SpacePoint*> hits = spacePointTrackCand->getSortedHits();

      // TODO: maybe implement a flag, if the hits are reversed or not - otherwise it would be more consistent...
      std::reverse(hits.begin(), hits.end());

      results.emplace_back(nullptr, hits, -1, spacePointTrackCand->getPosSeed(),
                           spacePointTrackCand->getMomSeed(), spacePointTrackCand->getChargeSeed());
    }
  }
}
