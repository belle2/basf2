/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Hernandez Villanueva                              *
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
   * Class for collecting variables related to the thrust of the event.
   *
   * Mainly used to compute the thrust of a tau-taubar event.
   */

  class EventShape : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0.
     */
    EventShape() : m_thrustAxis(0.0, 0.0, 0.0), m_thrust(0.0), m_missingMomentum(0.0, 0.0, 0.0) {};

    // setters
    /**
     * Add thrust axis.
     *
     * @param TVector3 thrust axis
     */
    void addThrustAxis(TVector3 thrustAxis);

    // setters
    /**
     * Add the missing momentum value.
     *
     * @param TVector3 missing momentum
     */
    void addMissingMomentum(TVector3 missingMomentum);

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
     * Get missing momentum vector.
     *
     * @return TVector3 missing momentum
     */
    TVector3 getMissingMomentum(void) const
    {
      return m_missingMomentum;
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

    TVector3 m_missingMomentum; /**< Missing momentum of the event */

    ClassDef(EventShape, 1) /**< class definition */

  };


} // end namespace Belle2

#endif
