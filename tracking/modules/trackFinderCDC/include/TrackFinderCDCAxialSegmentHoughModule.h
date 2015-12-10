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

#include <tracking/trackFindingCDC/hough/perigee/InPhi0ImpactCurvBox.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Axial track finder from segments for mainly for cosmics.
    class TrackFinderCDCAxialSegmentHoughModule: public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCAxialSegmentHoughModule()
      {
        setDescription("Generates axial tracks from segments using a hough space over phi0 impact and curvature for the spares case.");
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        B2ASSERT("Need exactly two curv bound", m_param_curvBounds.size() == 2);
        B2ASSERT("Need exactly two impact bound", m_param_impactBounds.size() == 2);

        const size_t nPhi0Bins = std::pow(c_phi0Divisions, m_param_maxLevel);
        const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                        m_param_discretePhi0Overlap,
                                        m_param_discretePhi0Width);

        std::array<double, 2> impactBounds{{m_param_impactBounds.front(), m_param_impactBounds.back()}};
        const size_t nImpactBins = std::pow(c_impactDivisions, m_param_maxLevel);
        const ImpactBinsSpec impactBinsSpec(impactBounds[0],
                                            impactBounds[1],
                                            nImpactBins,
                                            m_param_discreteImpactOverlap,
                                            m_param_discreteImpactWidth);

        std::array<double, 2> curvBounds{{m_param_curvBounds.front(), m_param_curvBounds.back()}};
        const size_t nCurvBins = std::pow(c_curvDivisions, m_param_maxLevel);
        const CurvBinsSpec curvBinsSpec(curvBounds[0],
                                        curvBounds[1],
                                        nCurvBins,
                                        m_param_discreteCurvOverlap,
                                        m_param_discreteCurvWidth);

        m_houghTree.setMaxLevel(m_param_maxLevel);
        m_houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
        m_houghTree.assignArray<ContinuousImpact>(impactBounds, impactBinsSpec.getOverlap()); // Continuous
        m_houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(), curvBinsSpec.getNOverlap());
        m_houghTree.initialize();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

      virtual void terminate() override
      {
        m_houghTree.raze();
      }

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
    }; // end class

    void TrackFinderCDCAxialSegmentHoughModule::
    generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
             std::vector<TrackFindingCDC::CDCTrack>& tracks)
    {
      m_houghTree.fell();

      size_t nAxialHits = 0;
      std::vector<const Belle2::TrackFindingCDC::CDCRecoSegment2D*> ptrAxialSegments;
      ptrAxialSegments.reserve(segments.size());

      for (const CDCRecoSegment2D& segment : segments) {
        if (segment.getStereoType() == StereoType::c_Axial) {
          ptrAxialSegments.push_back(&segment);
          nAxialHits += segment.size();
        }
      }

      m_houghTree.seed(ptrAxialSegments);
      // B2INFO("#axial segments: " << ptrAxialSegments.size());
      // B2INFO("#axial hits: " << nAxialHits);
      // B2INFO("#Nodes: " << m_houghTree.getTree()->getNNodes());
      using HoughBox = SimpleSegmentPhi0ImpactCurvHoughTree::HoughBox;

      Weight minWeight = std::min(m_param_minNHits, nAxialHits * m_param_minFractionNHits);
      // B2INFO("#min weight: " << minWeight);

      using Candidate = std::pair<HoughBox, std::vector<const CDCRecoSegment2D*> >;
      std::vector<Candidate> candidates = m_houghTree.findBest(minWeight);

      for (const Candidate& candidate : candidates) {
        // B2INFO("Found candidate");
        const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
        CDCObservations2D observations;

        const HoughBox& foundHoughBox = candidate.first;
        const std::vector<const CDCRecoSegment2D*>& foundSegments = candidate.second;
        // B2INFO("#Segments" << foundSegments.size());
        for (const CDCRecoSegment2D* segment : foundSegments) {
          observations.appendRange(*segment);
        }
        CDCTrajectory2D trajectory2D = fitter.fit(observations);

        // Check if the circle has been fitted reverse to the hough box by accident
        {
          GeneralizedCircle circle = trajectory2D.getGlobalCircle();
          const double& curv = circle.curvature();
          const std::array<DiscreteCurv, 2>& curvs = foundHoughBox.getBounds<DiscreteCurv>();
          const float& lowerCurv = *(curvs[0]);
          const float& upperCurv = *(curvs[1]);
          if (SameSignChecker::commonSign(lowerCurv, upperCurv) * curv < 0) {
            trajectory2D.reverse();
          }
        }

        CDCTrack track;
        for (const CDCRecoSegment2D* segment : foundSegments) {
          for (const CDCRecoHit2D& recoHit2D : *segment) {
            track.push_back(CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D));
          }
        }
        track.sortByArcLength2D();
        // B2INFO("#Hits" << track.size());

        /// Setting trajectories
        if (track.empty()) continue;
        const CDCRecoHit3D& startRecoHit3D = track.front();
        CDCTrajectory3D startTrajectory3D(trajectory2D);
        startTrajectory3D.setLocalOrigin(startRecoHit3D.getRecoPos3D());
        track.setStartTrajectory3D(startTrajectory3D);

        const CDCRecoHit3D& endRecoHit3D = track.back();
        CDCTrajectory3D endTrajectory3D(trajectory2D);
        endTrajectory3D.setLocalOrigin(endRecoHit3D.getRecoPos3D());
        track.setEndTrajectory3D(endTrajectory3D);

        tracks.push_back(std::move(track));
      } // for candidates
    }
  } //end namespace TrackFinderCDC

  using TrackFinderCDCAxialSegmentHoughModule = TrackFindingCDC::TrackFinderCDCAxialSegmentHoughModule;

} //end namespace Belle2
