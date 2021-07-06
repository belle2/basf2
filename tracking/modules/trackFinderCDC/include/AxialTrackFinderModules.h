/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>
#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderHough.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorSegmentHough.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorMCTruth.h>

#include <tracking/trackFindingCDC/findlets/combined/MonopoleAxialTrackFinderLegendre.h>
#include <tracking/trackFindingCDC/findlets/combined/AxialStraightTrackFinder.h>

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

    /**
     * Module implementation using the MonopoleAxialTrackFinderLegendre
     * Modification of conventional AxialTrackFinderLegendre
     */
    class TFCDC_MonopoleAxialTrackFinderLegendreModule : public FindletModule<MonopoleAxialTrackFinderLegendre> {

      /// Type of the base class
      using Super = FindletModule<MonopoleAxialTrackFinderLegendre>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_MonopoleAxialTrackFinderLegendreModule();
    };

    /**
     * Module implementation using the AxialStraightTrackFinder
     */
    class TFCDC_AxialStraightTrackFinderModule : public FindletModule<AxialStraightTrackFinder> {

      /// Type of the base class
      using Super = FindletModule<AxialStraightTrackFinder>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialStraightTrackFinderModule();
    };
  }
}
