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

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDStateObject {
  public:
    static constexpr unsigned int N = 7;

    using SeedObject = RecoTrack;
    using HitObject = SpacePoint;

    void initialize(RecoTrack* seed)
    {
      m_seedRecoTrack = seed;
      m_measuredStateOnPlane = seed->getMeasuredStateOnPlaneFromFirstHit();
    }

    std::pair<RecoTrack*, std::vector<const SpacePoint*>> finalize() const
    {
      std::vector<const SpacePoint*> spacePoints;
      spacePoints.reserve(N);

      const auto& spacePointAdder = [&spacePoints](const CKFCDCToVXDStateObject * walkObject) {
        const SpacePoint* spacePoint = walkObject->getSpacePoint();
        if (spacePoint) {
          spacePoints.push_back(spacePoint);
        }
      };
      walk(spacePointAdder);

      return std::make_pair(getSeedRecoTrack(), spacePoints);
    }

    // Important: set all values here!
    void buildFrom(CKFCDCToVXDStateObject* parent, const SpacePoint* spacePoint)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_layer = parent->getLayer() - 1;
      m_spacePoint = spacePoint;

      m_measuredStateOnPlane = parent->getMeasuredStateOnPlane();
      m_cachedMeasuredStateOnPlane = parent->getMeasuredStateOnPlane();
      m_hasCache = false;

      m_isFitted = false;
      m_isAdvanced = false;

      m_chi2 = 0;
    }

    // const Getters
    const CKFCDCToVXDStateObject* getParent() const
    {
      return m_parent;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    const SpacePoint* getSpacePoint() const
    {
      return m_spacePoint;
    }

    unsigned int getLayer() const
    {
      return m_layer;
    }

    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFCDCToVXDStateObject * walkObject) {
        if (not walkObject->getSpacePoint()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

    // mSoP handling
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      return m_measuredStateOnPlane;
    }

    genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane()
    {
      return m_measuredStateOnPlane;
    }

    const genfit::MeasuredStateOnPlane& getParentsCachedMeasuredStateOnPlane() const
    {
      return m_parent->m_cachedMeasuredStateOnPlane;
    }

    genfit::MeasuredStateOnPlane& getParentsCachedMeasuredStateOnPlane()
    {
      return m_parent->m_cachedMeasuredStateOnPlane;
    }

    bool parentHasCache() const
    {
      return m_parent->m_hasCache;
    }

    void setParentHasCache()
    {
      m_parent->m_hasCache = true;
    }

    // chi2
    double getChi2() const
    {
      return m_chi2;
    }

    double& getChi2()
    {
      return m_chi2;
    }

    // State control
    bool isFitted() const
    {
      return m_isFitted;
    }

    void setFitted()
    {
      m_isFitted = true;
    }

    bool isAdvanced() const
    {
      return m_isAdvanced;
    }

    void setAdvanced()
    {
      m_isAdvanced = true;
    }

  private:
    RecoTrack* m_seedRecoTrack = nullptr;
    const SpacePoint* m_spacePoint = nullptr;
    unsigned int m_layer = N;
    CKFCDCToVXDStateObject* m_parent = nullptr;

    double m_chi2 = 0;

    bool m_isFitted = false;
    bool m_isAdvanced = false;

    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;

    bool m_hasCache = false;
    genfit::MeasuredStateOnPlane m_cachedMeasuredStateOnPlane;


    void walk(const std::function<void(const CKFCDCToVXDStateObject*)> f) const
    {
      const CKFCDCToVXDStateObject* walkObject = this;

      while (walkObject != nullptr) {
        f(walkObject);
        walkObject = walkObject->getParent();
      }
    }
  };
}
