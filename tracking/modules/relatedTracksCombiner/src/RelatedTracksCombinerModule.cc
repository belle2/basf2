/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/RelatedTracksCombinerModule.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <tracking/dataobjects/RecoHitInformation.h>

using namespace Belle2;

REG_MODULE(RelatedTracksCombiner);

RelatedTracksCombinerModule::RelatedTracksCombinerModule() :
  Module()
{
  setDescription("Combine related tracks from CDC and VXD into a single track by copying the hit "
                 "information and combining the seed information. The sign of the weight defines, "
                 "if the hits go before (-1) or after (+1) the CDC track.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName, "Name of the input CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);
  addParam("VXDRecoTracksStoreArrayName", m_vxdRecoTracksStoreArrayName, "Name of the input VXD StoreArray.",
           m_vxdRecoTracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
  addParam("keepOnlyGoodQITracks", m_param_onlyGoodQITracks, "Only keep good QI tracks", m_param_onlyGoodQITracks);
  addParam("minRequiredQuality", m_param_qiCutValue, "Minimum QI to keep tracks", m_param_qiCutValue);
}

void RelatedTracksCombinerModule::initialize()
{
  m_vxdRecoTracks.isRequired(m_vxdRecoTracksStoreArrayName);
  m_cdcRecoTracks.isRequired(m_cdcRecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_vxdRecoTracks);
  m_recoTracks.registerRelationTo(m_cdcRecoTracks);
}

void RelatedTracksCombinerModule::event()
{
  TrackFitter trackFitter;

  // Loop over all CDC reco tracks and add them to the store array if they do not have a match or combined them with
  // their VXD partners if they do.
  // For this, the fitted or seed state of the tracks is used - if they are already fitted or not.
  for (RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    const RelationVector<RecoTrack>& relatedVXDRecoTracks = cdcRecoTrack.getRelationsWith<RecoTrack>(m_vxdRecoTracksStoreArrayName);

    if (m_param_onlyGoodQITracks and cdcRecoTrack.getQualityIndicator() <=  m_param_qiCutValue) {
      continue;
    }

    std::vector<RecoTrack*> vxdTracksBefore;
    std::vector<RecoTrack*> vxdTracksAfter;

    for (unsigned int index = 0; index < relatedVXDRecoTracks.size(); ++index) {
      if (m_param_onlyGoodQITracks and relatedVXDRecoTracks[index]->getQualityIndicator() <=  m_param_qiCutValue) {
        continue;
      }

      const double weight = relatedVXDRecoTracks.weight(index);
      if (weight < 0) {
        vxdTracksBefore.push_back(relatedVXDRecoTracks[index]);
      } else if (weight > 0) {
        vxdTracksAfter.push_back(relatedVXDRecoTracks[index]);
      }
    }

    // Do not output non-fittable tracks
    if (vxdTracksAfter.empty() and vxdTracksBefore.empty() and not trackFitter.fit(cdcRecoTrack)) {
      continue;
    }

    // TODO: This is upgrade specific. Sort tracks before according to layer number of first VTX hit
    const auto sortByFirstVTXLayer = [](const RecoTrack * lhs, const RecoTrack * rhs) {
      return lhs->getVTXHitList()[0]->getSensorID().getLayerNumber() < rhs->getVTXHitList()[0]->getSensorID().getLayerNumber();
    };
    sort(vxdTracksBefore.begin(), vxdTracksBefore.end(), sortByFirstVTXLayer);

    RecoTrack* newMergedTrack = nullptr;

    if (not vxdTracksBefore.empty()) {
      newMergedTrack = vxdTracksBefore[0]->copyToStoreArray(m_recoTracks);
      newMergedTrack->addHitsFromRecoTrack(vxdTracksBefore[0], newMergedTrack->getNumberOfTotalHits());
      newMergedTrack->addRelationTo(vxdTracksBefore[0]);
      for (unsigned int index = 1; index < vxdTracksBefore.size(); ++index) {
        newMergedTrack->addHitsFromRecoTrack(vxdTracksBefore[index], newMergedTrack->getNumberOfTotalHits());
        newMergedTrack->addRelationTo(vxdTracksBefore[index]);
      }
    } else {
      newMergedTrack = cdcRecoTrack.copyToStoreArray(m_recoTracks);
    }

    newMergedTrack->addHitsFromRecoTrack(&cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());
    newMergedTrack->addRelationTo(&cdcRecoTrack);

    for (unsigned int index = 0; index < vxdTracksAfter.size(); ++index) {
      newMergedTrack->addHitsFromRecoTrack(vxdTracksAfter[index], newMergedTrack->getNumberOfTotalHits(), true);
      newMergedTrack->addRelationTo(vxdTracksAfter[index]);
    }
  }

  // Now we only have to add the VXD tracks without a match
  for (RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    if (m_param_onlyGoodQITracks and vxdRecoTrack.getQualityIndicator() <=  m_param_qiCutValue) {
      continue;
    }

    const RecoTrack* cdcRecoTrack = vxdRecoTrack.getRelated<RecoTrack>(m_cdcRecoTracksStoreArrayName);
    if (not cdcRecoTrack and trackFitter.fit(vxdRecoTrack)) {
      RecoTrack* newTrack = vxdRecoTrack.copyToStoreArray(m_recoTracks);
      newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
      newTrack->addRelationTo(&vxdRecoTrack);
    }
  }
}

