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

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <framework/logging/Logger.h>

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
  template<class ASeed, class AHit>
  class CKFState {
  public:
    /// Copy the class of the seed track
    using Seed = ASeed;
    /// Copy the class of the hits
    using Hit = AHit;

    /// Default constructor needed for STL containers
    CKFState() = default;
    /// Move constructor needed for STL containers
    CKFState(CKFState&&) = default;
    /// No copy constructor
    CKFState(const CKFState&) = delete;
    /// No copy constructor
    CKFState& operator=(const CKFState&) = delete;
    /// Move assignment constructor
    CKFState& operator=(CKFState&&) = default;
    /// Destructor
    ~CKFState() = default;

    /// Initialize the state as a root without a related hit (but with a seed)
    explicit CKFState(const Seed* seed) : m_seed(seed)
    {
    }

    /// Initialize the state as non-root with a related hit (and with a seed)
    explicit CKFState(const Hit* hit) : m_hit(hit)
    {
    }

    /// Return the track this state is related to.
    const Seed* getSeed() const
    {
      return m_seed;
    }

    /// Return the SP this state is related to. May be nullptr.
    const Hit* getHit() const
    {
      return m_hit;
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
      m_isFitted = true;
    }

    /// Set the mSoP
    void setMeasuredStateOnPlane(const genfit::MeasuredStateOnPlane& mSoP)
    {
      m_measuredStateOnPlane = mSoP;
      m_hasMSoP = true;
    }

    /// Get the mSoP (or from the parent if not set already)
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      B2ASSERT("You are asking for an invalid variable!", mSoPSet());
      return m_measuredStateOnPlane;
    }

    /// Check if state was already fitted.
    bool isFitted() const
    {
      return m_isFitted;
    }

    /// Is the mSoP already set?
    bool mSoPSet() const
    {
      return m_hasMSoP;
    }

    /// Reset the state to be seen as "new"
    void reset()
    {
      m_isFitted = false;
      m_hasMSoP = false;
    }

    /// Getter for the automaton cell.
    TrackFindingCDC::AutomatonCell& getAutomatonCell()
    {
      return m_automatonCell;
    }


  private:
    /// The seed this state is related with.
    const Seed* m_seed = nullptr;
    /// The hit object this state is attached to
    const Hit* m_hit = nullptr;
    /// Chi2 of this special state with this hit and this reco track. Will only be set after fitting state.
    double m_chi2 = 0;
    /// Flag, if this state was already fitted.
    bool m_isFitted = false;
    /// Flag, if this state has a valid mSoP
    bool m_hasMSoP = false;
    /// MSoP after advancing. Is undetermined before extrapolating!
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
    /// Memory for the automaton cell.
    TrackFindingCDC::AutomatonCell m_automatonCell;
  };
}
