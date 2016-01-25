/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/collectors/base/FirstMatchCollector.h>
#include <tracking/trackFindingCDC/collectors/quadtree/QuadTreeBasedMatcher.h>

#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilter.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoHit3D;
    class CDCRLTaggedWireHit;
    class CDCTrajectory2D;

    class StereoHitTrackMatcherFilter {
    public:
      typedef CDCTrack CollectorItem;
      typedef CDCRLTaggedWireHit CollectionItem;

      /// Empty desctructor. Everything is handled via terminate.
      virtual ~StereoHitTrackMatcherFilter() { }

      /// Expose the parameters to the module.
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "");

      /**
       * Use the given filter (via the module parameters) to find a matching.
       */
      std::vector<WithWeight<const CollectionItem*>> match(const CollectorItem& collectorItem,
                                                           const std::vector<CollectionItem>& collectionList);

      /// Initialize the filter.
      virtual void initialize()
      {
        m_stereoHitFilter = std::move(m_filterFactory.create());
        m_stereoHitFilter->initialize();

        if (m_stereoHitFilter->needsTruthInformation()) {
          StoreArray <CDCSimHit>::required();
          StoreArray <MCParticle>::required();
        }
      }

      /// Terminate the filter.
      virtual void terminate()
      {
        m_stereoHitFilter->terminate();
      }

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers)
      bool m_param_checkForB2BTracks = true;
      StereoHitFilterFactory m_filterFactory;
      std::unique_ptr<BaseStereoHitFilter> m_stereoHitFilter;
    };
  }
}
