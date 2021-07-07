/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackMerger.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitHough.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackHitMigrator.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /// Generates axial tracks from hit using the special leaf processing inspired by the hough algorithm.
    class AxialTrackFinderHough : public Findlet<const CDCWireHit, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCTrack>;

    public:
      /// Constructor
      AxialTrackFinderHough();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Generates the tracks from the given segments into the output argument.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks) final;

    public:
      /// Get a series of parameters to be set for each pass over the fine hough space
      std::vector<ParameterVariantMap> getDefaultFineRelaxationSchedule() const;

      /// Get a series of parameters to be set for each pass over the rough hough space
      std::vector<ParameterVariantMap> getDefaultRoughRelaxationSchedule() const;

    private:
      /// Maximal curvature acceptance of the CDC
      const double m_maxCurvAcceptance = 0.13;

    private: // findlets
      /// First hough search over a fine hough grid
      AxialTrackCreatorHitHough m_fineHoughSearch;

      /// Second hough search over a fine hough grid
      AxialTrackCreatorHitHough m_roughHoughSearch;

      /// Findlet to exchange hits between tracks based on their proximity to the respective trajectory
      AxialTrackHitMigrator m_axialTrackHitMigrator;

      /// Findlet to merge the tracks after the hough finder.
      AxialTrackMerger m_axialTrackMerger;
    };
  }
}
