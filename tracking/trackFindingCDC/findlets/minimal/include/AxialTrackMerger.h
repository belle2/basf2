/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/numerics/WithWeight.h>
#include <tracking/trackingUtilities/utilities/MayBePtr.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;
  }

  namespace TrackFindingCDC {

    /// Findlet implementing the merging of axial tracks found in the legendre tree search
    class AxialTrackMerger : public
      TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&, const TrackingUtilities::CDCWireHit* const> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<TrackingUtilities::CDCTrack&, const TrackingUtilities::CDCWireHit* const>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Merge tracks together. Allows for axial hits to be added as it may see fit.
      void apply(std::vector<TrackingUtilities::CDCTrack>& axialTracks,
                 const std::vector<const TrackingUtilities::CDCWireHit*>& axialWireHits) final;

    private:
      /**
       *  The track finding often finds two curling tracks, originating from the same particle.
       *  This function merges them.
       */
      void doTracksMerging(std::vector<TrackingUtilities::CDCTrack>& axialTracks,
                           const std::vector<const TrackingUtilities::CDCWireHit*>& allAxialWireHits);

    private:
      /**
       *  Searches for the best candidate to merge this track to.
       *  @param track   track for which we try to find merging partner
       *  @param tracks  search range of tracks
       *  @return        a pointer to the best fit candidate including a fit probability
       *  @retval        {nullptr, 0} in case no match was found
       */
      template <class ACDCTracks>
      static TrackingUtilities::WithWeight<TrackingUtilities::MayBePtr<TrackingUtilities::CDCTrack> > calculateBestTrackToMerge(
        TrackingUtilities::CDCTrack& track, ACDCTracks& tracks);

      /**
       *  Fits the hit content of both tracks in a common fit repeated with an annealing schedule removing far away hits
       *
       *  @return Some measure of fit probability
       */
      static double doTracksFitTogether(TrackingUtilities::CDCTrack& track1, TrackingUtilities::CDCTrack& track2);

      /**
       *  Remove all hits that are further than factor * driftlength away from the trajectory
       *
       *  @param factor gives a number how far the hit is allowed to be.
       *  @param wireHits the wirehits that should be considered
       *  @param trajectory the reference trajectory
       */
      static void removeStrangeHits(double factor,
                                    std::vector<const TrackingUtilities::CDCWireHit*>& wireHits,
                                    TrackingUtilities::CDCTrajectory2D& trajectory);

      /**
       *  Function to merge two track candidates.
       *  The hits of track2 are deleted and transferred to track1 and the track1 is resorted.
       *  The method also applies some post processing and splits the track1 in case it appears
       *  to contain two back-to-back arms,
       */
      static void mergeTracks(TrackingUtilities::CDCTrack& track1,
                              TrackingUtilities::CDCTrack& track2,
                              const std::vector<const TrackingUtilities::CDCWireHit*>& allAxialWireHits);

    private:
      /// Parameter : Minimal fit probability of the common fit of two tracks to be eligible for merging
      double m_param_minFitProb = 0.85;
    };
  }
}
