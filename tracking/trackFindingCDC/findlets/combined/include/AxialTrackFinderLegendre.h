/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackHitMigrator.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitLegendre.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackingUtilities {
    class CDCTrack;
    class CDCWireHit;
  }
  namespace TrackFindingCDC {
    enum class LegendreFindingPass;

    /**
     * Generates axial tracks from hit using special leaf postprocessing.
     *
     * Implements Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class AxialTrackFinderLegendre : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack>;

    public:
      /// Constructor
      AxialTrackFinderLegendre();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method to apply the track finding.
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& wireHits, std::vector<TrackingUtilities::CDCTrack>& tracks);

    private: // findlets
      /// Class of Pass keys
      using EPass = AxialTrackCreatorHitLegendre::EPass;

      /// Findlet for the non-curler legendre pass
      AxialTrackCreatorHitLegendre m_nonCurlerAxialTrackCreatorHitLegendre{EPass::NonCurlers};

      /// Findlet for the non-curler with increased threshold legendre pass
      AxialTrackCreatorHitLegendre m_nonCurlersWithIncreasingThresholdAxialTrackCreatorHitLegendre{
        EPass::NonCurlersWithIncreasingThreshold};

      /// Findlet for the full range legendre pass
      AxialTrackCreatorHitLegendre m_fullRangeAxialTrackCreatorHitLegendre{EPass::FullRange};

      /// Findlet to exchange hits between tracks based on their proximity to the respective trajectory
      AxialTrackHitMigrator m_axialTrackHitMigrator;

      /// Findlet to merge the tracks after the legendre finder.
      AxialTrackMerger m_axialTrackMerger;
    };
  }
}
