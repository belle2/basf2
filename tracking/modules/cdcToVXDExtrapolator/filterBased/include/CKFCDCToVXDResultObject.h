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
      m_chi2 = m_seedRecoTrack->getTrackFitStatus()->getChi2();
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

    void buildFrom(const CKFCDCToVXDStateObject* parent, const SpacePoint* spacePoint)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_lastLayer = parent->getLastLayer() - 1;
      m_spacePoint = spacePoint;
      m_measuredStateOnPlane = parent->getMeasuredStateOnPlane();
      m_chi2 = parent->getChi2();
      m_lastChi2 = parent->getChi2();
    }

    // Getters
    const CKFCDCToVXDStateObject* getParent() const
    {
      return m_parent;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane()
    {
      return m_measuredStateOnPlane;
    }

    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      return m_measuredStateOnPlane;
    }

    const SpacePoint* getSpacePoint() const
    {
      return m_spacePoint;
    }

    unsigned int getLastLayer() const
    {
      return m_lastLayer;
    }

    double getChi2() const
    {
      return m_chi2;
    }

    double& getChi2()
    {
      return m_chi2;
    }

    double getLastChi2() const
    {
      return m_lastChi2;
    }

    double& getLastChi2()
    {
      return m_lastChi2;
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

    unsigned int getState() const
    {
      return m_state;
    }

    void setState(unsigned int state)
    {
      m_state = state;
    }

  private:
    RecoTrack* m_seedRecoTrack = nullptr;
    const SpacePoint* m_spacePoint = nullptr;
    unsigned int m_lastLayer = N;
    const CKFCDCToVXDStateObject* m_parent = nullptr;
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
    double m_chi2 = 0;
    double m_lastChi2 = 0;
    unsigned int m_state = 0;

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
