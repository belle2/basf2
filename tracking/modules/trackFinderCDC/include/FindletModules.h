/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentLinker.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentRejecter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

/******* Minimal Findlets **********/

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the ChooseableFacetFilter
     */
    class FacetCreatorModule : public FindletModule<FacetCreator> {

      /// Type of the base class
      using Super = FindletModule<FacetCreator>;

    public:
      /// Constructor setting the default store vector names
      FacetCreatorModule()
        : Super( {"CDCWireHitClusterVector", "CDCFacetVector"})
      {
      }
    };

    /**
     * Module implementation using the SegmentCreatorFacetAutomaton
     */
    class SegmentCreatorFacetAutomatonModule : public FindletModule<SegmentCreatorFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      SegmentCreatorFacetAutomatonModule()
        : Super( {"CDCFacetVector", "" /*to be set externally*/})
      {
      }
    };

    /**
     * Module implementation using the SegmentLinker
     */
    class SegmentLinkerModule : public FindletModule<SegmentLinker> {
    };

    /**
     * Module implementation using the SegmentOrienter
     */
    class SegmentOrienterModule : public FindletModule<SegmentOrienter> {
    };

    /**
     * Module implementation using the SegmentFitter
     */
    class SegmentFitterModule : public FindletModule<SegmentFitter> {
    };

    /**
     * Module implementation using the SegmentRejecter
     */
    class SegmentRejecterModule : public FindletModule<SegmentRejecter> {
    };

    /**
     * Module implementation using the SegmentCreatorMCTruth
     */
    class SegmentCreatorMCTruthModule : public FindletModule<SegmentCreatorMCTruth> {

      /// Type of the base class
      using Super = FindletModule<SegmentCreatorMCTruth>;

    public:
      /// Constructor setting the default store vector names
      SegmentCreatorMCTruthModule()
        : Super( {"CDCWireHitVector", "CDCSegment2DVector"})
      {
      }
    };
  }
}

/******* Combined Findlets **********/
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Combined Module using a combination of filters for segments
     */
    class SegmentFinderFacetAutomatonModule : public FindletModule<SegmentFinderFacetAutomaton> {

      /// Type of the base class
      using Super = FindletModule<SegmentFinderFacetAutomaton>;

    public:
      /// Constructor setting the default store vector names
      SegmentFinderFacetAutomatonModule()
        : Super( {"CDCWireHitVector", "CDCSegment2DVector"})
      {
      }
    };
  }
}
