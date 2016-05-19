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
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilter.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>

#include <tracking/trackFindingCDC/collectors/base/FilterBasedMatcher.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRLWireHit;

    /// A Matcher Algorithm for adding stereo hits to tracks using a configurable filter (e.g. TMVA).
    class StereoHitTrackMatcherFilter : public FilterBasedMatcher<StereoHitFilterFactory> {
    public:
      /// Use tracks as collector items.
      typedef CDCTrack CollectorItem;

      /// Use rl tagged wire hits a collection items.
      typedef CDCRLWireHit CollectionItem;

      /// Empty desctructor. Everything is handled via terminate.
      virtual ~StereoHitTrackMatcherFilter() { }

      /// Expose the parameters to the module.
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "") override;

      /**
       * Use the given filter (via the module parameters) to find a matching.
       */
      std::vector<WithWeight<const CollectionItem*>> match(const CollectorItem& collectorItem,
                                                           const std::vector<CollectionItem>& collectionList);

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;
    };
  }
}
