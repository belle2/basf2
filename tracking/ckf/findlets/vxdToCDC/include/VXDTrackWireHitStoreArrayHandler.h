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

#include <tracking/ckf/states/CKFResult.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /**
   * Store array handler for VXD RecoTrack seeds and Wire hits.
   */
  template<class ASeed, class AHitObject>
  class VXDTrackWireHitStoreArrayHandler : public TrackFindingCDC::Findlet<const CKFResult<ASeed, AHitObject>> {
    using Super = TrackFindingCDC::Findlet<const CKFResult<ASeed, AHitObject>>;

  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                    "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "CDCRecoTrackStoreArrayName"), m_param_cdcRecoTrackStoreArrayName,
                                    "StoreArray name of the output CDC Track Store Array", m_param_cdcRecoTrackStoreArrayName);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "debuggingRelationsTo"), m_param_debuggingRelationsTo,
                                    "Add a relation to the VXD tracks given in this store array name (or none to not add relations).",
                                    m_param_debuggingRelationsTo);
    }

    /// Require/register the store arrays
    void initialize() override
    {
      Super::initialize();

      m_cdcRecoTracks.registerInDataStore(m_param_cdcRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_cdcRecoTracks);

      if (not m_param_debuggingRelationsTo.empty()) {
        StoreArray<RecoTrack> vxdRecoTracks(m_param_debuggingRelationsTo);
        vxdRecoTracks.registerRelationTo(m_cdcRecoTracks);
      }
    }


    /**
     * Write back the found tracks (CDC only and the merged ones).
     */
    void apply(const std::vector<CKFResult<ASeed, AHitObject>>& vxdTracksWithMatchedWireHits) override
    {
      // Create new CDC tracks out of the found wire hits and store them into a store array
      for (const auto& vxdTrackWithMatchedWireHits : vxdTracksWithMatchedWireHits) {
        RecoTrack* vxdRecoTrack = vxdTrackWithMatchedWireHits.getSeed();
        if (not vxdRecoTrack) {
          continue;
        }

        const auto& matchedWireHits = vxdTrackWithMatchedWireHits.getHits();
        B2ASSERT("There are no wire hit related!", not matchedWireHits.empty());

        TVector3 vxdPosition;
        TVector3 trackMomentum;
        int trackCharge;

        extractTrackState(*vxdRecoTrack, vxdPosition, trackMomentum, trackCharge);

        RecoTrack* newCDCOnlyTrack = addNewTrack(vxdPosition, trackMomentum, trackCharge, matchedWireHits,
                                                 m_cdcRecoTracks);
        if (not m_param_debuggingRelationsTo.empty()) {
          vxdRecoTrack->addRelationTo(newCDCOnlyTrack);
        }
      }
    }

  private:
    // Parameters
    /** Export the tracks or not */
    bool m_param_exportTracks = true;
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_cdcRecoTrackStoreArrayName = "CKFVXDRecoTracks";
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_debuggingRelationsTo = "VXDRecoTracks";

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;

    /// Helper function to get the seed or the measured state on plane from a track
    void extractTrackState(const RecoTrack& recoTrack,
                           TVector3& position, TVector3& momentum, int& charge) const
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

    /// Add a new track to the given store array with the SVD/PXD hits and the given track parameters
    RecoTrack* addNewTrack(const TVector3& trackPosition, const TVector3& trackMomentum,
                           int trackCharge, std::vector<const TrackFindingCDC::CDCRLWireHit*> hits,
                           StoreArray<RecoTrack>& storeArray) const
    {
      // Add cdc-only track for reference
      RecoTrack* recoTrack = storeArray.appendNew(trackPosition, trackMomentum, trackCharge);

      int sortingParameter = -1;
      for (const auto& rlWireHit : hits) {
        ++sortingParameter;

        const TrackFindingCDC::CDCWireHit& wireHit = rlWireHit->getWireHit();
        const CDCHit* cdcHit = wireHit.getHit();

        // Right left ambiguity resolution
        TrackFindingCDC::ERightLeft rlInfo = rlWireHit->getRLInfo();

        if (rlInfo == TrackFindingCDC::ERightLeft::c_Left) {
          recoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_left);
        } else if (rlInfo == TrackFindingCDC::ERightLeft::c_Right) {
          recoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_right);
        } else if (rlInfo == TrackFindingCDC::ERightLeft::c_Invalid) {
          recoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_invalidRightLeftInformation);
        } else {
          recoTrack->addCDCHit(cdcHit, sortingParameter, RecoHitInformation::RightLeftInformation::c_undefinedRightLeftInformation);
        }
      }

      return recoTrack;
    }
  };
}
