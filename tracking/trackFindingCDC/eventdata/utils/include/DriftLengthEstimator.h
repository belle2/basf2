/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;
}

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCRecoHit3D;
    class CDCRecoHit2D;
    class CDCFacet;
    class CDCSegment2D;
    class CDCSegment3D;
  }
  namespace TrackFindingCDC {

    /// Helper construct implementing the (re)estimation of the drift length for various hit objects
    struct DriftLengthEstimator {

      /// Add the parameters of the estimator to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

      /// Update the drift length of the reconstructed hit in place.
      double updateDriftLength(TrackingUtilities::CDCRecoHit2D& recoHit2D);

      /// Update the drift length of the reconstructed hit in place.
      double updateDriftLength(TrackingUtilities::CDCRecoHit3D& recoHit3D, double tanLambda);

      /**
       *  Re-estimate the drift length of all three contained drift circles.
       *  Using the additional flight direction information the accuracy of the drift length
       *  can be increased a lot helping the filters following this step
       */
      void updateDriftLength(TrackingUtilities::CDCFacet& facet);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(TrackingUtilities::CDCSegment2D& segment);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(TrackingUtilities::CDCSegment3D& segment, double tanLambda);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(TrackingUtilities::CDCTrack& track, double tanLambda);

      /// Parameter : Switch to serve the alpha angle to the drift length translator.
      bool m_param_useAlphaInDriftLength = true;

      /// Parameter : Mass to estimate the velocity in the flight time to the hit
      double m_param_tofMassScale = NAN;
    };
  }
}
