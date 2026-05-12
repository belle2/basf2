/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/ClusterPreparer.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>

#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Module implementation using the ClusterPreparer
     */
    class TFCDC_ClusterPreparerModule : public TrackingUtilities::FindletModule<ClusterPreparer> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<ClusterPreparer>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterPreparerModule();
    };

    /**
     * Module implementation using the SuperClusterCreator
     */
    class TFCDC_SuperClusterCreatorModule : public TrackingUtilities::FindletModule<SuperClusterCreator> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<SuperClusterCreator>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_SuperClusterCreatorModule();
    };

    /**
     * Module implementation using the ClusterRefiner
     */
    class TFCDC_ClusterRefinerModule : public TrackingUtilities::FindletModule<ClusterRefiner<BridgingWireHitRelationFilter>> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<ClusterRefiner<BridgingWireHitRelationFilter>>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterRefinerModule();
    };

    /**
     * Module implementation using the ClusterBackgroundDetector
     */
    class TFCDC_ClusterBackgroundDetectorModule : public TrackingUtilities::FindletModule<ClusterBackgroundDetector> {

      /// Type of the base class
      using Super = TrackingUtilities::FindletModule<ClusterBackgroundDetector>;

    public:
      /// Constructor setting the default store vector names
      TFCDC_ClusterBackgroundDetectorModule();
    };
  }
}
