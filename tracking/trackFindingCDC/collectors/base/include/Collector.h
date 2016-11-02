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
#include <memory>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A templated and configurable algorithm designed class for performing a "collection" of items.
     * The idea is to have both a list of collection items and collector items. Via a matcher algorithm,
     * you can specify if and with which weight a pair of collection and collector items can be matched.
     * The implemented collection algorithm (in the moment BestMatch or FirstMatch) then handles the
     * rest and flattens this matrix of relations to one match per *collection item*
     * (so one collector item can have more than one collection items but not vice versa).
     * The given adder algorithm then uses these matches to do the matching (e.g. adding hits to the track).
     *
     * AMatcherAlgorithm must provide:
     *  - a function exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "")
     *  - a function initialize()
     *  - a function terminate()
     *  - two member types CollectorItem, CollectionItem
     *  - a function std::vector<MatchedCollectionItem> match(const CollectorItem& collectorItem, const std::vector<CollectionItem>& collectionList)
     *    with MatchedCollectionItem = WithWeight<const CollectionItem*> and the CollectionItem-pointers pointing to items in the collectionList.
     *
     * AAdderAlgorithm must provide:
     *  - a static function void add(CollectorItem&, const std::vector<MatchedCollectionItem>&)
     */
    template<class AMatherAlgorithm, class AAdderAlgorithm>
    class Collector {
    public:
      /// Copy the CollectorItem from the matcher algorithm.
      using CollectorItem = typename AMatherAlgorithm::CollectorItem;
      /// Copy the CollectionItem from the matcher algorithm.
      using CollectionItem = typename AMatherAlgorithm::CollectionItem;
      /// Shortcut for CollectionItem pointers with weight.
      using MatchedCollectionItem = WithWeight<const CollectionItem*>;

      /// Empty desctructor.
      virtual ~Collector() = default;

      /** Redirect all parameters of the matcher. **/
      void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "")
      {
        m_matcherInstance.exposeParameters(moduleParameters, prefix);
      }

      /** Redirect the initialization of the matcher. **/
      void initialize()
      {
        m_matcherInstance.initialize();
      }

      /** Redirect the termination of the matcher. **/
      void terminate()
      {
        m_matcherInstance.terminate();
      }

      /** Implement this collection function for your use case. See FirstMatchCollector for a simple example. **/
      virtual void collect(std::vector<CollectorItem>& collectorList, const std::vector<CollectionItem>& collectionItems) = 0;

    protected:
      /// The instance of the matcher algorithm we use.
      AMatherAlgorithm m_matcherInstance;
    };
  }
}
