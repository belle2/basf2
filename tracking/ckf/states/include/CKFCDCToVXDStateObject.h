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

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDStateObject {
  public:
    static constexpr unsigned int N = 8;

    using SeedObject = RecoTrack;
    using HitObject = SpacePoint;
    using ResultObject = TrackFindingCDC::WithWeight<std::pair<RecoTrack*, std::vector<const HitObject*>>>;

    void initialize(RecoTrack* seed)
    {
      m_seedRecoTrack = seed;

      // Reset other state
      m_hitObject = nullptr;
      m_number = N;
      m_parent = nullptr;

      m_chi2 = 0;

      m_isFitted = false;
      m_isAdvanced = false;

      // ATTENTION: We do not set the mSoP here, as this is quite costly.
    }

    void set(CKFCDCToVXDStateObject* parent, const HitObject* hitObject)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_number = parent->getNumber() - 1;
      m_hitObject = hitObject;

      // Reset other state
      m_chi2 = 0;

      m_isFitted = false;
      m_isAdvanced = false;

      // ATTENTION: We do not set the mSoP here, as this is quite costly.
    }

    ResultObject finalize() const
    {
      std::vector<const HitObject*> hits;
      hits.reserve(N);

      const auto& hitAdder = [&hits](const CKFCDCToVXDStateObject * walkObject) {
        const HitObject* hitObject = walkObject->getHit();
        if (hitObject) {
          hits.push_back(hitObject);
        }
      };
      walk(hitAdder);

      double chi2 = 0;

      const auto& chi2Adder = [&chi2](const CKFCDCToVXDStateObject * walkObject) {
        chi2 += walkObject->getChi2();
      };
      walk(chi2Adder);

      return ResultObject(std::make_pair(getSeedRecoTrack(), hits), chi2);
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

    const HitObject* getHit() const
    {
      return m_hitObject;
    }

    unsigned int getNumber() const
    {
      return m_number;
    }

    unsigned int extractGeometryLayer() const
    {
      return static_cast<unsigned int>((static_cast<double>(m_number) / 2) + 3);
    }

    bool isOnOverlapLayer() const
    {
      return m_number % 2 == 0;
    }

    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFCDCToVXDStateObject * walkObject) {
        if (not walkObject->getHit()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

    // chi2
    double getChi2() const
    {
      return m_chi2;
    }

    void setChi2(double chi2)
    {
      m_chi2 = chi2;
    }

    // weight
    void setWeight(double weight)
    {
      m_weight = weight;
    }

    double getWeight() const
    {
      return m_weight;
    }

    // ordering
    bool operator<(const CKFCDCToVXDStateObject& rhs)
    {
      return getWeight() < rhs.getWeight();
    }

    // mSoP
    genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane()
    {
      B2ASSERT("Measured on plane is not set", isAdvanced());
      return m_measuredStateOnPlane;
    }

    void setMeasuredStateOnPlane(const genfit::MeasuredStateOnPlane& mSoP)
    {
      m_measuredStateOnPlane = mSoP;
    }

    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneSavely() const
    {
      if (isAdvanced() and m_hitObject) {
        return m_measuredStateOnPlane;
      } else {
        return getMeasuredStateOnPlaneFromParent();
      }
    }

    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneFromParent() const
    {
      if (getParent()) {
        return getParent()->getMeasuredStateOnPlaneSavely();
      } else {
        return getSeedRecoTrack()->getMeasuredStateOnPlaneFromFirstHit();
      }
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
    const HitObject* m_hitObject = nullptr;
    unsigned int m_number = N;
    CKFCDCToVXDStateObject* m_parent = nullptr;

    double m_chi2 = 0;

    bool m_isFitted = false;
    bool m_isAdvanced = false;

    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;

    double m_weight = 0;

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
