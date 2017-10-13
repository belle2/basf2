/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /// Generates axial tracks from hit using the special leaf processing inspired by the legendre algorithm.
    class AxialTrackCreatorHitHough : public Findlet<const CDCWireHit* const, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit* const, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the findlet before event processing
      void initialize() final;

      /// Generates the tracks from the given segments into the output argument.
      void apply(const std::vector<const CDCWireHit*>& axialWireHits, std::vector<CDCTrack>& tracks) final;

      /// Cleanup the findlet after event processing
      void terminate() final;

    public:
      /// Get a series of parameters to be set for each pass over the rough hough space
      static std::vector<float> getDefaultCurvBounds(std::array<float, 2> curvSpan, int granularityLevel);

    private:
      /// Curvature below which particles from IP do not leave the CDC
      const double m_curlCurv = 0.018;

    private:
      // Hough space
      /// Parameter: Level of divisions in the hough space.
      int m_param_granularityLevel = 12;

      /// Parameter: Number of levels to be skipped in the hough space on the first level to form sectors
      int m_param_sectorLevelSkip = 2;

      /// Parameter: hough bounds.
      std::vector<float> m_param_curvBounds{{ -0.02, 0.14}};

      /// Parameter: Width of the phi0 bins at the highest level of the hough space.
      int m_param_discretePhi0Width = 19;

      /// Parameter: Overlap of the phi0 bins at the highest level of the hough space.
      int m_param_discretePhi0Overlap = 5;

      /// Parameter: Width of the curvature bins at the highest level of the hough space.
      int m_param_discreteCurvWidth = 1;

      /// Parameter: Overlap of the curvature bins at the highest level of the hough space.
      int m_param_discreteCurvOverlap = -1;

      /// Parameter: Relaxation schedule for the leaf processor in the hough tree
      std::vector<ParameterVariantMap> m_param_relaxationSchedule;

      /// Fixed parameter: Number of divisions in the phi0 direction
      static const int c_phi0Divisions = 2;

      /// Fixed parameter: Number of divisions in the curv direction
      static const int c_curvDivisions = 2;

    private:
      /// Type of the hough space tree search
      using SimpleRLTaggedWireHitPhi0CurvHough =
        SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, c_phi0Divisions, c_curvDivisions>;

      /// The hough space tree search
      std::unique_ptr<SimpleRLTaggedWireHitPhi0CurvHough> m_houghTree;
    };
  }
}
