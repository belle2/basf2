/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RUNMETADATA_H
#define RUNMETADATA_H

#include <framework/gearbox/Unit.h>

namespace Belle2 {
  /** Store information that is constant during a run, and is needed during the event loop.
   *
   *  @todo Instead of retrieving hardcoded information from RunMetaData, these data
   *        should come from the database.
   */
  class RunMetaData {
  public:
    /** LER energy getter. */
    static float getEnergyLER() { return m_energyLER; }

    /** HER energy getter. */
    static float getEnergyHER() { return m_energyHER; }

    /** Crossing angle getter. */
    static float getCrossingAngle() { return m_crossingAngle; }

    /** LER angle getter (>0). */
    static float getAngleLER() { return m_angleLER; }

    /** set LER energy. */
    static void setEnergyLER(float e) { m_energyLER = e; }

    /** set HER energy. */
    static void setEnergyHER(float e) { m_energyHER = e; }


  private:
    /** prevent instantiation. */
    RunMetaData();
    ~RunMetaData();

    /** Center of Mass Energy.
     */
    static float m_energyLER;

    /** \f$\gamma\f$ factor in the lab frame.
     */
    static float m_energyHER;

    /** Angle between beams. */
    static const float m_crossingAngle;

    /** Angle in the lab system */
    static const float m_angleLER;

  }; //class
} // namespace Belle2
#endif
