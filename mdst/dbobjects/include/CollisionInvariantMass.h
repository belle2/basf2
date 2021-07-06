/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

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
     * @param error uncertainty of mean invariant mass
     * @param spread width of invariant mass distribution
     */
    void setMass(double mass, double error, double spread)
    {
      m_mass = mass;
      m_massError = error;
      m_massSpread = spread;
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

    /** Get the spread of the measured CMS energy */
    double getMassSpread() const
    {
      return m_massSpread;
    }

  private:

    /** Average center-of-mass energy = invariant mass of the colliding beams */
    Double32_t m_mass{0};

    /** Uncertainty of the average center-of-mass energy / invariant mass of the colliding beams */
    Double32_t m_massError{0};

    /** Width of the distribution of the center-of-mass energy / invariant mass of the colliding beams */
    Double32_t m_massSpread{0};

    ClassDef(CollisionInvariantMass, 1); /**< Measured average invariant mass of the colliding beams and its uncertainty **/
  };

} //Belle2 namespace
