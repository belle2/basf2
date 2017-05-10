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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

namespace Belle2 {
  template<class AStateObject, class AFilter, class AnAdvanceAlgorithm, class AFitterAlgorithm>
  class TreeSearchFindlet : public TrackFindingCDC::Findlet <
    typename AStateObject::SeedObject*,
    const typename AStateObject::HitObject*,
    std::pair<typename AStateObject::SeedObject*, std::vector<const typename AStateObject::HitObject*>>
        > {
  public:
    using SeedPtr = typename AStateObject::SeedObject*;
    using HitPtr = const typename AStateObject::HitObject*;
    using StateObject = AStateObject;
    using StateArray = typename std::array < AStateObject, AStateObject::N + 1 >;
    using StateIterator = typename StateArray::iterator;
    using ResultPair = std::pair<typename AStateObject::SeedObject*, std::vector<const typename AStateObject::HitObject*>>;

    using Super = TrackFindingCDC::Findlet<SeedPtr, HitPtr, ResultPair>;
    /// Construct this findlet and add the two filters as listeners
    TreeSearchFindlet() : Super()
    {
      Super::addProcessingSignalListener(&m_firstFilter);
      Super::addProcessingSignalListener(&m_secondFilter);
      Super::addProcessingSignalListener(&m_thirdFilter);
      Super::addProcessingSignalListener(&m_advanceAlgorithm);
      Super::addProcessingSignalListener(&m_fitterAlgorithm);
    }

    /// Expose the parameters of the two filters and the makeHitJumpsPossible parameter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "first"));
      m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "second"));
      m_thirdFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "third"));

      m_advanceAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "advance"));
      m_fitterAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fitter"));

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "makeHitJumpingPossible"), m_param_makeHitJumpingPossible,
                                    "", m_param_makeHitJumpingPossible);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "advance"), m_param_advance,
                                    "Do the advance step.", m_param_advance);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fit"), m_param_fit,
                                    "Do the fit step.", m_param_fit);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNResults"), m_param_useNResults,
                                    "Do only use the best N results.", m_param_useNResults);
    }

    /// Main function of this findlet: traverse a tree starting from a given seed object.
    void apply(std::vector<SeedPtr>& seedsVector, std::vector<HitPtr>& hitVector,
               std::vector<ResultPair>& results) override
    {
      initializeEventCache(seedsVector, hitVector);

      for (SeedPtr seed : seedsVector) {
        initializeSeedCache(seed);

        StateIterator firstStateIterator = m_states.begin();
        firstStateIterator->initialize(seed);
        traverseTree(firstStateIterator, results);
      }
    }

  protected:
    /// Overloadable function to return the possible range of next hit objects for a given state
    virtual TrackFindingCDC::SortedVectorRange<HitPtr> getMatchingHits(StateObject& currentState) = 0;

    virtual void initializeEventCache(std::vector<SeedPtr>& seedsVector, std::vector<HitPtr>& hitVector) { }

    virtual void initializeSeedCache(SeedPtr seed) { }

  private:
    /// Object pool for states
    StateArray m_states{};

    /// Temporary object pool for finding the next state
    std::array < std::vector<StateObject>, StateObject::N + 1 > m_temporaryStates;

    /// Parameter: make hit jumps possible
    bool m_param_makeHitJumpingPossible = true;

    /// Parameter: do the advance step
    bool m_param_advance = true;

    /// Parameter: do the fit step
    bool m_param_fit = true;

    /// Parameter:
    unsigned int m_param_useNResults = 5;

    /// Subfindlet: Filter 1
    AFilter m_firstFilter;
    /// Subfindlet: Filter 2
    AFilter m_secondFilter;
    /// Subfindlet: Filter 3
    AFilter m_thirdFilter;
    /// Subfindlet: Advancer
    AnAdvanceAlgorithm m_advanceAlgorithm;
    /// Subfindlet: Fitter
    AFitterAlgorithm m_fitterAlgorithm;

    /// Implementation of the traverseTree function
    void traverseTree(StateIterator currentState, std::vector<ResultPair>& resultsVector)
    {
      StateIterator nextState = std::next(currentState);

      if (nextState == m_states.end()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      // TODO: factor this out into another findlet
      // Filter and apply the advance & fit functions
      auto childStates = filterFromHits(*currentState, m_firstFilter, 2 * m_param_useNResults);

      if (m_param_advance) {
        applyAndFilter(childStates, m_advanceAlgorithm, 0);
      }

      applyAndFilter(childStates, m_secondFilter, m_param_useNResults);

      if (m_param_fit) {
        applyAndFilter(childStates, m_fitterAlgorithm, 0);
      }

      applyAndFilter(childStates, m_thirdFilter, m_param_useNResults);

      if (childStates.empty()) {
        resultsVector.emplace_back(currentState->finalize());
        return;
      }

      for (StateObject& childState : childStates) {
        // Attention: In the moment we only have states from the temporary list!
        std::swap(*nextState, childState);
        traverseTree(nextState, resultsVector);
      }
    }

    // This method does more or less the same as the one below -> can this be combined?
    template <class APredicate>
    TrackFindingCDC::VectorRange<StateObject> filterFromHits(StateObject& parentState, APredicate& predicate, unsigned int useNResults)
    {
      const auto& matchingHits = getMatchingHits(parentState);
      auto& temporaryStates = m_temporaryStates[parentState.getNumber()];

      temporaryStates.resize(matchingHits.size() + 1);

      auto lastFiniteState = temporaryStates.begin();
      for (const auto& hit : matchingHits) {
        lastFiniteState->set(&parentState, hit);
        TrackFindingCDC::Weight weight = predicate(*lastFiniteState);

        if (not std::isnan(weight)) {
          lastFiniteState->setWeight(weight);
          lastFiniteState = std::next(lastFiniteState);
        }
      }

      if (m_param_makeHitJumpingPossible) {
        lastFiniteState->set(&parentState, nullptr);
        TrackFindingCDC::Weight weight = predicate(*lastFiniteState);

        if (not std::isnan(weight)) {
          lastFiniteState->setWeight(weight);
          lastFiniteState = std::next(lastFiniteState);
        }
      }

      TrackFindingCDC::VectorRange<StateObject> childStates(temporaryStates.begin(), lastFiniteState);

      // TODO: THE SAME
      // Select only the N best if necessary (otherwise we do not have to sort at all).
      if (useNResults > 0 and childStates.size() > useNResults) {
        std::sort(childStates.begin(), childStates.end());
        childStates.second = std::next(childStates.begin(), m_param_useNResults);
      }

      return childStates;
    }

    template <class APredicate>
    void applyAndFilter(TrackFindingCDC::VectorRange<StateObject>& childStates, APredicate& predicate, unsigned int useNResults)
    {
      if (childStates.empty()) {
        return;
      }

      auto firstFiniteState = childStates.begin();
      for (auto& state : childStates) {
        TrackFindingCDC::Weight weight = predicate(state);
        if (std::isnan(weight)) {
          std::swap(*firstFiniteState, state);
          firstFiniteState = std::next(firstFiniteState);
        } else {
          state.setWeight(weight);
        }
      }

      childStates.first = firstFiniteState;

      // TODO: THE SAME
      // Select only the N best if necessary (otherwise we do not have to sort at all).
      if (useNResults > 0 and childStates.size() > useNResults) {
        std::sort(childStates.begin(), childStates.end());
        childStates.second = std::next(childStates.begin(), m_param_useNResults);
      }
    }
  };
}