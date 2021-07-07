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

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

/******* Combined Findlets **********/
namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the SegmentFinderFacetAutomaton
     */
    class TFCDC_SegmentFinderFacetAutomatonModule : public FindletModule<SegmentFinderFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentFinderFacetAutomaton>;

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
    class TFCDC_FacetCreatorModule : public FindletModule<FacetCreator> {

      /// Type of the base class
      using Super = FindletModule<FacetCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_FacetCreatorModule();
    };

    /**
     * Module implementation using the SegmentCreatorFacetAutomaton
     */
    class TFCDC_SegmentCreatorFacetAutomatonModule : public FindletModule<SegmentCreatorFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentCreatorFacetAutomatonModule();
    };

    /**
     * Module implementation using the SegmentLinker
     */
    class TFCDC_SegmentLinkerModule : public FindletModule<SegmentLinker> {

      /// Type of the base class
      using Super = FindletModule<SegmentLinker>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentLinkerModule();
    };

    /**
     * Module implementation using the SegmentOrienter
     */
    class TFCDC_SegmentOrienterModule : public FindletModule<SegmentOrienter> {

      /// Type of the base class
      using Super = FindletModule<SegmentOrienter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentOrienterModule();
    };

    /**
     * Module implementation using the SegmentFitter
     */
    class TFCDC_SegmentFitterModule : public FindletModule<SegmentFitter> {
      /// Type of the base class
      using Super = FindletModule<SegmentFitter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentFitterModule();
    };

    /**
     * Module implementation using the SegmentRejecter
     */
    class TFCDC_SegmentRejecterModule : public FindletModule<SegmentRejecter> {
      /// Type of the base class
      using Super = FindletModule<SegmentRejecter>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentRejecterModule();
    };

    /**
     * Module implementation using the SegmentCreatorMCTruth
     */
    class TFCDC_SegmentCreatorMCTruthModule : public FindletModule<SegmentCreatorMCTruth> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorMCTruth>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SegmentCreatorMCTruthModule();
    };
  }
}
