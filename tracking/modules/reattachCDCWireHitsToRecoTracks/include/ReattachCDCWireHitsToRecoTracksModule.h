/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Cyrille Praz, Tracking group                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /**
     * Module to loop over low-ADC/TOT CDCWireHits and fitted RecoTracks,
     * and reattach the hits to the tracks if they are closer
     * than a given distance. Then, the tracks are refitted.
     */
    class ReattachCDCWireHitsToRecoTracksModule : public Module {

    public:

      /// Constructor of the module. Setting up parameters and description.
      ReattachCDCWireHitsToRecoTracksModule();

      /// Declare required StoreArray
      void initialize() override;

      /// Event processing, combine store array
      void event() override;

    private:

      /// Internal structure to store the results of the reconstruction.
      struct ReconstructionResults {
        /// Arc length of the hit w.r.t. to a genfit::MeasuredStateOnPlane
        double arcLength = 0.0;
        /// Reconstructed z position of the hit
        double z = 0.0;
        /// Distance from the hit to the track
        double distanceToTrack = 0.0;
        /// Right-left information of the hit
        ERightLeft rlInfo = ERightLeft::c_Unknown;
        /// Success status of the reconstruction
        bool isValid = false;
      };

      /// Internal structure to store the information about a hit to be added.
      struct HitToAddInfo {
        /// Pointer the hit to be added
        CDCWireHit* hit = nullptr;
        /// Right-left information of the hit
        ERightLeft rlInfo = ERightLeft::c_Unknown;
      };

      /// Find the hits that can be added to the RecoTracks.
      void findHits();

      /// Add the selected CDC hits to the RecoTracks.
      void addHits();

      /// Compute distance from a CDCWireHit to a RecoTrack using the mSoP found with a RecoHitInformation.
      ReconstructionResults reconstruct(const CDCWireHit& wireHit,
                                        const RecoTrack& recoTrack,
                                        const RecoHitInformation* recoHitInformation) const;

      /// Translate a TrackFindingCDC::ERightLeft into a RecoHitInformation::RightLeftInformation.
      RecoHitInformation::RightLeftInformation rightLeftInformationTranslator(ERightLeft rlInfo) const;

    private:

      /// Name of the input CDCWireHit StoreWrappedObjPtr
      std::string m_CDCWireHitsStoreArrayName = "CDCWireHitVector";
      /// Name of the input RecoTrack StoreArray
      std::string m_inputRecoTracksStoreArrayName = "CDCRecoTracks";
      /// Name of the output RecoTrack StoreArray
      std::string m_outputRecoTracksStoreArrayName = "ExtendedCDCRecoTracks";
      /// Distance (cm) below which (exclusive) a CDC hit can be reattached to a track
      double m_maximumDistance = 0.1;
      /// ADC above which (inclusive) a CDC hit can be reattached to a track
      int m_minimumADC = 1;
      /// TOT above which (inclusive) a CDC hit can be reattached to a track
      int m_minimumTOT = 1;
      /// Only tracks with an absolute value of d0 below (exclusive) this parameter (cm) are considered
      double m_maximumAbsD0 = 13.5;
      /// Only tracks with an absolute value of z0 below (exclusive) this parameter (cm) are considered
      double m_maximumAbsZ0 = 35.0;

      /// Input CDCWireHits
      StoreWrappedObjPtr<std::vector<CDCWireHit> > m_CDCWireHits;
      /// Input tracks
      StoreArray<RecoTrack> m_inputRecoTracks;
      /// Output tracks
      StoreArray<RecoTrack> m_outputRecoTracks;

      /// Map from a RecoTrack ptr to the vector of the hits that belong to this track.
      std::unordered_map<RecoTrack*, std::vector<CDCHit*> > m_mapToHitsOnTrack;
      /// Map from a RecoTrack ptr to the vector of the hits that need to be added to this track.
      std::unordered_map<RecoTrack*, std::vector<HitToAddInfo> > m_mapToHitsToAdd;
    };
  }
}
