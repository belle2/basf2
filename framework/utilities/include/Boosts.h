/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Heck                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BOOSTS_H
#define BOOSTS_H

#include "TLorentzRotation.h"


namespace Belle2 {


  /** This class handles Lorentz boosts between various systems.
   *
   *  You don't have to construct an object of this class yourself, but you can directly
   *  call the functions of the class by writing e.g.
   *  Boosts::getCMS2LabB...
   */
  class Boosts {
  public:

    /** Get Lorentz boost for transformation from CMS to lab system.
     *
     *  @param EHER         Energy of high energy beam. The default value is for running on Y(4S).
     *  @param ELER         Energy of low energy beam. The default value is for running on Y(4S).
     *  @param crossAngle   Crossing Angle of the two beams. The default value is the design parameter.
     *  @param angle        Angle between the inverse direction of the LER and the solenoid magnetic field axis.
     */
    TLorentzRotation getCMS2LabBoost(double EnergyHER, double EnergyLER, double crossAngle, double angle);

    /** Get Lorentz boost for transformation from lab to CMS system.
     *
     *  @param EHER         Energy of high energy beam. The default value is for running on Y(4S).
     *  @param ELER         Energy of low energy beam. The default value is for running on Y(4S).
     *  @param crossAngle   Crossing Angle of the two beams. The default value is the design parameter.
     *  @param angle        Angle between the inverse direction of the LER and the solenoid magnetic field axis.
     */
    TLorentzRotation getLab2CMSBoost(double EnergyHER, double EnergyLER, double crossAngle, double angle);
  };
}

#endif
