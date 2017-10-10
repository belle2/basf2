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

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <genfit/MeasurementOnPlane.h>
#include <TVector3.h>

#include <vector>

#include <framework/logging/Logger.h>

namespace Belle2 {
  /**
   * Object for temporary storage of a CKF tree search result.
   * It stores the seed together with the associated hits and a chi2 value.
   * Additionally, it has a storage option for a weight, which can be used to transport e.g. MC information
   * to the filters.
   * This object will not be stored to the DataStore, but is only for internal usage.
   */
  template <class ASeed, class AHit>
  class CKFResult {

  public:
    /// Copy seed definition
    using Seed = ASeed;
    /// Copy hit definition
    using Hit = AHit;

    /// Constructor
    template <class AState>
    CKFResult(const std::vector<const AState*>& path, const genfit::MeasuredStateOnPlane& mSoP)
    {
      for (const AState* state : path) {
        const Hit* hit = state->getHit();
        if (hit) {
          m_hits.push_back(hit);
        }

        if (state->isFitted()) {
          m_chi2 += state->getChi2();
        }
      }

      m_trackCharge = mSoP.getCharge();
      m_trackMomentum = mSoP.getMom();
      m_trackPosition = mSoP.getPos();

      m_seed = path.front()->getSeed();
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

  private:
    /// The stored seed
    const ASeed* m_seed;
    /// The stored hits
    std::vector<const AHit*> m_hits;
    /// The stored chi2
    double m_chi2 = 0;
    /// The position this track should start at
    TVector3 m_trackPosition;
    /// The momentum this track should start at (defined at the position)
    TVector3 m_trackMomentum;
    /// The charge of the track
    short m_trackCharge = 0;
  };
}
