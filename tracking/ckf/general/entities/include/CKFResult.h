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

#include <genfit/MeasuredStateOnPlane.h>
#include <TVector3.h>

#include <vector>

#include <framework/logging/Logger.h>

namespace Belle2 {
  /**
   * Object for temporary storage of a CKF tree search result.
   * It stores the seed together with the associated hits and a chi2 value.
   * This object will not be stored to the DataStore, but is only for internal usage.
   *
   * All sub-detector CKFs should inherit their own object from this.
   */
  template <class ASeed, class AHit>
  class CKFResult {

  public:
    /// Copy seed definition
    using Seed = ASeed;
    /// Copy hit definition
    using Hit = AHit;

    /// Constructor from the path of the result and the final mSoP, which defines the track position of the resulting track
    template <class AState>
    CKFResult(const std::vector<TrackFindingCDC::WithWeight<const AState*>>& path, const genfit::MeasuredStateOnPlane& mSoP)
      : m_seed(path.front()->getSeed()),
        m_trackPosition(mSoP.getPos()),
        m_trackMomentum(mSoP.getMom()),
        m_trackCharge(static_cast<short>(mSoP.getCharge())),
        m_seedMSoP(path.front()->getMeasuredStateOnPlane()),
        m_mSoP(mSoP)
    {
      m_hits.reserve(path.size());

      for (const TrackFindingCDC::WithWeight<const AState*> state : path) {
        const Hit* hit = state->getHit();
        if (hit) {
          m_hits.push_back(hit);
        }

        if (state->isFitted()) {
          const double stateChi2 = state->getChi2();
          m_chi2 += stateChi2;

          // The initial value of the maximal and minimal chi2 is NAN, so we always override this default value.
          if (stateChi2 > m_maximalChi2 or std::isnan(m_maximalChi2)) {
            m_maximalChi2 = stateChi2;
          }

          if (stateChi2 < m_minimalChi2 or std::isnan(m_minimalChi2)) {
            m_minimalChi2 = stateChi2;
          }
        }

        m_weightSum = state.getWeight();
      }
    }

    /// Getter for the stored hits
    const std::vector<const AHit*>& getHits() const
    {
      return m_hits;
    }

    /// Getter for the stored seeds
    const ASeed* getSeed() const
    {
      return m_seed;
    }

    /// Getter for the chi2
    double getChi2() const
    {
      return m_chi2;
    }

    /// Getter for the maximal chi2 of all stored hits. NAN means there is no valid chi2 at all.
    double getMaximalChi2() const
    {
      return m_maximalChi2;
    }

    /// Getter for the minimal chi2 of all stored hits. NAN means there is no valid chi2 at all.
    double getMinimalChi2() const
    {
      return m_minimalChi2;
    }

    /// Get the position this track should start at
    const TVector3& getPosition() const
    {
      return m_trackPosition;
    }

    /// Get the momentum this track should start at (defined at the position)
    const TVector3& getMomentum() const
    {
      return m_trackMomentum;
    }

    /// Set the charge of the track
    short getCharge() const
    {
      return m_trackCharge;
    }

    /// Getter for the sum of weights
    double getWeightSum() const
    {
      return m_weightSum;
    }

    /// Getter for the mSoP of the seed associated with this result
    const genfit::MeasuredStateOnPlane& getSeedMSoP() const
    {
      return m_seedMSoP;
    }

    /// Getter for the mSoP associated with this result
    const genfit::MeasuredStateOnPlane& getMSoP() const
    {
      return m_mSoP;
    }

  private:
    /// The stored seed
    const ASeed* m_seed;
    /// The stored hits
    std::vector<const AHit*> m_hits;
    /// The stored chi2
    double m_chi2 = 0;
    /// The maximal chi2 of the single states. NAN means there is no valid chi2 at all.
    double m_maximalChi2 = NAN;
    /// The minimal chi2 of the single states NAN means there is no valid chi2 at all.
    double m_minimalChi2 = NAN;
    /// The position this track should start at
    TVector3 m_trackPosition;
    /// The momentum this track should start at (defined at the position)
    TVector3 m_trackMomentum;
    /// The charge of the track
    short m_trackCharge = 0;
    /// The stored sum of weights
    TrackFindingCDC::Weight m_weightSum = 0;
    /// The measured state on plane, which was used from the seed
    genfit::MeasuredStateOnPlane m_seedMSoP;
    /// The measured state on plane, which this result was initialized with
    genfit::MeasuredStateOnPlane m_mSoP;
  };
}
