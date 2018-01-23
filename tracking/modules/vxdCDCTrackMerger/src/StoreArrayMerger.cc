/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/StoreArrayMerger.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>


#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/dataobjects/Helix.h>

using namespace Belle2;

void StoreArrayMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  // CDC input tracks
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "CDCRecoTrackStoreArrayName"), m_param_cdcRecoTrackStoreArrayName,
                                "StoreArray name of the CDC Track Store Array", m_param_cdcRecoTrackStoreArrayName);
  // VXD input tracks
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "VXDRecoTrackStoreArrayName"), m_param_vxdRecoTrackStoreArrayName,
                                "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);
}

void StoreArrayMerger::initialize()
{
  m_cdcRecoTracks.isRequired(m_param_cdcRecoTrackStoreArrayName);
  m_vxdRecoTracks.isRequired(m_param_vxdRecoTrackStoreArrayName);

  m_cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);
  m_vxdRecoTracks.registerRelationTo(m_cdcRecoTracks);

  Super::initialize();
}

void StoreArrayMerger::apply(std::vector<RecoTrack*>& cdcRecoTrackVector, std::vector<RecoTrack*>& vxdRecoTrackVector)
{
  cdcRecoTrackVector.reserve(m_cdcRecoTracks.getEntries());
  vxdRecoTrackVector.reserve(m_vxdRecoTracks.getEntries());

  for (RecoTrack& recoTrack : m_cdcRecoTracks) {
    cdcRecoTrackVector.push_back(&recoTrack);
  }

  for (RecoTrack& recoTrack : m_vxdRecoTracks) {
    vxdRecoTrackVector.push_back(&recoTrack);
  }
}

void StoreArrayMerger::removeCDCRecoTracksWithPartner(std::vector<RecoTrack*>& tracks)
{
  removeRecoTracksWithPartner(tracks, m_param_vxdRecoTrackStoreArrayName);
}

void StoreArrayMerger::removeVXDRecoTracksWithPartner(std::vector<RecoTrack*>& tracks)
{
  removeRecoTracksWithPartner(tracks, m_param_cdcRecoTrackStoreArrayName);
}

void StoreArrayMerger::removeRecoTracksWithPartner(std::vector<RecoTrack*>& tracks, const std::string& partnerStoreArrayName)
{
  const auto& trackHasAlreadyRelations = [&partnerStoreArrayName](const RecoTrack * recoTrack) {
    return recoTrack->getRelated<RecoTrack>(partnerStoreArrayName) != nullptr;
  };

  TrackFindingCDC::erase_remove_if(tracks, trackHasAlreadyRelations);
}
