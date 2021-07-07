/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Findlet implementing the merging of axial tracks found in the legendre tree search
    class AxialTrackMerger : public Findlet<CDCTrack&, const CDCWireHit* const> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&, const CDCWireHit* const>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Merge tracks together. Allows for axial hits to be added as it may see fit.
      void apply(std::vector<CDCTrack>& axialTracks, const std::vector<const CDCWireHit*>& axialWireHits) final;

    private:
      /**
       *  The track finding often finds two curling tracks, originating from the same particle.
       *  This function merges them.
       */
      void doTracksMerging(std::vector<CDCTrack>& axialTracks,
                           const std::vector<const CDCWireHit*>& allAxialWireHits);

    private:
      /**
       *  Searches for the best candidate to merge this track to.
       *  @param track   track for which we try to find merging partner
       *  @param tracks  search range of tracks
       *  @return        a pointer to the best fit candidate including a fit probability
       *  @retval        <tt>{nullptr, 0}<\tt> in case no match was found
       */
      template <class ACDCTracks>
      static WithWeight<MayBePtr<CDCTrack> > calculateBestTrackToMerge(CDCTrack& track, ACDCTracks& tracks);

      /**
       *  Fits the hit content of both tracks in a common fit repeated with an annealing schedule removing far away hits
       *
       *  @return Some measure of fit probability
       */
      static double doTracksFitTogether(CDCTrack& track1, CDCTrack& track2);

      /**
       *  Remove all hits that are further than factor * driftlength away from the trajectory
       *
       *  @param factor gives a number how far the hit is allowed to be.
       *  @param wireHits the wirehits that should be considered
       *  @param trajectory the reference trajectory
       */
      static void removeStrangeHits(double factor,
                                    std::vector<const CDCWireHit*>& wireHits,
                                    CDCTrajectory2D& trajectory);

      /**
       *  Function to merge two track candidates.
       *  The hits of track2 are deleted and transfered to track1 and the track1 is resorted.
       *  The method also applys some postprocessing and splits the track1 in case it appears
       *  to contain two back-to-back arms,
       */
      static void mergeTracks(CDCTrack& track1,
                              CDCTrack& track2,
                              const std::vector<const CDCWireHit*>& allAxialWireHits);

    private:
      /// Parameter : Minimal fit probability of the common fit of two tracks to be eligible for merging
      double m_param_minFitProb = 0.85;
    };
  }
}
