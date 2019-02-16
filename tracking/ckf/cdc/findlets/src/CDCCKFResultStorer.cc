/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CDCCKFResultStorer.h>

#include <tracking/ckf/general/utilities/SearchDirection.h>
#include <tracking/ckf/general/utilities/ClassMnemomics.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

void CDCCKFResultStorer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                "Export the result tracks into a StoreArray.",
                                m_param_exportTracks);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRecoTrackStoreArrayName"),
                                m_param_outputRecoTrackStoreArrayName,
                                "StoreArray name of the output Track Store Array.");
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRelationRecoTrackStoreArrayName"),
                                m_param_outputRelationRecoTrackStoreArrayName,
                                "StoreArray name of the tracks, the output reco tracks should be related to.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeOutDirection"),
                                m_param_writeOutDirectionAsString,
                                "Write out the relations with the direction of the CDC part as weight");
}

void CDCCKFResultStorer::initialize()
{
  Super::initialize();

  if (not m_param_exportTracks) {
    return;
  }

  m_outputRecoTracks.registerInDataStore(m_param_outputRecoTrackStoreArrayName);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  StoreArray<RecoTrack> relationRecoTracks(m_param_outputRelationRecoTrackStoreArrayName);
  relationRecoTracks.registerRelationTo(m_outputRecoTracks);

  m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);
}

void CDCCKFResultStorer::apply(const std::vector<CDCCKFResult>& results)
{
  for (const CDCCKFResult& result : results) {
    if (result.size() < 2) {
      continue;
    }

    const auto& trackState = result[1].getTrackState();
    const TVector3& trackPosition = trackState.getPos();
    const TVector3& trackMomentum = trackState.getMom();
    const double trackCharge = trackState.getCharge();

    RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);

    unsigned int sortingParameter = 0;
    for (const CDCCKFState& state : result) {
      if (state.isSeed()) {
        continue;
      }


      const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();

      auto rl = state.getRLinfo()  == TrackFindingCDC::ERightLeft::c_Right ?
                RecoHitInformation::RightLeftInformation::c_right :
                RecoHitInformation::RightLeftInformation::c_left;

      newRecoTrack->addCDCHit(wireHit->getHit(), sortingParameter, rl);

      sortingParameter++;
    }

    const RecoTrack* seed = result.front().getSeed();
    if (not seed) {
      continue;
    }

    seed->addRelationTo(newRecoTrack, m_param_writeOutDirection);
  }
}
