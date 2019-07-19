/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  /** This class contains the measured average center-of-mass energy,
   * which is equal to the invariant mass of the colliding beams, and its uncertainty.
   */
  class CollisionInvariantMass: public TObject {
  public:

    /** equality operator */
    bool operator==(const CollisionInvariantMass& other) const
    {
      return other.m_mass == m_mass && other.m_massError == m_massError;
    }

    /** Set the CMS energy and its uncertainty.
     * @param mass invariant mass of the colliding beams
     * @param error uncertainty of invariant mass
     */
    void setMass(double mass, double error)
    {
      m_mass = mass;
      m_massError = error;
    }

    /** Get the measured average CMS energy */
    double getMass() const
    {
      return m_mass;
    }

    /** Get the uncertainty of the measured average CMS energy */
    double getMassError() const
    {
      return m_massError;
    }

  private:

    /** Average center-of-mass energy = invariant mass of the colliding beams */
    Double32_t m_mass{0};

    /** Uncertainty of the average center-of-mass energy / invariant mass of the colliding beams */
    Double32_t m_massError{0};

    ClassDef(CollisionInvariantMass, 1); /**< Measured average invariant mass of the colliding beams and its uncertainty **/
  };

} //Belle2 namespace
