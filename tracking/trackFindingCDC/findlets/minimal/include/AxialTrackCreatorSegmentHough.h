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

#include <tracking/trackFindingCDC/hough/perigee/SimpleSegmentHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0ImpactCurvBox.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generates axial tracks from segments using the hough algorithm
    class AxialTrackCreatorSegmentHough:
      public Findlet<const CDCRecoSegment2D, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCTrack>;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final;

      /// Initialize the findlet before event processing
      virtual void initialize() override;

      /// Generates the tracks from the given segments into the output argument.
      virtual void apply(const std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks) override final;

      /// Cleanup the findlet after event processing
      virtual void terminate() override;

    private:
      /// Parameter: Absolute minimal number of hits to make an axial track.
      double m_param_minNHits = 40;

      /// Parameter: Minimal number of hits as a fraction of the total hits in the event.
      double m_param_minFractionNHits = 0.5;

      /// Parameter: Level of divisions in the hough space.
      size_t m_param_maxLevel = 12;

      /// Parameter: Curvature bounds of the hough space.
      std::vector<float> m_param_curvBounds{{ -0.13, 0.13}};

      /// Parameter: Impact parameter bounds of the hough space.
      std::vector<float> m_param_impactBounds{{ -100, 100}};

      /// Parameter: Width of the phi0 bins at the lowest level of the hough space.
      size_t m_param_discretePhi0Width = 2;

      /// Parameter: Overlap of the phi0 bins at the lowest level of the hough space.
      size_t m_param_discretePhi0Overlap = 1;

      /// Parameter: Width of the impact bins at the lowest level of the hough space.
      size_t m_param_discreteImpactWidth = 2;

      /// Parameter: Overlap of the impact bins at the lowest level of the hough space.
      size_t m_param_discreteImpactOverlap = 1;

      /// Parameter: Width of the curvature bins at the lowest level of the hough space.
      size_t m_param_discreteCurvWidth = 2;

      /// Parameter: Overlap of the curvature bins at the lowest level of the hough space.
      size_t m_param_discreteCurvOverlap = 1;

      /// Fixed parameter: Number of divisions in the phi0 direction
      static const size_t c_phi0Divisions = 2;

      /// Fixed parameter: Number of divisions in the impact direction
      static const size_t c_impactDivisions = 2;

      /// Fixed parameter: Number of divisions in the curv direction
      static const size_t c_curvDivisions = 2;

    private:
      /// Type of the hough space tree search
      using SimpleSegmentPhi0ImpactCurvHoughTree =
        SimpleSegmentHoughTree<InPhi0ImpactCurvBox, c_phi0Divisions, c_impactDivisions, c_curvDivisions>;

      /// The hough space tree search
      SimpleSegmentPhi0ImpactCurvHoughTree m_houghTree{m_param_maxLevel};
    };

  }

}
