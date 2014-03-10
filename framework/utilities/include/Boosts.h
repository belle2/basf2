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

#include <framework/gearbox/Unit.h>

class TLorentzRotation;

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
     *  For definition of the parameters, see the following:<br>
    <pre>

                                 Inverse Lab z-Axis
                                <_______________________________________________
    _ _ _ _ _                            *                   .........&lt;
      /   HER > _ _ _ _ _                *         .........&lt; LER
     /                   &gt; _ _ _ _ _     .*.......&lt;
     /                        .......&gt; _&lt;_ _ _ _
     /              .........&lt;                  &gt; _ _ _ _ _
      /   .........&lt;                                       &gt; _ _ _ _ _
    .........&lt;                       Lab z-Axis                           &gt;_ _ _ _ _
                                 defined by solenoidal field
                                 _____________________________________________________&gt;

    Symbol * represents angleLER.
    Symbol / represents crossAngle.
    </pre>
     *
     *  @param energyHER        Energy of high energy beam. The default value is for running on Y(4S).
     *  @param energyLER        Energy of low energy beam. The default value is for running on Y(4S).
     *  @param crossingAngle    Crossing Angle of the two beams. The default value is the design parameter.
     *  @param angleLER         Angle between the inverse direction of the LER and the solenoid magnetic field axis.
     *  @return                 <a href="http://root.cern.ch/root/html/TLorentzRotation.html">TLorentzRotation</a>
     *                          from the ROOT::physics Vector package, that translates a TLorentzVector from
     *                          the center of mass system of the beam interaction into the laboratory system.
     */
    static inline TLorentzRotation getCMS2LabBoost(const float& energyLER     = 4. * Unit::GeV,  const float& energyHER = 7.   * Unit::GeV,
                                                   const float& crossingAngle = 83.* Unit::mrad, const float& angleLER  = 41.5 * Unit::mrad);

    /** Get Lorentz boost for transformation from lab to CMS system.
     *
     *  @param energyHER        Energy of high energy beam. The default value is for running on Y(4S).
     *  @param energyLER        Energy of low energy beam. The default value is for running on Y(4S).
     *  @param crossingAngle    Crossing Angle of the two beams. The default value is the design parameter.
     *  @param angleLER         Angle between the inverse direction of the LER and the solenoid magnetic field axis.
     *  @return                 <a href="http://root.cern.ch/root/html/TLorentzRotation.html">TLorentzRotation</a>
     *                          from the ROOT::physics Vector package, that translates a TLorentzVector from
     *                          the laboratory system into the center of mass system of the beam interaction.
     */
    static TLorentzRotation getLab2CMSBoost(const float& energyLER     = 4. * Unit::GeV,  const float& energyHER = 7.   * Unit::GeV,
                                            const float& crossingAngle = 83.* Unit::mrad, const float& angleLER  = 41.5 * Unit::mrad);
  };
}

#endif
