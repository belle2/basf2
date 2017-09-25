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
    using SeedObject = ASeed;
    /// Copy hit definition
    using HitObject = AHit;

    /// Constructor
    CKFResult(ASeed* seed, const std::vector<const AHit*> hits, const genfit::MeasuredStateOnPlane& mSoP,
              double chi2) :
      m_seed(seed), m_hits(hits), m_chi2(chi2)
    {
      m_trackCharge = mSoP.getCharge();
      m_trackMomentum = mSoP.getMom();
      m_trackPosition = mSoP.getPos();
    }

    /// Getter for the stored hits
    const std::vector<const AHit*>& getHits() const
    {
      return m_hits;
    }

    /// Getter for the stored seeds
    ASeed* getSeed() const
    {
      return m_seed;
    }

    /// Getter for the chi2
    double getChi2() const
    {
      return m_chi2;
    }

    /// Getter for the teacher information
    TrackFindingCDC::Weight getTeacherInformation() const
    {
      return m_teacherInformation;
    }

    /// Setter for the teacher information
    void setTeacherInformation(TrackFindingCDC::Weight teacherInformation)
    {
      m_teacherInformation = teacherInformation;
    }

    /// Get the position this track should start at
    const TVector3& getPosition() const
    {
      return m_trackPosition;
    }

    /// Get the position this track should start at
    void setPosition(const TVector3& position)
    {
      m_trackPosition = position;
    }

    /// Get the momentum this track should start at (defined at the position)
    const TVector3& getMomentum() const
    {
      return m_trackMomentum;
    }

    /// Set the momentum this track should start at (defined at the position)
    void setMomentum(const TVector3& momentum)
    {
      m_trackMomentum = momentum;
    }

    /// Set the charge of the track
    short getCharge() const
    {
      return m_trackCharge;
    }

    /// Set the charge of the track
    void setCharge(short charge)
    {
      m_trackCharge = charge;
    }

  private:
    /// The stored seed
    ASeed* m_seed;
    /// The stored hits
    std::vector<const AHit*> m_hits;
    /// The stored chi2
    double m_chi2;
    /// A weight, which transports the teacher information
    TrackFindingCDC::Weight m_teacherInformation = NAN;
    /// The position this track should start at
    TVector3 m_trackPosition;
    /// The momentum this track should start at (defined at the position)
    TVector3 m_trackMomentum;
    /// The charge of the track
    short m_trackCharge = 0;
  };
}
