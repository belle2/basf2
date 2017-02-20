/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>

#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class performs extension (adding new hits) of given candidate using conformal transformation w.r.t point on the trajectory
     */
    class OffOriginExtension {

    public:
      /// Constructor
      OffOriginExtension(std::vector<const CDCWireHit*> allAxialWireHits, double levelPrecision = 9)
        : m_allAxialWireHits(std::move(allAxialWireHits))
        , m_levelPrecision(levelPrecision)
      {
      }

      /// Main entry point for the post processing call from the QuadTreeProcessor
      void operator()(const std::vector<const CDCWireHit*>& inputWireHits,
                      void* qt  __attribute__((unused)))
      {
        // Unset the taken flag and let the postprocessing decide
        for (const CDCWireHit* wireHit : inputWireHits) {
          (*wireHit)->setTakenFlag(false);
        }

        std::vector<const CDCWireHit*> candidateHits = roadSearch(inputWireHits);
        AxialTrackUtil::addCandidateFromHitsWithPostprocessing(candidateHits, m_allAxialWireHits, m_tracks);
      }

      /// Perform transformation for set of given hits; reference position taken as POCA of the fitted trajectory
      std::vector<const CDCWireHit*> roadSearch(const std::vector<const CDCWireHit*>& wireHits)
      {
        const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getNoDriftVarianceFitter();
        CDCTrajectory2D trackTrajectory2D = fitter.fit(wireHits);

        double chi2 = trackTrajectory2D.getChi2();
        Vector2D refPos = trackTrajectory2D.getGlobalPerigee();

        // change sign of the curvature; should be the same as the charge of the candidate
        double curv = trackTrajectory2D.getCurvature();

        // theta is the vector normal to the trajectory at the perigee
        double theta = trackTrajectory2D.getGlobalCircle().phi0() + M_PI_2;

        // Hide the current hits from the road search
        for (const CDCWireHit* hit : wireHits) {
          hit->getAutomatonCell().setTakenFlag(true);
        }

        std::vector<const CDCWireHit*> newWireHits = getHitsWRTtoRefPos(refPos, curv, theta);

        // Unhide the current hits from the road search
        for (const CDCWireHit* hit : wireHits) {
          hit->getAutomatonCell().setTakenFlag(false);
        }

        if (newWireHits.size() == 0) return wireHits;

        std::vector<const CDCWireHit*> combinedWireHits;

        for (const CDCWireHit* hit : wireHits) {
          combinedWireHits.push_back(hit);
        }

        for (const CDCWireHit* hit : newWireHits) {
          combinedWireHits.push_back(hit);
        }

        CDCTrajectory2D combinedTrajectory2D = fitter.fit(wireHits);
        double combinedChi2 = combinedTrajectory2D.getChi2();

        if (combinedChi2 < chi2 * 2.) {
          return combinedWireHits;
        }

        return wireHits;
      }

      /**
       * @brief Get hits which are compatible with given trajectory
       *
       * @param refPos defines 2D reference position with respect to which transformation will be performed
       * @param curv curvarute of the track trajectory
       * @param theta angle between x-axis and vector to the center of the circle which represents trajectory
       * @return vector of CDCWireHit objects which satisfy legendre transformation with respect to the given parameters
       */
      std::vector<const CDCWireHit*> getHitsWRTtoRefPos(Vector2D refPos, float curv, float theta)
      {
        float thetaPrecision = 3.1415 / (pow(2., m_levelPrecision + 1));
        float curvPrecision = 0.15 / (pow(2., m_levelPrecision));

        using XYSpans = AxialHitQuadTreeProcessorWithNewReferencePoint::XYSpans;
        XYSpans ranges({theta - thetaPrecision, theta + thetaPrecision},
        {curv - curvPrecision, curv + curvPrecision});

        AxialHitQuadTreeProcessorWithNewReferencePoint qtProcessor(ranges, std::make_pair(refPos.x(), refPos.y()));
        qtProcessor.seed(m_allAxialWireHits);

        std::vector<const CDCWireHit*> newWireHits = qtProcessor.getAssignedHits();
        return newWireHits;
      }

      /// Get the collected tracks
      const std::vector<CDCTrack>& getTracks() const
      {
        return m_tracks;
      }

    private:
      /// Pool of all axial hits from which the road search may select additional hits
      std::vector<const CDCWireHit*> m_allAxialWireHits;

      /// Collected tracks
      std::vector<CDCTrack> m_tracks;

      /// Precision level for the width of the off origin hough search
      double m_levelPrecision;
      //.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);
    };
  }
}
