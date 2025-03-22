/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>

#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>

/******* Combined Findlets **********/
namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the SegmentFinderFacetAutomaton
     */
    class TFCDC_SegmentFinderFacetAutomatonModule : public TrackingUtilities::FindletModule<SegmentFinderFacetAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentFinderFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentFinderFacetAutomatonModule();
    };
  }
}

/******* Minimal Findlets **********/
namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the FacetCreator
     */
    class TFCDC_FacetCreatorModule : public TrackingUtilities::FindletModule<FacetCreator> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<FacetCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_FacetCreatorModule();
    };

    /**
     * Module implementation using the SegmentCreatorFacetAutomaton
     */
    class TFCDC_SegmentCreatorFacetAutomatonModule : public TrackingUtilities::FindletModule<SegmentCreatorFacetAutomaton> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentCreatorFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentCreatorFacetAutomatonModule();
    };

    /**
     * Module implementation using the SegmentLinker
     */
    class TFCDC_SegmentLinkerModule : public TrackingUtilities::FindletModule<SegmentLinker> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentLinker>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentLinkerModule();
    };

    /**
     * Module implementation using the SegmentOrienter
     */
    class TFCDC_SegmentOrienterModule : public TrackingUtilities::FindletModule<SegmentOrienter> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentOrienter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentOrienterModule();
    };

    /**
     * Module implementation using the SegmentFitter
     */
    class TFCDC_SegmentFitterModule : public TrackingUtilities::FindletModule<SegmentFitter> {
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentFitter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentFitterModule();
    };

    /**
     * Module implementation using the SegmentRejecter
     */
    class TFCDC_SegmentRejecterModule : public TrackingUtilities::FindletModule<SegmentRejecter> {
      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentRejecterModule();
    };

    /**
     * Module implementation using the SegmentCreatorMCTruth
     */
    class TFCDC_SegmentCreatorMCTruthModule : public TrackingUtilities::FindletModule<SegmentCreatorMCTruth> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SegmentCreatorMCTruth>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentCreatorMCTruthModule();
    };
  }
}
