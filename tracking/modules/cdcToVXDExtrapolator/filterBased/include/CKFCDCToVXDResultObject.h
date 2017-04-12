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

    unsigned int getLastLayer() const
    {
      return m_lastLayer;
    }

    void initialize(RecoTrack* seed)
    {
      m_seedRecoTrack = seed;
      m_measuredStateOnPlane = seed->getMeasuredStateOnPlaneFromFirstHit();
    }

    std::pair<RecoTrack*, std::vector<const SpacePoint*>> finalize() const
    {
      std::vector<const SpacePoint*> spacePoints;
      spacePoints.reserve(N);

      const CKFCDCToVXDStateObject* walkObject = this;

      while (walkObject != nullptr) {
        const SpacePoint* spacePoint = walkObject->getSpacePoint();
        if (spacePoint) {
          spacePoints.push_back(spacePoint);
        }
        walkObject = walkObject->getParent();
      }

      return std::make_pair(getSeedRecoTrack(), spacePoints);
    }

    const CKFCDCToVXDStateObject* getParent() const
    {
      return m_parent;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      return m_measuredStateOnPlane;
    }

    const SpacePoint* getSpacePoint() const
    {
      return m_spacePoint;
    }

    void buildFrom(const CKFCDCToVXDStateObject* parent, const SpacePoint* spacePoint)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_lastLayer = parent->getLastLayer() - 1;
      m_spacePoint = spacePoint;
      // TODO: Include the Kalman Filter part here!
      m_measuredStateOnPlane = parent->getMeasuredStateOnPlane();
    }

    void advance()
    {
      if (m_spacePoint) {
        m_measuredStateOnPlane.extrapolateToPoint(m_spacePoint->getPosition());
      }
    }

  private:
    RecoTrack* m_seedRecoTrack = nullptr;
    const SpacePoint* m_spacePoint = nullptr;
    unsigned int m_lastLayer = N;
    const CKFCDCToVXDStateObject* m_parent = nullptr;
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
  };
}