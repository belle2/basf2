/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;
}

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;
    class CDCRecoHit2D;
    class CDCRecoHit3D;
    class CDCSegment2D;
    class CDCSegment3D;
    class CDCTrack;

    /// Helper construct implementing the (re)estimation of the drift length for various hit objects
    struct DriftLengthEstimator {

      /// Add the parameters of the estimator to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

      /// Update the drift length of the reconstructed hit in place.
      double updateDriftLength(CDCRecoHit2D& recoHit2D);

      /// Update the drift length of the reconstructed hit in place.
      double updateDriftLength(CDCRecoHit3D& recoHit3D, double tanLambda);

      /**
       *  Reestimate the drift length of all three contained drift circles.
       *  Using the additional flight direction information the accuracy of the drift length
       *  can be increased alot helping the filters following this step
       */
      void updateDriftLength(CDCFacet& facet);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(CDCSegment2D& segment);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(CDCSegment3D& segment, double tanLambda);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(CDCTrack& track, double tanLambda);

      /// Parameter : Switch to serve the alpha angle to the drift length translator.
      bool m_param_useAlphaInDriftLength = true;

      /// Parameter : Mass to estimate the velocity in the flight time to the hit
      double m_param_tofMassScale = NAN;
    };
  }
}
