/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <Math/Vector3D.h>

namespace Belle2 {


  /**
   * Class for collecting variables related to the global kinematics of the event
   *
   * Mainly used to compute the  missing momentum/energy/mass of qqbar continuum and tau-taubar events.
   */

  class EventKinematics : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0.
     */
    EventKinematics(bool builtFromMC = false) :  m_missingMomentum(0.0, 0.0, 0.0), m_missingMomentumCMS(0.0, 0.0, 0.0),
      m_missingEnergyCMS(0.0), m_missingMass2(0.0), m_visibleEnergyCMS(0.0), m_photonsEnergy(0.0), m_builtFromMC(builtFromMC) {};

    // setters

    /**
     * Add the missing momentum vector in lab.
     *
     * @param missingMomentum missing momentum vector
     */
    void addMissingMomentum(const ROOT::Math::XYZVector& missingMomentum);

    /**
     * Add the missing momentum vector in CMS.
     *
     * @param missingMomentumCMS missing momentum vector
     */
    void addMissingMomentumCMS(const ROOT::Math::XYZVector& missingMomentumCMS);

    /**
     * Add missing energy in CMS.
     *
     * @param missingEnergyCMS missing energy
     */
    void addMissingEnergyCMS(float missingEnergyCMS);

    /**
     * Add missing mass squared.
     *
     * @param missingMass2 missing mass squared
     */
    void addMissingMass2(float missingMass2);

    /**
     * Add visible energy of the event in CMS.
     *
     * @param visibleEnergyCMS visible energy
     */
    void addVisibleEnergyCMS(float visibleEnergyCMS);

    /**
     * Add total energy of photons in the event.
     *
     * @param totalPhotonsEnergy total photon energy
     */
    void addTotalPhotonsEnergy(float totalPhotonsEnergy);


    // getters

    /**
     * Get missing momentum vector in lab.
     *
     * @return XYZVector missing momentum
     */
    ROOT::Math::XYZVector getMissingMomentum(void) const
    {
      return m_missingMomentum;
    }

    /**
     * Get missing momentum vector in CMS.
     *
     * @return XYZVector missing momentum
     */
    ROOT::Math::XYZVector getMissingMomentumCMS(void) const
    {
      return m_missingMomentumCMS;
    }

    /**
    * Get missing energy in CMS.
    *
    * @return Float missing energy
    */
    float getMissingEnergyCMS(void) const
    {
      return m_missingEnergyCMS;
    }

    /**
     * Get missing mass squared.
     *
     * @return Float missing mass squared
     */
    float getMissingMass2(void) const
    {
      return m_missingMass2;
    }

    /**
     * Get visible energy of the event.
     *
     * @return Float visible energy
     */
    float getVisibleEnergyCMS(void) const
    {
      return m_visibleEnergyCMS;
    }

    /**
     * Get total energy of photons in the event.
     *
     * @return Float total energy of photons
     */
    float getTotalPhotonsEnergy(void) const
    {
      return m_photonsEnergy;
    }



  private:

    // persistent data members
    ROOT::Math::XYZVector m_missingMomentum; /**< Missing momentum of the event in lab*/
    ROOT::Math::XYZVector m_missingMomentumCMS; /**< Missing momentum of the event in CMS*/

    float m_missingEnergyCMS; /**< Missing energy of the event in CMS  */
    float m_missingMass2; /**< Missing mass squared computed from  m_missingMomentumCMS and m_missingEnergyCMS */

    float m_visibleEnergyCMS; /**< Visible energy of the event in CMS  */

    float m_photonsEnergy; /**< Total energy of photons in lab */

    bool m_builtFromMC; /**< Is from MC switch */
    ClassDef(EventKinematics, 3) /**< class definition */

  };


} // end namespace Belle2

