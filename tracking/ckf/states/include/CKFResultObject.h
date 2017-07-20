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

namespace Belle2 {
  /**
   * Object for temporary storage of a CKF tree search result.
   * It stores the seed together with the associated hits and a chi2 value.
   * Additionally, it has a storage option for a weight, which can be used to transport e.g. MC information
   * to the filters.
   * This object will not be stored to the DataStore, but is only for internal usage.
   */
  template <class AStateObject>
  class CKFResultObject {
    /// Copy the hit object definition from the state
    using HitObject = typename AStateObject::HitObject;
    // / Copy the seed object definition from the state
    using SeedObject = typename AStateObject::SeedObject;

  public:
    /// Constructor
    CKFResultObject(SeedObject* seed, const std::vector<const HitObject*> hits, double chi2) : m_seed(seed), m_hits(hits),
      m_chi2(chi2) {}

    /// Getter for the stored hits
    const std::vector<const HitObject*>& getHits() const
    {
      return m_hits;
    }

    /// Getter for the stored seeds
    SeedObject* getSeed() const
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

  private:
    /// The stored seed
    SeedObject* m_seed;
    /// The stored hits
    std::vector<const HitObject*> m_hits;
    /// The stored chi2
    double m_chi2;
    /// A weight, which transports the teacher information
    TrackFindingCDC::Weight m_teacherInformation = NAN;
  };
}
