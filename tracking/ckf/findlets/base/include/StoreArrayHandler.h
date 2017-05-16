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
#include <framework/core/ModuleParamList.h>

#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Findlet to handle import and export from the StoreArray.
   *
   * Its fetch function fills a vector of RecoTracks from the fitted tracks in an input store array
   * and additionally a hit vector.
   *
   * Its relateAndCombineTracks function can be used in derived classes, to export previously related
   * reco track candidates into a third store array.
   */
  template <class AStateObject>
  class StoreArrayHandler : public TrackFindingCDC::Findlet<const typename AStateObject::ResultObject> {
    using Super = TrackFindingCDC::Findlet<const typename AStateObject::ResultObject>;
    using HitObject = typename AStateObject::HitObject;

  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      // CDC input tracks
      moduleParamList->addParameter("CDCRecoTrackStoreArrayName", m_param_cdcRecoTrackStoreArrayName,
                                    "StoreArray name of the CDC Track Store Array", m_param_cdcRecoTrackStoreArrayName);
      // VXD input tracks
      moduleParamList->addParameter("VXDRecoTrackStoreArrayName", m_param_vxdRecoTrackStoreArrayName,
                                    "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);

      // combined output tracks
      moduleParamList->addParameter("MergedRecoTrackStoreArrayName", m_param_mergedRecoTrackStoreArrayName,
                                    "StoreArray name of the merged Track Store Array", m_param_mergedRecoTrackStoreArrayName);


      moduleParamList->addParameter("exportTracks", m_param_exportTracks, "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);
    }

    /// Require/register the store arrays
    void initialize() override
    {
      Super::initialize();

      m_cdcRecoTracks.isRequired(m_param_cdcRecoTrackStoreArrayName);
      m_hits.isRequired();

      m_vxdRecoTracks.registerInDataStore(m_param_vxdRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_vxdRecoTracks);

      m_mergedRecoTracks.registerInDataStore(m_param_mergedRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_mergedRecoTracks);

      m_cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);
      m_vxdRecoTracks.registerRelationTo(m_cdcRecoTracks);
    }

    /// Fetch the CDC RecoTracks and the hits from the input Store Arrays and fill them into a vector.
    void fetch(std::vector<RecoTrack*>& cdcRecoTrackVector, std::vector<const HitObject*>& hitVector)
    {
      cdcRecoTrackVector.reserve(cdcRecoTrackVector.size() + m_cdcRecoTracks.getEntries());

      for (RecoTrack& recoTrack : m_cdcRecoTracks) {
        if (recoTrack.wasFitSuccessful()) {
          cdcRecoTrackVector.push_back(&recoTrack);
        }
      }

      hitVector.reserve(hitVector.size() + m_hits.getEntries());

      for (const HitObject& hit : m_hits) {
        hitVector.push_back(&hit);
      }
    }

  protected:
    // Parameters
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName = "CKFVXDRecoTracks";
    /** StoreArray name of the CDC Track Store Array */
    std::string m_param_cdcRecoTrackStoreArrayName = "CDCRecoTracks";
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_mergedRecoTrackStoreArrayName = "MergedRecoTracks";
    /** Export the tracks or not */
    bool m_param_exportTracks = true;

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// Space Points Store Array
    StoreArray<HitObject> m_hits;
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Merged Reco Tracks Store Array
    StoreArray<RecoTrack> m_mergedRecoTracks;

    /// Helper function to get the seed or the measured state on plane from a track
    void extractTrackState(const RecoTrack& recoTrack, TVector3& position, TVector3& momentum, int& charge) const
    {
      if (recoTrack.getRepresentations().empty() or not recoTrack.wasFitSuccessful()) {
        position = recoTrack.getPositionSeed();
        momentum = recoTrack.getMomentumSeed();
        charge = recoTrack.getChargeSeed();
      } else {
        const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromFirstHit();
        position = measuredStateOnPlane.getPos();
        momentum = measuredStateOnPlane.getMom();
        charge = measuredStateOnPlane.getCharge();
      }
    }

    /// Extract a momentum and charge from a reco track at a given position.
    void extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition, TVector3& extrapolatedMomentum,
                             int& cdcCharge)
    {
      TVector3 cdcPosition;
      TVector3 cdcMomentum;

      extractTrackState(relatedCDCRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

      const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

      const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
      const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

      extrapolatedMomentum = cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);
    }

    /**
     * Combine the vxd and cdc tracks based on the relations between tracks in the Data Store.
     * Write all vxd tracks to StoreArray and add a CDC track, if there is one.
     */
    void relateAndCombineTracks();
  };

  /**
   * Merge related CDC and VXD tracks together and export them into a third store array.
   * Also add CDC tracks without a match.
   */
  template <class AStateObject>
  void StoreArrayHandler<AStateObject>::relateAndCombineTracks()
  {
    for (const RecoTrack& currentVXDTrack : m_vxdRecoTracks) {
      // track position will be filled with seed or fitted position of VXD track first.
      TVector3 trackPosition;
      TVector3 trackMomentum;
      int trackCharge;

      extractTrackState(currentVXDTrack, trackPosition, trackMomentum, trackCharge);

      // Now check for the related CDC track
      RecoTrack* relatedCDCRecoTrack = currentVXDTrack.template getRelated<RecoTrack>(m_param_cdcRecoTrackStoreArrayName);

      if (relatedCDCRecoTrack) {
        // We construct a helix out of the CDC track parameter and "extrapolate" it to the vxd start position to get
        // the momentum right
        extrapolateMomentum(*relatedCDCRecoTrack, trackPosition, trackMomentum, trackCharge);
      }

      RecoTrack* newRecoTrack = m_mergedRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);
      newRecoTrack->addHitsFromRecoTrack(&currentVXDTrack);

      if (relatedCDCRecoTrack) {
        newRecoTrack->addHitsFromRecoTrack(relatedCDCRecoTrack, newRecoTrack->getNumberOfTotalHits());
      }
    }

    // add all unmatched CDCTracks to the StoreArray as well
    for (const RecoTrack& currentCDCTrack : m_cdcRecoTracks) {
      RecoTrack* relatedVXDRecoTrack = currentCDCTrack.template getRelated<RecoTrack>(m_param_vxdRecoTrackStoreArrayName);

      if (not relatedVXDRecoTrack) {
        TVector3 cdcPosition;
        TVector3 cdcMomentum;
        int cdcCharge;

        extractTrackState(currentCDCTrack, cdcPosition, cdcMomentum, cdcCharge);

        auto newRecoTrack = m_mergedRecoTracks.appendNew(cdcPosition, cdcMomentum, cdcCharge);
        newRecoTrack->addHitsFromRecoTrack(&currentCDCTrack);
      }
    }
  }
}
