/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/general/findlets/TrackLoader.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.icc.h>

using namespace Belle2;

TrackLoader::TrackLoader() : Super()
{
  this->addProcessingSignalListener(&m_trackFitter);
}

void TrackLoader::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackFitter.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "inputRecoTrackStoreArrayName"),
                                m_param_inputRecoTrackStoreArrayName,
                                "StoreArray name of the input Track Store Array.",
                                m_param_inputRecoTrackStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "relatedRecoTrackStoreArrayName"),
                                m_param_relationRecoTrackStoreArrayName,
                                "If not null, check for relations to this store array name and only use the unrelated ones",
                                m_param_relationRecoTrackStoreArrayName);

  moduleParamList->addParameter("minimalPtRequirement", m_param_minimalPtRequirement,
                                "Minimal Pt requirement for the input CDC tracks",
                                m_param_minimalPtRequirement);
}

void TrackLoader::initialize()
{
  Super::initialize();

  m_inputRecoTracks.isRequired(m_param_inputRecoTrackStoreArrayName);

  if (not m_param_relationRecoTrackStoreArrayName.empty()) {
    StoreArray<RecoTrack> relatedRecoTracks;
    relatedRecoTracks.isRequired(m_param_relationRecoTrackStoreArrayName);
  }
}

void TrackLoader::apply(std::vector<RecoTrack*>& seeds)
{
  seeds.reserve(seeds.size() + m_inputRecoTracks.getEntries());

  for (auto& item : m_inputRecoTracks) {
    if (m_param_relationRecoTrackStoreArrayName.empty() or
        not item.template getRelated<RecoTrack>(m_param_relationRecoTrackStoreArrayName)) {
      seeds.push_back(&item);
    }
  }

  const auto hasLowPt = [this](const auto & track) {
    return track->getMomentumSeed().Pt() < m_param_minimalPtRequirement;
  };
  TrackFindingCDC::erase_remove_if(seeds, hasLowPt);

  m_trackFitter.apply(seeds);
}
