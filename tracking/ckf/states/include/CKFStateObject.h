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

#include <genfit/MeasurementOnPlane.h>
#include <framework/logging/Logger.h>
#include <TVector3.h>
#include <TMatrixDSym.h>
#include <TVector.h>

namespace Belle2 {
  /**
   * State object to store one step in the CKF algorithm together with its parent (the state before), the hit
   * (e.g. a space point) and the seed (e.g. a cdc reco track).
   * The number shows, which step is stored.
   *
   * Please remember: the states are reused during the algorithm and only once constructed (in the C++ sense).
   * They are set using the "initialize" and "set" function to reuse resources. To save computation time in
   * copying, we do not set the mSoP in the set function. It is only set in the advancing step and during initialization.
   */
  template<class ASeedObject, class AHitObject>
  class CKFStateObject {
  public:
    /// Copy the class of the seed track
    using SeedObject = ASeedObject;
    /// Copy the class of the hits
    using HitObject = AHitObject;
    /// The class of the object returned by finalize()
    using ResultObject = CKFResultObject<SeedObject, HitObject>;

    /// Default constructor needed for STL containers
    CKFStateObject() = default;
    /// Move constructor needed for STL containers
    CKFStateObject(CKFStateObject&&) = default;
    /// No copy constructor
    CKFStateObject(const CKFStateObject&) = delete;
    /// No copy constructor
    CKFStateObject& operator=(const CKFStateObject&) = delete;

    /**
     * Initialize the state as a root without a related space point (but with a reco track)
     */
    CKFStateObject(SeedObject* seed, unsigned int number) :
      m_seedRecoTrack(seed), m_number(number), m_maximumNumber(number)
    {
      setMeasuredStateOnPlane(seed->getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0)));
    }

    /**
     * Set this state to be related to a space point and as child of the given state.
     *
     * ATTENTION: We do not set the mSoP here (or the cached positions),
     * as this is quite costly. This is only set in advancing and can not be accessed before (in all other cases the
     * parent's properties will be accessed).
     */
    void set(CKFStateObject* parent, const HitObject* hitObject)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_number = parent->getNumber() - 1;
      m_hitObject = hitObject;
      m_maximumNumber = parent->getMaximumNumber();

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

      const auto hitAdder = [&hits](const CKFStateObject * walkObject) {
        const HitObject* hitObject = walkObject->getHit();
        if (hitObject) {
          hits.push_back(hitObject);
        }
      };
      walk(hitAdder);

      double chi2 = 0;

      const auto chi2Adder = [&chi2](const CKFStateObject * walkObject) {
        if (walkObject->isFitted()) {
          chi2 += walkObject->getChi2();
        }
      };
      walk(chi2Adder);

      return ResultObject(getSeedRecoTrack(), hits, getMeasuredStateOnPlane(), chi2);
    }

    /// Return the parent state.
    const CKFStateObject* getParent() const
    {
      return m_parent;
    }

    /// Return the track this state is related to.
    SeedObject* getSeedRecoTrack() const
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

    /// Return the maximum number
    unsigned int getMaximumNumber() const
    {
      return m_maximumNumber;
    }

    /// Return the number of times no SP is attached to the track in all parents until the root.
    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFStateObject * walkObject) {
        if (not walkObject->getHit()) {
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
    bool operator<(const CKFStateObject& rhs) const
    {
      return getWeight() < rhs.getWeight();
    }

    /// Set the truth information
    void setTruthInformation(bool truthInformation)
    {
      m_truthInformation = truthInformation;
    }

    /// Return the truth information
    bool getTruthInformation() const
    {
      return m_truthInformation;
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
        B2ASSERT("You are asking for a parent property without a parent!", getParent());
        return getParent()->getMeasuredStateOnPlane();
      }
    }

    /// Get the 3d position of the mSoP (cached)
    const TVector3& getMSoPPosition() const
    {
      if (mSoPSet()) {
        return m_mSoPPosition;
      } else {
        B2ASSERT("You are asking for a parent property without a parent!", getParent());
        return getParent()->getMSoPPosition();
      }
    }

    /// Get the 3d momentum of the mSoP (cached)
    const TVector3& getMSoPMomentum() const
    {
      if (mSoPSet()) {
        return m_mSoPMomentum;

      } else {
        B2ASSERT("You are asking for a parent property without a parent!", getParent());
        return getParent()->getMSoPMomentum();
      }
    }

    /// Get the 6d covariance matrix of the mSoP (cached)
    const TMatrixDSym& getMSoPCovariance() const
    {
      if (mSoPSet()) {
        return m_mSoPCov;
      } else {
        B2ASSERT("You are asking for a parent property without a parent!", getParent());
        return getParent()->getMSoPCovariance();
      }
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

    /// Helper function to call a function on this and all parent states until the root.
    void walk(const std::function<void(const CKFStateObject*)> f) const
    {
      const CKFStateObject* walkObject = this;

      while (walkObject != nullptr) {
        f(walkObject);
        walkObject = walkObject->getParent();
      }
    }

  private:
    /// The seed reco track this state is related with.
    SeedObject* m_seedRecoTrack = nullptr;
    /// The hit object this state is attached to
    const HitObject* m_hitObject = nullptr;
    /// Where on the hierarchy this state is located.
    unsigned int m_number = 0;
    /// Where on the hierarchy the first state is located.
    unsigned int m_maximumNumber = 0;
    /// The parent state of this state
    const CKFStateObject* m_parent = nullptr;
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
    /// Temporary storage for the weight (used during filtering).
    double m_weight = 0;
    /// Truth information given by a teacher module
    bool m_truthInformation = false;
    /// Cache for the position of the mSoP. May be invalid if the mSoP is not set
    TVector3 m_mSoPPosition;
    /// Cache for the momentum of the mSoP. May be invalid if the mSoP is not set
    TVector3 m_mSoPMomentum;
    /// Cache for the cov of the mSoP. May be invalid if the mSoP is not set
    TMatrixDSym m_mSoPCov {6};

    /// Is the mSoP already set?
    bool mSoPSet() const
    {
      return m_hasMSoP;
    }
  };
}
