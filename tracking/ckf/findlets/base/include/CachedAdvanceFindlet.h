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
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/ckf/findlets/base/AdvanceAlgorithm.h>
#include <tracking/ckf/utilities/SelectionAlgorithms.h>

#include <TVector3.h>
#include <genfit/MeasurementOnPlane.h>

namespace Belle2 {
  template <class AState>
  class CachedAdvanceFindlet : public TrackFindingCDC::Findlet<AState> {
  private:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<AState>;

    /// A type that is used as a key for the cache map - the normal of the plane.
    using Key = TVector3;

    /// Helper functor for printing something with B2DEBUG
    struct B2DEBUGPrinter {
      /// Marker function for the isFunctor test
      operator TrackFindingCDC::FunctorTag();

      /// Operator for printing
      template<class T1>
      void operator()(const T1& t1) const
      {
        B2DEBUG(50, t1);
      }
    };

  public:
    /// Constructor adding the subfindlets as listeners
    CachedAdvanceFindlet();

    /// Expose the parameters of the filters and our own parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Main function of this findlet: call extrapolation on all states and return only the valid ones.
    void apply(std::vector<AState>& childStates) override;

  private:
    /// Parametere: use caching
    bool m_param_useCaching = true;
    /// Subfindlet: Advance Algorithm
    AdvanceAlgorithm m_advanceAlgorithm;

    /// Cache for the extrapolated measured state on planes
    std::vector<std::pair<Key, genfit::MeasuredStateOnPlane>> m_cachedMSoPs;

    /// With the help of the advance algorithm, to the extrapolation but only in cases where we have not cached the result already
    bool advanceWithCache(const AState& currentState)
    {
      B2ASSERT("Encountered invalid state", not currentState->isFitted() and not currentState->isAdvanced());

      const auto* hit = currentState->getHit();

      if (not hit) {
        // If we do not have a hit, we do not need to do anything here.
        currentState->setAdvanced();
        return true;
      }

      GeometryLayerExtractor extractGeometryLayer;
      OverlapExtractor isOnOverlapLayer;
      B2DEBUG(50, "Now advancing to state on " << currentState->getNumber()
              << " which is on " << extractGeometryLayer(*currentState) << " and "
              << isOnOverlapLayer(*currentState) << " " << HitIDExtractor()(currentState));
      B2DEBUG(50, "Coming from:");
      currentState->walk(TrackFindingCDC::Composition<B2DEBUGPrinter, HitIDExtractor>());

      // Get the parent to check, if we have already seem it and have something in our cache
      B2ASSERT("How could a state without a parent end up here?", currentState->getParent());

      // Normally, we would just use the (updated) mSoP of the parent and extrapolate it to the
      // plane, this state/hit is located on. So we start with a copy of the mSoP of the parent here ...
      genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState->getMeasuredStateOnPlane();
      // ... however, to speed things up, we first check if the plane of the cached mSoP
      //  and our own plane is the same.
      const genfit::SharedPlanePtr& plane = m_advanceAlgorithm.getPlane(measuredStateOnPlane, *hit);

      const TVector3& normal = plane->getNormal();

      if (m_param_useCaching) {

        const auto sameNormal = [&normal](const std::pair<Key, genfit::MeasuredStateOnPlane>& pair) {
          return pair.first == normal;
        };
        const auto& cachedItem = std::find_if(m_cachedMSoPs.begin(), m_cachedMSoPs.end(), sameNormal);

        if (cachedItem != m_cachedMSoPs.end()) {
          // We have already calculated this! so we can just reuse the cached mSoP
          B2DEBUG(50, "No extrapolation needed!");
          const genfit::MeasuredStateOnPlane& cachedMeasuredStateOnPlane = cachedItem->second;

          B2ASSERT("U vector must be the same!", plane->getU() == cachedMeasuredStateOnPlane.getPlane()->getU());
          B2ASSERT("V vector must be the same!", plane->getV() == cachedMeasuredStateOnPlane.getPlane()->getV());

          const double oldU = cachedMeasuredStateOnPlane.getState()[3];
          const double oldV = cachedMeasuredStateOnPlane.getState()[4];
          const TVector3& oldO = cachedMeasuredStateOnPlane.getPlane()->getO();
          const TVector3& newO = plane->getO();
          const double newU = (oldO - newO).Dot(plane->getU()) + oldU;
          const double newV = (oldO - newO).Dot(plane->getV()) + oldV;

          genfit::MeasuredStateOnPlane translatedCachedMeasuredStateOnPlane = cachedMeasuredStateOnPlane;
          translatedCachedMeasuredStateOnPlane.getState()[3] = newU;
          translatedCachedMeasuredStateOnPlane.getState()[4] = newV;
          translatedCachedMeasuredStateOnPlane.setPlane(plane);

          currentState->setMeasuredStateOnPlane(translatedCachedMeasuredStateOnPlane);
          currentState->setAdvanced();
          return true;
        }
      }

      B2DEBUG(50, "Extrapolation needed!");

      // this means the two are not equal, so we have to do the extrapolation. We start with the mSoP of the
      // parent state.
      if (not m_advanceAlgorithm.extrapolateToPlane(measuredStateOnPlane, plane)) {
        return false;
      }

      // we update the cache for the next hit - anticipating we will visit this sensor plane again in the next round
      currentState->setMeasuredStateOnPlane(measuredStateOnPlane);
      currentState->setAdvanced();

      if (m_param_useCaching) {
        m_cachedMSoPs.emplace_back(normal, measuredStateOnPlane);
      }

      return true;
    }
  };

  template <class AState>
  CachedAdvanceFindlet<AState>::CachedAdvanceFindlet() : Super()
  {
    Super::addProcessingSignalListener(&m_advanceAlgorithm);

    m_cachedMSoPs.reserve(10);
  }

  template <class AState>
  void CachedAdvanceFindlet<AState>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_advanceAlgorithm.exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useCaching"), m_param_useCaching,
                                  "Use caching during the extrapolation (handle with care).",
                                  m_param_useCaching);
  }

  template <class AState>
  void CachedAdvanceFindlet<AState>::apply(std::vector<AState>& childStates)
  {
    if (childStates.empty()) {
      return;
    }

    // Sort the states to make the caching work better
    std::sort(childStates.begin(), childStates.end(), TrackFindingCDC::LessOf<HitIDExtractor>());

    m_cachedMSoPs.clear();
    for (auto* state : childStates) {
      if (advanceWithCache(state)) {
        state->setWeight(1);
      } else {
        state->setWeight(NAN);
      }
    }

    TrackFindingCDC::erase_remove_if(childStates, TrackFindingCDC::IndirectTo<TrackFindingCDC::HasNaNWeight>());
  }
}
