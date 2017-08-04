/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/ckf/findlets/base/HitSelector.h>
#include <tracking/ckf/findlets/base/StateTransformer.h>

namespace Belle2 {
  /**
   * General implementation of a tree search algorithm using a given class as seeds and hits
   * and two algorithm classes to decide, which child states should be used.
   * All combinations of childs are returned in the form of CKFResultObjects.
   * Make sure that your return vector can hold that much results (e.g. reserve space before to save time).
   *
   * The hit finder should return a Range/list of possible hits.
   * From each of those hits, a new state is created using the state transformator (actually, a pool
   * of states is reused, but this is hidden to the user).
   * The transformer algorithm has to take care, that if it returns iterators to temporary objects, that these
   * objects still live long enough (that is, they live as long as all child states of this state
   * are accessed, which is until the layer counter is increased again).
   *
   * The hit selector filter factory is then used to create a hit selector, to delete all states that should not
   * be used as new children and the algorithm is advanced.
   *
   * This class is very much optimized for calculation speed and performance, which leads to some
   * "unusual" code. E.g. we try not to construct state objects again and again, so we reuse a pool of N states
   * (we do not need more than that at a given time, as we traverse the result objects one-by-one).
   * This means however, that you have to make sure that the "set" and the "initialize" functions of the state
   * reset the object properly.
   *
   * In the end, the result object needs to return a copy of the objects, as the state objects
   * will be reused afterwards.
   */
  template<class ASeedObject, class AHitObject, class AHitFinder, class AHitSelectorFilterFactory, unsigned int MaxNumber>
  class TreeSearchFindlet : public TrackFindingCDC::Findlet <
    ASeedObject*,
    const AHitObject*,
    CKFResultObject<ASeedObject, AHitObject >> {
  public:
    /// The class of the seed
    using SeedPtr = ASeedObject*;
    /// The class of the hit
    using HitPtr = const AHitObject*;
    /// The class of the state
    using StateObject = CKFStateObject<ASeedObject, AHitObject>;
    /// The returned objects after tree traversal.
    using ResultObject = CKFResultObject<ASeedObject, AHitObject>;
    /// Parent class
    using Super = TrackFindingCDC::Findlet<SeedPtr, HitPtr, ResultObject>;

    /// Construct this findlet and add the subfindlet as listener
    TreeSearchFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_hitSelector);
      Super::addProcessingSignalListener(&m_hitFinder);
    }

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
      m_hitSelector.exposeParameters(moduleParamList, prefix);
      m_hitFinder.exposeParameters(moduleParamList, prefix);
    }

    /**
     * Main function of this findlet: traverse a tree starting from a given seed object,
     * ask the selector for the child states and traverse the tree starting from these child states.
     *
     * ATTENTION: As described above, the state objects are reused during the tree traversal. At a given time,
     * only the states from the current state up to the first state (the one with number = N) are valid.
     */
    void apply(std::vector<SeedPtr>& seedsVector, std::vector<HitPtr>& hitVector,
               std::vector<ResultObject>& results) override
    {
      m_hitFinder.initializeEventCache(seedsVector, hitVector);

      for (SeedPtr seed : seedsVector) {
        B2DEBUG(50, "Starting with new seed...");

        StateObject firstState(seed, MaxNumber);
        traverseTree(&firstState, results);
        B2DEBUG(50, "... finished with seed");

      }

      // Remove all empty results
      const auto& resultIsEmpty = [](const ResultObject & result) {
        return result.getHits().empty();
      };
      TrackFindingCDC::erase_remove_if(results, resultIsEmpty);
    }

  private:
    /// Subfindlet: hit selector
    HitSelector<AHitSelectorFilterFactory> m_hitSelector;

    /// Subfindlet: hit finder
    AHitFinder m_hitFinder;

    /// Subfindlet: state transformer
    StateTransformer<StateObject, MaxNumber> m_stateTransformer;

    /// Implementation of the traverseTree function
    void traverseTree(StateObject* currentState, std::vector<ResultObject>& resultsVector);
  };

  template<class ASeedObject, class AHitObject, class AHitFinder, class AHitSelectorFilterFactory, unsigned int MaxNumber>
  void TreeSearchFindlet<ASeedObject, AHitObject, AHitFinder, AHitSelectorFilterFactory, MaxNumber>::traverseTree(
    StateObject* currentState,
    std::vector<ResultObject>& resultsVector)
  {
    B2DEBUG(50, "Now on number " << currentState->getNumber());

    if (currentState->getNumber() == 0) {
      B2DEBUG(50, "Giving up this route, as this is the last possible state.");
      resultsVector.emplace_back(currentState->finalize());
      return;
    }

    // Ask hit finder, which hits are possible (in principle)
    const auto& matchingHits = m_hitFinder.getMatchingHits(*currentState);

    B2DEBUG(50, "Having found " << matchingHits.size() << " possible hits");

    // Transform the hits into states
    std::vector<StateObject*> childStates;
    m_stateTransformer.transform(matchingHits, childStates, currentState);

    // Filter out bad states
    m_hitSelector.apply(childStates);

    if (childStates.empty()) {
      B2DEBUG(50, "Giving up this route, as there are no possible child states.");
      resultsVector.emplace_back(currentState->finalize());
      return;
    }

    // Traverse the tree from each new state on
    B2DEBUG(50, "Having found " << childStates.size() << " child states.");
    for (StateObject* childState : childStates) {
      traverseTree(childState, resultsVector);
    }
  }
}