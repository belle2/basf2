/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <framework/logging/Logger.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /**
   * State object to store one step in the CKF algorithm together with its parent (the state before), the hit
   * (e.g. a space point) and the seed (e.g. a cdc reco track).
   *
   * Please remember: the states are reused during the algorithm and only once constructed (in the C++ sense).
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

    /// Set the chi2 of this state during fitting.
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

    /// Get the mSoP if already set during extrapolation (or fitting)
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

    /// Is the mSoP already set? (= state was already extrapolated)
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

    /// Cache containing the most important information of this state which will often be needed
    struct stateCache {
      /// getPtOfSeed (if seed state)
      float ptSeed = 0.;
      /// Phi of the sensor this state is based on, only set if based on SpacePoint
      float sensorCenterPhi = std::numeric_limits<float>::quiet_NaN();
      /// Phi if this state (from SpacePoint or RecoTrack)
      float phi = std::numeric_limits<float>::quiet_NaN();
      /// Theta if this state (from SpacePoint or RecoTrack)
      float theta = std::numeric_limits<float>::quiet_NaN();
      /// Local normalized uCoordinate of this state, only set if based on SpacePoint
      float localNormalizedu = std::numeric_limits<float>::quiet_NaN();
      /// Local normalized vCoordinate of this state, only set if based on SpacePoint
      float localNormalizedv = std::numeric_limits<float>::quiet_NaN();
      /// Geometrical Layer this state is based on. Will be set to the SVD layer number if it's a state based on a SpacePoint, to an artificial layer else by using getGeometricalLayer()
      unsigned short geoLayer = std::numeric_limits<unsigned short>::quiet_NaN();
      /// Ladder this state is based on (only use for SpacePoint based states)
      unsigned short ladder = std::numeric_limits<unsigned short>::quiet_NaN();
      /// VxdID of this state, only set if based on SpacePoint
      VxdID sensorID = VxdID(0);
      /// Is this state based on a hit (=SpacePoint) or a seed (=RecoTrack)
      bool isHitState = false;
    };

  protected:
    /// Cache the most important data of this state for better runtime performance
    stateCache m_stateCache;


  private:
    /// The seed this state is related with.
    const Seed* m_seed = nullptr;
    /// The hit object this state is attached to
    const Hit* m_hit = nullptr;
    /// Chi2 of this special state with this hit and this reco track. Will only be set after fitting state.
    double m_chi2 = 0;
    /// MSoP after advancing. Is undetermined before extrapolating!
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
    /// Memory for the automaton cell.
    TrackFindingCDC::AutomatonCell m_automatonCell;
    /// Flag, if this state was already fitted.
    bool m_isFitted = false;
    /// Flag, if this state has a valid mSoP
    bool m_hasMSoP = false;
  };
}
