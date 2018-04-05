/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva, Ami Rostomyan               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTSHAPE_H
#define EVENTSHAPE_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <vector>
#include <set>

namespace Belle2 {


  /**
   * Class for collecting variables related to the event shape.
   *
   * Mainly used to compute the thrust, missing momentum/energy/mass of qqbar continuum and tau-taubar events.
   */

  class EventShape : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0.
     */
    EventShape() : m_thrustAxis(0.0, 0.0, 0.0), m_thrust(0.0), m_missingMomentumCMS(0.0, 0.0, 0.0), m_missingMomentumMagCMS(0.0),
      m_missingMomentum(0.0, 0.0, 0.0),
      m_missingMomentumMag(0.0), m_missingEnergyCMS(0.0), m_missingMass2(0.0), m_visibleEnergy(0.0) {};

    // setters
    /**
     * Add thrust axis.
     *
     * @param TVector3 thrust axis
     */
    void addThrustAxis(TVector3 thrustAxis);

    /**
     * Add magnitude of thrust.
     *
     * @param Float magnitude of thrust
     */
    void addThrust(float thrust);

    /**
     * Add the missing momentum vector in CMS.
     *
     * @param TVector3 missing momentum
     */
    void addMissingMomentumCMS(TVector3 missingMomentumCMS);

    /**
     * Add the missing momentum magnitude in CMS.
     *
     * @param float missing momentum magnitude
     */
    void addMissingMomentumMagCMS(float missingMomentumMagCMS);

    /**
     * Add the missing momentum vector in lab.
     *
     * @param TVector3 missing momentum
     */
    void addMissingMomentum(TVector3 missingMomentum);

    /**
     * Add the missing momentum magnitude in lab.
     *
     * @param float missing momentum magnitude
     */
    void addMissingMomentumMag(float missingMomentumMag);

    /**
     * Add missing energy in CMS.
     *
     * @param Float missing energy
     */
    void addMissingEnergyCMS(float missingEnergyCMS);

    /**
     * Add missing mass squared.
     *
     * @param Float missing mass squared
     */
    void addMissingMass2(float missingMass2);

    /**
     * Add visible energy of the event.
     *
     * @param Float visible energy
     */
    void addVisibleEnergy(float visibleEnergy);

    // getters
    /**
     * Get thrust axis.
     *
     * @return TVector3 thrust axis
     */
    TVector3 getThrustAxis(void) const
    {
      return m_thrustAxis;
    }

    /**
     * Get magnitude of thrust.
     *
     * @return Float magnitude of thrust
     */
    float getThrust(void) const
    {
      return m_thrust;
    }

    /**
     * Get missing momentum vector in CMS.
     *
     * @return TVector3 missing momentum
     */
    TVector3 getMissingMomentumCMS(void) const
    {
      return m_missingMomentumCMS;
    }

    /**
     * Get missing momentum magnitude in CMS.
     *
     * @return float missing momentum magnitude
     */
    float getMissingMomentumMagCMS(void) const
    {
      return m_missingMomentumMagCMS;
    }

    /**
     * Get missing momentum vector in lab.
     *
     * @return TVector3 missing momentum
     */
    TVector3 getMissingMomentum(void) const
    {
      return m_missingMomentum;
    }

    /**
     * Get missing momentum magnitude in lab.
     *
     * @return float missing momentum magnitude
     */
    float getMissingMomentumMag(void) const
    {
      return m_missingMomentumMag;
    }

    /**
    * Get missing energy in CMS.
    *
    * @return Float missing energy
    */
    float getMissingEnergyCMS(void) const
    {
      return m_missingEnergyCMS;
    };

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
    float getVisibleEnergy(void) const
    {
      return m_visibleEnergy;
    }

  private:

    // persistent data members
    TVector3 m_thrustAxis; /**< Thrust axis */
    float m_thrust;   /**< magnitude of thrust */

    TVector3 m_missingMomentumCMS; /**< Missing momentum of the event in CMS*/
    float m_missingMomentumMagCMS; /**< magnitude of the missing momentum in CMS */

    TVector3 m_missingMomentum; /**< Missing momentum of the event in lab*/
    float m_missingMomentumMag;  /**< magnitude of the missing momentum in lab */

    float m_missingEnergyCMS; /**< Missing energy of the event in CMS  */
    float m_missingMass2; /**< Missing mass squared computed from  m_missingMomentumCMS and m_missingEnergyCMS */

    float m_visibleEnergy; /**< Visible energy of the event in CMS  */

    ClassDef(EventShape, 1) /**< class definition */

  };


} // end namespace Belle2

#endif
