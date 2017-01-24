/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

namespace Belle2 {
  class StoreArrayMerger : public TrackFindingCDC::Findlet<> {
  public:
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      // input
      moduleParamList->addParameter("CDCRecoTrackStoreArrayName", m_param_cdcRecoTrackStoreArrayName,
                                    "StoreArray name of the CDC Track Store Array", m_param_cdcRecoTrackStoreArrayName);
      moduleParamList->addParameter("VXDRecoTrackStoreArrayName", m_param_vxdRecoTrackStoreArrayName,
                                    "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);

      // output
      moduleParamList->addParameter("MergedRecoTrackStoreArrayName", m_param_mergedRecoTrackStoreArrayName,
                                    "StoreArray name of the merged Track Store Array", m_param_mergedRecoTrackStoreArrayName);
    }

    void initialize() override
    {
      m_cdcRecoTracks.isRequired(m_param_cdcRecoTrackStoreArrayName);
      m_vxdRecoTracks.isRequired(m_param_vxdRecoTrackStoreArrayName);
      m_mergedRecoTracks.registerInDataStore(m_param_mergedRecoTrackStoreArrayName);

      // register all required relations to be able to use RecoTrack
      // and its relations
      RecoTrack::registerRequiredRelations(m_mergedRecoTracks);

      m_cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);
      m_vxdRecoTracks.registerRelationTo(m_cdcRecoTracks);

      TrackFindingCDC::Findlet<>::initialize();
    }

    /**
     */
    void fetch(std::vector<RecoTrack*>& cdcRecoTrackVector, std::vector<RecoTrack*>& vxdRecoTrackVector)
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

    void apply() override
    {
      // Combine the vxd and cdc tracks based on the relations between tracks in the Data Store
      // write all vxd tracks to StoreArray and add a CDC track, if there is one.
      for (const auto& currentVXDTrack : m_vxdRecoTracks) {

        auto relatedCDCRecoTrack = currentVXDTrack.getRelated<RecoTrack>(m_param_cdcRecoTrackStoreArrayName);

        //add related RecoTracks to VXD tracks if merging was successful
        if (relatedCDCRecoTrack) {
          // We construct a helix out of the CDC track parameter and "extrapolate" it to the vxd start position to get
          // the momentum right
          // TODO: If the track was already fitted, it may be better to just use the fitted position here...
          const auto& vxdPosition = currentVXDTrack.getPositionSeed();
          const auto& cdcPosition = relatedCDCRecoTrack->getPositionSeed();
          const auto& cdcMomentum = relatedCDCRecoTrack->getMomentumSeed();
          const auto& charge = relatedCDCRecoTrack->getChargeSeed();

          const auto bField = BFieldManager::getField(cdcPosition).Z();

          const Helix cdcHelix(cdcPosition, cdcMomentum, charge, bField);
          const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

          const auto& momentum = cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);

          auto newRecoTrack = m_mergedRecoTracks.appendNew(vxdPosition,
                                                           momentum,
                                                           charge);
          newRecoTrack->addHitsFromRecoTrack(&currentVXDTrack);
          newRecoTrack->addHitsFromRecoTrack(relatedCDCRecoTrack, newRecoTrack->getNumberOfTotalHits());
        } else {
          // And not if not...
          auto newRecoTrack = m_mergedRecoTracks.appendNew(currentVXDTrack.getPositionSeed(),
                                                           currentVXDTrack.getMomentumSeed(),
                                                           currentVXDTrack.getChargeSeed());
          newRecoTrack->addHitsFromRecoTrack(&currentVXDTrack);
        }
      }

      // add all unmatched CDCTracks to the StoreArray as well
      for (const auto& currentCDCTrack : m_cdcRecoTracks) {
        auto relatedVXDRecoTrack = currentCDCTrack.getRelated<RecoTrack>(m_param_vxdRecoTrackStoreArrayName);
        if (not relatedVXDRecoTrack) {
          auto newRecoTrack = m_mergedRecoTracks.appendNew(currentCDCTrack.getPositionSeed(), currentCDCTrack.getMomentumSeed(),
                                                           currentCDCTrack.getChargeSeed());
          newRecoTrack->addHitsFromRecoTrack(&currentCDCTrack);
        }
      }
    }

    void removeCDCRecoTracksWithPartner(std::vector<RecoTrack*>& tracks)
    {
      removeRecoTracksWithPartner(tracks, m_param_vxdRecoTrackStoreArrayName);
    }

    void removeVXDRecoTracksWithPartner(std::vector<RecoTrack*>& tracks)
    {
      removeRecoTracksWithPartner(tracks, m_param_cdcRecoTrackStoreArrayName);
    }

  private:
    void removeRecoTracksWithPartner(std::vector<RecoTrack*>& tracks, const std::string& partnerStoreArrayName)
    {
      const auto& trackHasAlreadyRelations = [&partnerStoreArrayName](const RecoTrack * recoTrack) {
        return recoTrack->getRelated<RecoTrack>(partnerStoreArrayName) != nullptr;
      };

      TrackFindingCDC::erase_remove_if(tracks, trackHasAlreadyRelations);
    }

    // Parameters
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName;
    /** StoreArray name of the CDC Track Store Array */
    std::string m_param_cdcRecoTrackStoreArrayName;
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_mergedRecoTrackStoreArrayName;

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Merged Reco Tracks Store Array
    StoreArray<RecoTrack> m_mergedRecoTracks;
  };
}