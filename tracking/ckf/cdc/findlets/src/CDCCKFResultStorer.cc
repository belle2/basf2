/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CDCCKFResultStorer.h>

#include <tracking/ckf/general/utilities/SearchDirection.h>
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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "trackFindingDirection"),
                                m_param_trackFindingDirectionAsString,
                                "Direction in which the track is reconstructed (SVD/ECL/VTX seed)",
                                m_param_trackFindingDirectionAsString);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportAllTracks"),
                                m_param_exportAllTracks,
                                "Export all tracks, even if they did not reach the center of the CDC",
                                m_param_exportAllTracks);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "seedComponent"),
                                m_seedComponentString,
                                "Where does the seed track come from (typically SVD, ECL, VTX)",
                                m_seedComponentString);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "setTakenFlag"),
                                m_param_setTakenFlag,
                                "Set flag that hit is taken",
                                m_param_setTakenFlag);
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
  m_outputRecoTracks.registerRelationTo(relationRecoTracks);

  m_param_writeOutDirection = fromString(m_param_writeOutDirectionAsString);

  m_param_trackFindingDirection = fromString(m_param_trackFindingDirectionAsString);

  if (m_seedComponentString == "SVD") {
    m_trackFinderType = RecoHitInformation::c_SVDtoCDCCKF;
  } else if (m_seedComponentString == "ECL") {
    m_trackFinderType = RecoHitInformation::c_ECLtoCDCCKF;
  } else if (m_seedComponentString == "VTX") {
    m_trackFinderType = RecoHitInformation::c_VTXtoCDCCKF;
  } else {
    B2FATAL("CDCCKFResultStorer: No valid seed component specified. Please use SVD/ECL/VTX.");
  }
}

void CDCCKFResultStorer::apply(const std::vector<CDCCKFResult>& results)
{
  for (const CDCCKFResult& result : results) {
    if (result.size() < 2) {
      continue;
    }

    genfit::MeasuredStateOnPlane const* trackState = nullptr;
    if (m_param_trackFindingDirection == TrackFindingCDC::EForwardBackward::c_Forward) {
      trackState = &result.at(1).getTrackState();
    } else if (m_param_trackFindingDirection == TrackFindingCDC::EForwardBackward::c_Backward) {
      trackState = &result.back().getTrackState();
    } else {
      B2FATAL("CDCCKFResultStorer: No valid direction specified. Please use forward/backward.");
    }

    // only accept paths that reached the center of the CDC (for ECL seeding)
    if (not m_param_exportAllTracks
        && m_param_trackFindingDirection == TrackFindingCDC::EForwardBackward::c_Backward
        && result.back().getWireHit()->getWire().getICLayer() > 2) {
      continue;
    }

    const TVector3& trackPosition = trackState->getPos();
    const TVector3& trackMomentum = trackState->getMom();
    const double trackCharge = trackState->getCharge();

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

      newRecoTrack->addCDCHit(wireHit->getHit(), sortingParameter, rl, m_trackFinderType);
      sortingParameter++;

      if (m_param_setTakenFlag) {
        wireHit->getAutomatonCell().setTakenFlag();
      }
    }

    const RecoTrack* seed = result.front().getSeed();
    if (not seed) {
      continue;
    }

    seed->addRelationTo(newRecoTrack, m_param_writeOutDirection);
    newRecoTrack->addRelationTo(seed, m_param_writeOutDirection);
  }
}
