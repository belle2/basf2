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

namespace Belle2 {
  /** Interface to the accelerator parameters stored in the database/gearbox.
   */
  class RunMetaData {
  public:
    /** LER energy getter. */
    static float getEnergyLER();

    /** HER energy getter. */
    static float getEnergyHER();

    /** Crossing angle getter. */
    static float getCrossingAngle();

    /** LER angle getter (>0). */
    static float getAngleLER();


  private:
    /** prevent instantiation. */
    RunMetaData();
    ~RunMetaData();
  }; //class
} // namespace Belle2
#endif
