/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef THRUSTOEVENT_H
#define THRUSTOEVENT_H

#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>
#include <vector>
#include <set>

namespace Belle2 {


  /**
   * Class for collecting variables related to the thrust of the event.
   *
   * Mainly used to compute the thrust of a tau-taubar event.
   */

  class ThrustOfEvent : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0.
     */
    ThrustOfEvent() : m_thrustAxis(0.0, 0.0, 0.0), m_thrust(0.0) {};

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

  private:

    // persistent data members
    TVector3 m_thrustAxis; /**< Thrust axis */

    float m_thrust;   /**< magnitude of thrust */

    ClassDef(ThrustOfEvent, 1) /**< class definition */

  };


} // end namespace Belle2

#endif
