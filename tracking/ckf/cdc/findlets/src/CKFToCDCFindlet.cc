/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CKFToCDCFindlet.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToCDCFindlet::~CKFToCDCFindlet() = default;

CKFToCDCFindlet::CKFToCDCFindlet()
{
  addProcessingSignalListener(&m_trackHandler);
  addProcessingSignalListener(&m_treeSearcher);
}

void CKFToCDCFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_trackHandler.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>("preFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("basicFilter").setDefaultValue("rough");
  moduleParamList->getParameter<std::string>("extrapolationFilter").setDefaultValue("extrapolate_and_update");
  moduleParamList->getParameter<std::string>("finalFilter").setDefaultValue("distance");
}

void CKFToCDCFindlet::beginEvent()
{
  Super::beginEvent();

  m_vxdRecoTrackVector.clear();
}

void CKFToCDCFindlet::initialize()
{
  Super::initialize();

  StoreArray<RecoTrack> outputRecoTracks("CDCCKFRecoTracks");
  outputRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(outputRecoTracks);
}

void CKFToCDCFindlet::apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits)
{
  StoreArray<RecoTrack> outputRecoTracks("CDCCKFRecoTracks");

  m_trackHandler.apply(m_vxdRecoTrackVector);

  const auto& wireHitPtrs = TrackFindingCDC::as_pointers<const TrackFindingCDC::CDCWireHit>(wireHits);

  // Do the tree search
  for (const RecoTrack* recoTrack : m_vxdRecoTrackVector) {
    B2DEBUG(100, "Starting new seed");
    std::vector<CDCCKFPath> paths;

    CDCCKFState seedState(recoTrack, recoTrack->getMeasuredStateOnPlaneFromLastHit());
    paths.push_back({seedState});
    m_treeSearcher.apply(paths, wireHitPtrs);

    // TODO: do something with the paths
    for (const CDCCKFPath& path : paths) {
      const auto& result = path.back().getTrackState();
      const TVector3& trackPosition = result.getPos();
      const TVector3& trackMomentum = result.getMom();
      const double trackCharge = result.getCharge();

      RecoTrack* newRecoTrack = outputRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);

      unsigned int sortingParameter = 0;
      for (const CDCCKFState& state : path) {
        if (state.isSeed()) {
          continue;
        }

        // TODO: RL info + track finder info
        const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();
        newRecoTrack->addCDCHit(wireHit->getHit(), sortingParameter);

        sortingParameter++;
      }

      /*const RecoTrack* seed = path.front().getSeed();
      if (not seed) {
        continue;
      }
      //seed->addRelationTo(newRecoTrack);*/
    }
  }

  // TODO: write out the path candidates correctly
}
