/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <tracking/trackingUtilities/geometry/UncertainParameterLine2D.h>

#include <memory>
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

      /**
       *  Memory for the results of a facet drift length update that do not depend on the
       *  right left passage hypothesis of the middle hit.
       *
       *  Facets that differ only in the middle right left passage hypothesis have the same
       *  fit line by construction, such that most of the update can be shared between them.
       */
      struct FacetDriftLengthCache {
        /// Indicator that the cache holds valid values
        bool valid = false;

        /// The fit line of the facet at the time of the update
        TrackingUtilities::UncertainParameterLine2D fitLine;

        /// The angle between the position and the flight direction used in the update
        double alpha = 0;

        /// The flight time estimate of the middle hit
        double middleFlightTime = 0;

        /// The updated drift length of the start hit
        double startDriftLength = 0;

        /// The updated drift length of the end hit
        double endDriftLength = 0;
      };

      /**
       *  Re-estimate the drift lengths like updateDriftLength(facet), sharing the results
       *  that do not depend on the middle right left passage hypothesis via the given cache.
       *
       *  On the first call the cache is filled from a full update. Subsequent calls must be
       *  made with facets that differ from the first one only in the right left passage
       *  hypothesis of the middle hit and with unchanged base drift lengths, and only
       *  recompute the drift length of the middle hit.
       */
      void updateDriftLength(TrackingUtilities::CDCFacet& facet, FacetDriftLengthCache& cache);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(TrackingUtilities::CDCSegment2D& segment);

      /// Update the drift length of the contained reconstructed hit in place.
      static void updateDriftLength(TrackingUtilities::CDCSegment3D& segment, double tanLambda);

      /// Update the drift length of the contained reconstructed hit in place.
      static void updateDriftLength(TrackingUtilities::CDCTrack& track, double tanLambda);

      /// Parameter : Switch to serve the alpha angle to the drift length translator.
      bool m_param_useAlphaInDriftLength = true;

      /// Parameter : Mass to estimate the velocity in the flight time to the hit
      double m_param_tofMassScale = NAN;

    private:
      /**
       *  Getter for the drift time translator constructing it on first use.
       *
       *  The translator cannot be constructed together with the estimator, which usually
       *  happens during module construction, since it requires the geometry to be present.
       */
      CDC::RealisticTDCCountTranslator& getTDCCountTranslator()
      {
        if (not m_tdcCountTranslator) {
          m_tdcCountTranslator = std::make_shared<CDC::RealisticTDCCountTranslator>();
        }
        return *m_tdcCountTranslator;
      }

      /// Drift time translator constructed lazily on first use
      std::shared_ptr<CDC::RealisticTDCCountTranslator> m_tdcCountTranslator;
    };
  }
}
