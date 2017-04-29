/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>
#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderHough.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorSegmentHough.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorMCTruth.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * CDC tracking module, using Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class TFCDC_AxialTrackFinderLegendreModule : public FindletModule<AxialTrackFinderLegendre> {

      /// Type of the base class
      using Super = FindletModule<AxialTrackFinderLegendre>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialTrackFinderLegendreModule();
    };

    /**
     * Module implementation using the AxialTrackFinderHough
     */
    class TFCDC_AxialTrackFinderHoughModule : public FindletModule<AxialTrackFinderHough> {

      /// Type of the base class
      using Super = FindletModule<AxialTrackFinderHough>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialTrackFinderHoughModule();
    };

    /**
     * Module implementation using the AxialTrackCreatorSegmentHough
     */
    class TFCDC_AxialTrackCreatorSegmentHoughModule : public FindletModule<AxialTrackCreatorSegmentHough> {

      /// Type of the base class
      using Super = FindletModule<AxialTrackCreatorSegmentHough>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialTrackCreatorSegmentHoughModule();
    };

    /**
     * Module implementation using the AxialTrackCreatorMCTruth
     */
    class TFCDC_AxialTrackCreatorMCTruthModule : public FindletModule<AxialTrackCreatorMCTruth> {

      /// Type of the base class
      using Super = FindletModule<AxialTrackCreatorMCTruth>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialTrackCreatorMCTruthModule();
    };
  }
}
