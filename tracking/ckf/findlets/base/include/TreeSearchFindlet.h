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

namespace Belle2 {
  /**
   * General implementation of a tree search algorithm using a given class as states
   * and an algorithm class to decide, which child states should be used.
   * All combinations of childs are returned in the form of ResultObjects.
   * Make sure that your return vector can hold that much results (e.g. reserve space before to save time).
   *
   * The hit selector should return a Range/list of possible child states. The selector
   * algorithm has to take care, that if it returns iterators to temporary objects, that these
   * objects still live long enough (that is, they live as long as all child states of this state
   * are accessed, which is until the layer counter is increased again).
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
  template<class AStateObject, class AHitSelector>
  class TreeSearchFindlet : public TrackFindingCDC::Findlet <
    typename AStateObject::SeedObject*,
    const typename AStateObject::HitObject*,
    typename AStateObject::ResultObject > {
  public:
    /// The class of the seed
    using SeedPtr = typename AStateObject::SeedObject*;
    /// The class of the hit
    using HitPtr = const typename AStateObject::HitObject*;
    /// A C++ array of states
    using StateArray = typename std::array < AStateObject, AStateObject::N + 1 >;
    /// An iterator through the states in the pool array
    using StateIterator = typename StateArray::iterator;
    /// The returned objects after tree traversal.
    using ResultObject = typename AStateObject::ResultObject ;

    using Super = TrackFindingCDC::Findlet<SeedPtr, HitPtr, ResultObject>;

    /// Construct this findlet and add the subfindlet as listener
    TreeSearchFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_hitSelector);
    }

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
      m_hitSelector.exposeParameters(moduleParamList, prefix);
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
      m_hitSelector.initializeEventCache(seedsVector, hitVector);

      for (SeedPtr seed : seedsVector) {
        StateIterator firstStateIterator = m_states.begin();
        firstStateIterator->initialize(seed);

        traverseTree(firstStateIterator, results);
      }
    }

  private:
    /// Object pool for states
    StateArray m_states{};

    /// Subfindlet: hit selector
    AHitSelector m_hitSelector;

    /// Implementation of the traverseTree function
    void traverseTree(StateIterator currentState, std::vector<ResultObject>& resultsVector)
    {
      StateIterator nextState = std::next(currentState);

      if (nextState == m_states.end()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      const auto& childStates = m_hitSelector.getChildStates(*currentState);

      if (childStates.empty()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      for (AStateObject& childState : childStates) {
        std::swap(*nextState, childState);
        traverseTree(nextState, resultsVector);
      }
    }
  };
}