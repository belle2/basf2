/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/CKFDataHandler.dcl.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.icc.h>

namespace Belle2 {
  template <class AResult>
  /// Add the subfindlets
  CKFDataHandler<AResult>::CKFDataHandler() : Super()
  {
    this->addProcessingSignalListener(&m_trackFitter);
  }

  /// Expose the parameters of the sub findlets.
  template <class AResult>
  void CKFDataHandler<AResult>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_trackFitter.exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                  "Export the result tracks into a StoreArray.",
                                  m_param_exportTracks);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "inputRecoTrackStoreArrayName"),
                                  m_param_inputRecoTrackStoreArrayName,
                                  "StoreArray name of the input Track Store Array.",
                                  m_param_inputRecoTrackStoreArrayName);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRecoTrackStoreArrayName"),
                                  m_param_outputRecoTrackStoreArrayName,
                                  "StoreArray name of the output Track Store Array.",
                                  m_param_outputRecoTrackStoreArrayName);

    moduleParamList->addParameter("minimalPtRequirement", m_param_minimalPtRequirement,
                                  "Minimal Pt requirement for the input CDC tracks",
                                  m_param_minimalPtRequirement);
  }

  /// Create the store arrays
  template <class AResult>
  void CKFDataHandler<AResult>::initialize()
  {
    Super::initialize();

    m_inputRecoTracks.isRequired(m_param_inputRecoTrackStoreArrayName);

    if (not m_param_exportTracks) {
      return;
    }

    m_outputRecoTracks.registerInDataStore(m_param_outputRecoTrackStoreArrayName);
    RecoTrack::registerRequiredRelations(m_outputRecoTracks);

    StoreArray<RecoTrack> relationRecoTracks(m_param_inputRecoTrackStoreArrayName);
    relationRecoTracks.registerRelationTo(m_outputRecoTracks);
  }

  /// Load in the reco tracks and fit them
  template <class AResult>
  void CKFDataHandler<AResult>::apply(std::vector<RecoTrack*>& seeds)
  {
    seeds.reserve(seeds.size() + m_inputRecoTracks.getEntries());

    for (auto& item : m_inputRecoTracks) {
      if (not item.template getRelated<RecoTrack>(m_param_outputRecoTrackStoreArrayName)) {
        seeds.push_back(&item);
      }
    }

    const auto hasLowPt = [this](const auto & track) {
      return track->getMomentumSeed().Pt() < m_param_minimalPtRequirement;
    };
    TrackFindingCDC::erase_remove_if(seeds, hasLowPt);

    m_trackFitter.apply(seeds);
  }

  /// Store the reco tracks and the relations
  template <class AResult>
  void CKFDataHandler<AResult>::store(std::vector<AResult>& results)
  {
    if (not m_param_exportTracks) {
      return;
    }

    for (const auto& result : results) {

      const TVector3& trackPosition = result.getPosition();
      const TVector3& trackMomentum = result.getMomentum();
      const short& trackCharge = result.getCharge();

      RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);
      result.addToRecoTrack(*newRecoTrack);

      const RecoTrack* seed = result.getSeed();
      if (not seed) {
        continue;
      }
      seed->addRelationTo(newRecoTrack);
    }
  }
}
