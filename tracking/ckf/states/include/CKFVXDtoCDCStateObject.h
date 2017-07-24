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
#include <tracking/ckf/states/CKFResultObject.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * State object to store one step in the CKF algorithm together with its parent (the state before), the hit
   * (a space point) and the seed (a cdc reco track).
   * The number shows, which step is stored.
   *
   * Please remember: the states are reused during the algorithm and only once constructed (in the C++ sense).
   * They are set using the "initialize" and "set" function to reuse resources. To save computation time in
   * copying, we do not set the mSoP in the set function. It is only set in the advancing step and during initialization.
   */
  class CKFVXDtoCDCStateObject {
  public:
    /**
     * How many states we need to describe the whole track.
     *
     * 12 = 2 * (SVD layers = 4) + 2 * (PXD layers = 2)
     */
    static constexpr unsigned int N = 12;

    /// The class of the seed track
    using SeedObject = RecoTrack;
    /// The class of the hits
    using HitObject = SpacePoint;
    /// The class of the object returned by finalize()
    using ResultObject = CKFResultObject<CKFVXDtoCDCStateObject>;

    CKFVXDtoCDCStateObject() = default;
    CKFVXDtoCDCStateObject(CKFVXDtoCDCStateObject&&) = default;
    CKFVXDtoCDCStateObject(const CKFVXDtoCDCStateObject&) = delete;
    CKFVXDtoCDCStateObject& operator=(const CKFVXDtoCDCStateObject&) = delete;

    /**
     * Initialize the state as a root without a related space point (but with a reco track)
     */
    CKFVXDtoCDCStateObject(RecoTrack* seed) :
      m_seedRecoTrack(seed)
    {
      setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneFromFirstHit());
    }

    /**
     * Set this state to be related to a space point and as child of the given state.
     *
     * ATTENTION: We do not set the mSoP here (or the cached positions),
     * as this is quite costly. This is only set in advancing and can not be accessed before (in all other cases the
     * parent's properties will be accessed).
     */
    void set(CKFVXDtoCDCStateObject* parent, const HitObject* hitObject)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_number = parent->getNumber() - 1;
      m_hitObject = hitObject;

      // Reset other state
      m_chi2 = 0;

      m_isFitted = false;
      m_isAdvanced = false;
      m_hasMSoP = false;

      m_weight = 0;
    }

    /**
     * Create a storable result object out of this state with all its parents
     * (more or less a vector of the space points together with the chi2 of the whole track).
     */
    ResultObject finalize() const
    {
      std::vector<const HitObject*> hits;
      hits.reserve(N);

      const auto& hitAdder = [&hits](const CKFVXDtoCDCStateObject * walkObject) {
        const HitObject* hitObject = walkObject->getHit();
        if (hitObject) {
          hits.push_back(hitObject);
        }
      };
      walk(hitAdder);

      double chi2 = 0;

      const auto& chi2Adder = [&chi2](const CKFVXDtoCDCStateObject * walkObject) {
        if (walkObject->isFitted()) {
          chi2 += walkObject->getChi2();
        }
      };
      walk(chi2Adder);

      return ResultObject(getSeedRecoTrack(), hits, chi2);
    }

    /// Return the parent state.
    const CKFVXDtoCDCStateObject* getParent() const
    {
      return m_parent;
    }

    /// Return the track this state is related to.
    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    /// Return the SP this state is related to. May be nullptr.
    const HitObject* getHit() const
    {
      return m_hitObject;
    }

    /// Return the number ( ~ layer * 2)
    unsigned int getNumber() const
    {
      return m_number;
    }

    /// Calculate the VXD layer this state is located on.
    unsigned int extractGeometryLayer() const
    {
      return static_cast<unsigned int>((static_cast<double>(m_number) / 2) + 1);
    }

    /// Check if this state should describe an overlap hit.
    bool isOnOverlapLayer() const
    {
      return m_number % 2 == 0;
    }

    /// Return the number of times no SP is attached to the track in all parents until the root.
    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFVXDtoCDCStateObject * walkObject) {
        if (not walkObject->getHit()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

    /// Return the number of times no SP is attached to the track in a non overlap layer in all parents until the root.
    unsigned int getNumberOfHolesOnNonOverlappingLayers() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFVXDtoCDCStateObject * walkObject) {
        if (not walkObject->getHit() and not walkObject->isOnOverlapLayer()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

    /// Return the chi2 set during fitting. Is only valid after fitting.
    double getChi2() const
    {
      B2ASSERT("Chi2 is not set", isFitted());
      return m_chi2;
    }

    /// Set the chi2 of this state (only of this hit) during fitting.
    void setChi2(double chi2)
    {
      m_chi2 = chi2;
    }

    /// Set the temporary weight during overlap check.
    void setWeight(double weight)
    {
      m_weight = weight;
    }

    /// Return the temporary weight during overlap check.
    double getWeight() const
    {
      return m_weight;
    }

    /// States can be ordered by weight (used during overlap check).
    bool operator<(const CKFVXDtoCDCStateObject& rhs)
    {
      return getWeight() < rhs.getWeight();
    }

    /// Set the mSoP
    void setMeasuredStateOnPlane(const genfit::MeasuredStateOnPlane& mSoP)
    {
      m_measuredStateOnPlane = mSoP;
      mSoP.getPosMomCov(m_mSoPPosition, m_mSoPMomentum, m_mSoPCov);
      m_hasMSoP = true;
    }

    /// Get the mSoP (or from the parent if not set already)
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      if (mSoPSet()) {
        return m_measuredStateOnPlane;
      } else {
        return getParent()->getMeasuredStateOnPlane();
      }
    }

    /// Get the 3d position of the mSoP (cached)
    const TVector3& getMSoPPosition() const
    {
      if (mSoPSet()) {
        return m_mSoPPosition;
      } else {
        return getParent()->getMSoPPosition();
      }
    }

    /// Get the 3d momentum of the mSoP (cached)
    const TVector3& getMSoPMomentum() const
    {
      if (mSoPSet()) {
        return m_mSoPMomentum;
      } else {
        return getParent()->getMSoPMomentum();
      }
    }

    /// Get the 6d covariance matrix of the mSoP (cached)
    const TMatrixDSym& getMSoPCovariance() const
    {
      if (mSoPSet()) {
        return m_mSoPCov;
      } else {
        return getParent()->getMSoPCovariance();
      }
    }

    /// Shortcut to get the hit position
    const B2Vector3<double>& getHitPosition() const
    {
      B2ASSERT("Hit is invalid", m_hitObject);
      return m_hitObject->getPosition();
    }

    /// Check if state was already fitted.
    bool isFitted() const
    {
      return m_isFitted;
    }

    /// Set that state was already fitted.
    void setFitted()
    {
      m_isFitted = true;
    }

    /// Check if state was already advanced.
    bool isAdvanced() const
    {
      return m_isAdvanced;
    }

    /// Set that state was already advanced.
    void setAdvanced()
    {
      m_isAdvanced = true;
    }

    bool mSoPSet() const
    {
      return m_hasMSoP;
    }

  private:
    /// The seed reco track this state is related with.
    RecoTrack* m_seedRecoTrack = nullptr;
    /// The hit object this state is attached to
    const HitObject* m_hitObject = nullptr;
    /**
     * Where on the hierarchy this state is located.
     *
     * Each layer is split into two numbers (so basically layer = number / 2),
     * the second number for each layer is to handle overlaps (two space points on the same layer).
     */
    unsigned int m_number = N;
    /// The parent state of this state
    CKFVXDtoCDCStateObject* m_parent = nullptr;
    /// Chi2 of this special state with this hit and this reco track. Will only be set after fitting state.
    double m_chi2 = 0;
    /// Flag, if this state was already fitted.
    bool m_isFitted = false;
    /// Flag, if this state was already advanced.
    bool m_isAdvanced = false;
    /// Flag, if this state has a valid mSoP
    bool m_hasMSoP = false;
    /// MSoP after advancing. Is undetermined before extrapolating!
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
    /// Temporary storage for the weight (used during overlap check).
    double m_weight = 0;
    /// Cache for the position of the mSoP. May be invalid if the mSoP is not set
    TVector3 m_mSoPPosition;
    /// Cache for the momentum of the mSoP. May be invalid if the mSoP is not set
    TVector3 m_mSoPMomentum;
    /// Cache for the cov of the mSoP. May be invalid if the mSoP is not set
    TMatrixDSym m_mSoPCov {6};

    /// Helper function to call a function on this and all parent states until the root.
    void walk(const std::function<void(const CKFVXDtoCDCStateObject*)> f) const
    {
      const CKFVXDtoCDCStateObject* walkObject = this;

      while (walkObject != nullptr) {
        f(walkObject);
        walkObject = walkObject->getParent();
      }
    }
  };
}
