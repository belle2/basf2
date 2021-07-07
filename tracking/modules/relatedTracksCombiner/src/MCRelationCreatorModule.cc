/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/MCRelationCreatorModule.h>

using namespace Belle2;

REG_MODULE(MCRelationCreator);

MCRelationCreatorModule::MCRelationCreatorModule() :
  Module()
{
  setDescription("Create relations between MC-matched RecoTracks in CDC and SVD.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName , "Name of the input CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);
  addParam("VXDRecoTracksStoreArrayName", m_vxdRecoTracksStoreArrayName , "Name of the input VXD StoreArray.",
           m_vxdRecoTracksStoreArrayName);
}

void MCRelationCreatorModule::initialize()
{
  m_vxdRecoTracks.isRequired(m_vxdRecoTracksStoreArrayName);
  m_cdcRecoTracks.isRequired(m_cdcRecoTracksStoreArrayName);

  m_cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);

  m_cdcTrackMatchLookUp.reset(new TrackMatchLookUp("MCRecoTracks", m_cdcRecoTracks.getName()));
  m_vxdTrackMatchLookUp.reset(new TrackMatchLookUp("MCRecoTracks", m_vxdRecoTracks.getName()));
}

void MCRelationCreatorModule::event()
{
  for (const RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    const RecoTrack* matchedMCRecoTrack = m_cdcTrackMatchLookUp->getMatchedMCRecoTrack(cdcRecoTrack);
    if (not matchedMCRecoTrack) {
      continue;
    }

    const RecoTrack* matchedVXDRecoTrack = m_vxdTrackMatchLookUp->getMatchedPRRecoTrack(*matchedMCRecoTrack);
    if (not matchedVXDRecoTrack) {
      continue;
    }

    cdcRecoTrack.addRelationTo(matchedVXDRecoTrack, -1);
  }
}

