/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/general/findlets/TrackLoader.h>
#include <tracking/ckf/general/utilities/SearchDirection.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.h>

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
                                "StoreArray name of the input Track Store Array.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "relatedRecoTrackStoreArrayName"),
                                m_param_relationRecoTrackStoreArrayName,
                                "Check for relations to this store array name and only use the unrelated ones or "
                                "relations with different direction",
                                m_param_relationRecoTrackStoreArrayName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalPtRequirement"),
                                m_param_minimalPtRequirement,
                                "Minimal Pt requirement for the input tracks",
                                m_param_minimalPtRequirement);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "relationCheckForDirection"),
                                m_param_relationCheckForDirectionAsString,
                                "Check for this direction when checking for related tracks.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ignoreTracksWithCDChits"),
                                m_noCDChits, "Do not consider tracks containing CDC hits.", false);

}

void TrackLoader::initialize()
{
  Super::initialize();

  m_inputRecoTracks.isRequired(m_param_inputRecoTrackStoreArrayName);

  m_param_relationCheckForDirection = fromString(m_param_relationCheckForDirectionAsString);

  if (m_param_relationCheckForDirection != TrackFindingCDC::EForwardBackward::c_Invalid) {
    StoreArray<RecoTrack> relatedRecoTracks;
    if (not relatedRecoTracks.isOptional(m_param_relationRecoTrackStoreArrayName)) {
      m_param_relationCheckForDirection = TrackFindingCDC::EForwardBackward::c_Invalid;
    }
  }
}

void TrackLoader::apply(std::vector<RecoTrack*>& seeds)
{
  seeds.reserve(seeds.size() + m_inputRecoTracks.getEntries());

  for (auto& item : m_inputRecoTracks) {

    if (m_noCDChits) {
      if (item.hasCDCHits()) continue;
    }

    if (m_param_relationCheckForDirection != TrackFindingCDC::EForwardBackward::c_Invalid) {
      const auto& relatedTracksWithWeight = item.template getRelationsWith<RecoTrack>(m_param_relationRecoTrackStoreArrayName);
      bool hasAlreadyRelation = false;
      for (unsigned int index = 0; index < relatedTracksWithWeight.size(); ++index) {
        const RecoTrack* relatedTrack = relatedTracksWithWeight[index];
        const float weight = relatedTracksWithWeight.weight(index);
        if (relatedTrack and weight == static_cast<float>(m_param_relationCheckForDirection)) {
          hasAlreadyRelation = true;
          break;
        }
      }

      if (not hasAlreadyRelation) {
        seeds.push_back(&item);
      } else {
        B2DEBUG(29, "Do not use this track, because it has already a valid relation");
      }
    } else {
      seeds.push_back(&item);
    }
  }

  const auto hasLowPt = [this](const auto & track) {
    return track->getMomentumSeed().Rho() < m_param_minimalPtRequirement;
  };
  TrackFindingCDC::erase_remove_if(seeds, hasLowPt);

  m_trackFitter.apply(seeds);
}
