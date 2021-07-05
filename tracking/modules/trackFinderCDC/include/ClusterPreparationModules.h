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

#include <tracking/trackFindingCDC/findlets/combined/ClusterPreparer.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the ClusterPreparer
     */
    class TFCDC_ClusterPreparerModule : public FindletModule<ClusterPreparer> {

      /// Type of the base class
      using Super = FindletModule<ClusterPreparer>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterPreparerModule();
    };

    /**
     * Module implementation using the SuperClusterCreator
     */
    class TFCDC_SuperClusterCreatorModule : public FindletModule<SuperClusterCreator> {

      /// Type of the base class
      using Super = FindletModule<SuperClusterCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SuperClusterCreatorModule();
    };

    /**
     * Module implementation using the ClusterRefiner
     */
    class TFCDC_ClusterRefinerModule : public FindletModule<ClusterRefiner<BridgingWireHitRelationFilter>> {

      /// Type of the base class
      using Super = FindletModule<ClusterRefiner<BridgingWireHitRelationFilter>>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterRefinerModule();
    };

    /**
     * Module implementation using the ClusterBackgroundDetector
     */
    class TFCDC_ClusterBackgroundDetectorModule : public FindletModule<ClusterBackgroundDetector> {

      /// Type of the base class
      using Super = FindletModule<ClusterBackgroundDetector>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterBackgroundDetectorModule();
    };
  }
}
