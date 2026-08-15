/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>

#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Module implementation using the TrackFinderSegmentPairAutomaton
     */
    class TFCDC_TrackFinderSegmentPairAutomatonModule : public TrackingUtilities::FindletModule<TrackFinderSegmentPairAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackFinderSegmentPairAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackFinderSegmentPairAutomatonModule();
    };

    /**
     * Module implementation using the TrackFinderSegmentTripleAutomaton
     */
    class TFCDC_TrackFinderSegmentTripleAutomatonModule : public TrackingUtilities::FindletModule<TrackFinderSegmentTripleAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackFinderSegmentTripleAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackFinderSegmentTripleAutomatonModule();
    };

    /**
     * Module implementation using the TrackCreatorSingleSegments
     */
    class TFCDC_TrackCreatorSingleSegmentsModule : public TrackingUtilities::FindletModule<TrackCreatorSingleSegments> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackCreatorSingleSegments>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackCreatorSingleSegmentsModule();
    };
  }
}

// ****** Minimal Findlets ********** //
namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the TrackCreatorSegmentPairAutomaton
     */
    class TFCDC_TrackCreatorSegmentPairAutomatonModule : public TrackingUtilities::FindletModule<TrackCreatorSegmentPairAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackCreatorSegmentPairAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackCreatorSegmentPairAutomatonModule();
    };

    /**
     * Module implementation using the TrackCreatorSegmentTripleAutomaton
     */
    class TFCDC_TrackCreatorSegmentTripleAutomatonModule : public TrackingUtilities::FindletModule<TrackCreatorSegmentTripleAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<TrackCreatorSegmentTripleAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_TrackCreatorSegmentTripleAutomatonModule();
    };

    /**
     * Module implementation using the SegmentPairCreator
     */
    class TFCDC_SegmentPairCreatorModule : public TrackingUtilities::FindletModule<SegmentPairCreator> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentPairCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentPairCreatorModule();
    };

    /**
     * Module implementation using the AxialSegmentPairCreator
     */
    class TFCDC_AxialSegmentPairCreatorModule : public TrackingUtilities::FindletModule<AxialSegmentPairCreator> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<AxialSegmentPairCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_AxialSegmentPairCreatorModule();
    };

    /**
     * Module implementation using the SegmentTripleCreator
     */
    class TFCDC_SegmentTripleCreatorModule : public TrackingUtilities::FindletModule<SegmentTripleCreator> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentTripleCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentTripleCreatorModule();
    };
  }
}
