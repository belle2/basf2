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

#include <tracking/ckf/states/CKFResultObject.h>
#include <tracking/ckf/utilities/ResultAlgorithms.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /**
   * Store array handler for CKF results
   *
   * This findlet is responsible for the interface between the DataStore and the CKF modules:
   * to write back the found tracks only and the merged tracks as relations
   */
  template<class ASeedObject, class AHitObject>
  class ResultWriter : public TrackFindingCDC::Findlet<const CKFResultObject<ASeedObject, AHitObject>> {
    using Super = TrackFindingCDC::Findlet<const CKFResultObject<ASeedObject, AHitObject>>;

  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                    "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRecoTrackStoreArrayName"),
                                    m_param_vxdRecoTrackStoreArrayName,
                                    "StoreArray name of the output Track Store Array", m_param_vxdRecoTrackStoreArrayName);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "debuggingRelationsTo"), m_param_debuggingRelationsTo,
                                    "Add a relation to the tracks given in this store array name (or none to not add relations).",
                                    m_param_debuggingRelationsTo);
    }

    /// Require/register the store arrays
    void initialize() override
    {
      Super::initialize();

      if (not m_param_exportTracks) {
        return;
      }

      m_recoTracks.registerInDataStore(m_param_vxdRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_recoTracks);

      if (not m_param_debuggingRelationsTo.empty()) {
        StoreArray<RecoTrack> relationRecoTracks(m_param_debuggingRelationsTo);
        relationRecoTracks.registerRelationTo(m_recoTracks);
      }
    }


    /**
     * Write back the found tracks and add relations.
     */
    void apply(const std::vector<CKFResultObject<ASeedObject, AHitObject>>& results) override
    {
      if (not m_param_exportTracks) {
        return;
      }

      for (const auto& result : results) {
        RecoTrack* seed = result.getSeed();
        if (not seed) {
          continue;
        }

        const auto& matchedHits = result.getHits();
        B2ASSERT("There are no hits related!", not matchedHits.empty());

        const TVector3& trackPosition = result.getPosition();
        const TVector3& trackMomentum = result.getMomentum();
        const short& trackCharge = result.getCharge();

        RecoTrack* newRecoTrack = m_recoTracks.appendNew(trackPosition, trackMomentum, trackCharge);
        RecoTrackHitsAdder::addHitsToRecoTrack(matchedHits, *newRecoTrack);

        if (not m_param_debuggingRelationsTo.empty()) {
          seed->addRelationTo(newRecoTrack);
        }
      }
    }

  private:
    // Parameters
    /** Export the tracks or not */
    bool m_param_exportTracks = true;
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName = "CKFRecoTracks";
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_debuggingRelationsTo = "RecoTracks";

    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_recoTracks;
  };
}
