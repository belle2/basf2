/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Dmitrii Neverov               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/BaseCandidateReceiver.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;
    class Vector2D;

    /**
     *  Class performs extension (adding new hits) of given candidate using conformal transformation w.r.t point on the trajectory
     */
    class OffOriginExtension : public BaseCandidateReceiver {

    public:
      /// Constructor
      OffOriginExtension(std::vector<const CDCWireHit*> allAxialWireHits, double levelPrecision = 9);

      /// Main entry point for the post processing call from the QuadTreeProcessor
      void operator()(const std::vector<const CDCWireHit*>& inputWireHits, void* qt) final;

      /// Perform transformation for set of given hits; reference position taken as POCA of the fitted trajectory
      std::vector<const CDCWireHit*> roadSearch(const std::vector<const CDCWireHit*>& wireHits);
      /**
       * @brief Get hits which are compatible with given trajectory
       *
       * @param refPos defines 2D reference position with respect to which transformation will be performed
       * @param curv curvarute of the track trajectory
       * @param theta angle between x-axis and vector to the center of the circle which represents trajectory
       * @return vector of CDCWireHit objects which satisfy legendre transformation with respect to the given parameters
       */
      std::vector<const CDCWireHit*> getHitsWRTtoRefPos(const Vector2D& refPos, float curv, float theta);

    private:

      /// Precision level for the width of the off origin hough search
      double m_levelPrecision;
      //.5 - 0.24 * exp(-4.13118 * TrackCandidate::convertRhoToPt(fabs(track_par.second)) + 2.74);
    };
  }
}
