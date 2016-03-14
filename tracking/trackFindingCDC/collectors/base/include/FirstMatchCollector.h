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
#include <tracking/trackFindingCDC/collectors/base/Collector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * A templated collector algorithm to take the matches as they are and add them to the collector
     * (regardless of any weights or double adding).
     */
    template <class AMatcherAlgorithm, class AAdderAlgorithm>
    class FirstMatchCollector : public Collector<AMatcherAlgorithm, AAdderAlgorithm> {
    public:
      typedef Collector<AMatcherAlgorithm, AAdderAlgorithm> Super;
      typedef typename Super::CollectorItem CollectorItem;
      typedef typename Super::CollectionItem CollectionItem;
      typedef typename Super::MatchedCollectionItem MatchedCollectionItem;

      virtual void collect(std::vector<CollectorItem>& collectorItems, const std::vector<CollectionItem>& collectionItems) override
      {
        for (CollectorItem& collectorItem : collectorItems) {
          const std::vector<MatchedCollectionItem>& matchesForCollector = this->m_matcherInstance.match(collectorItem, collectionItems);
          AAdderAlgorithm::add(collectorItem, matchesForCollector);
        }
      }
    };
  }
}
