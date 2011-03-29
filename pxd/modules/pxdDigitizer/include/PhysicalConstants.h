/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Peter Kvasnicka                           *
 *                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PHYSICALCONSTANTS_H
#define PHYSICALCONSTANTS_H 1

namespace Belle2 {

  namespace SiDigi {

    //
    // System of units
    //
    //  Basic units:
    //
    //    charge        in electrons         [C]
    //    distance      in milimetres        [mm]
    //    energy        in kiloelectronvolts [keV]
    //    mag. filed    in Tesla             [T]
    //    temperature   in Kelvin            [K]
    //    time          in seconds           [s]
    //    voltage       in volts             [V]
    //
    // author: Z.Drasal, Charles University Prague
    //

    // Elementary charge
    static const float       e = 1.;

    // Charge
    static const float       C = 1 / 1.602176462E-19*e;
    static const float      fC = C / 1.E15;

    // Distance
    static const float      mm = 1.;
    static const float      cm = mm * 10  ;
    static const float       m = mm * 1.E3;
    static const float      um = mm / 1.E3;

    // Energy
    static const float      eV = 1.;
    static const float     keV = eV * 1.E3;
    static const float     MeV = eV * 1.E6;
    static const float     GeV = eV * 1.E9;

    // Temperature
    static const float       K = 1.;

    // Time
    static const float       s = 1.;
    static const float      ms = s / 1.E3;
    static const float      us = s / 1.E6;
    static const float      ns = s / 1.E9;

    // Voltage
    static const float       V = 1.;

    // Magnetic field
    static const float       T = 1.*V*s / m / m;

    //
    //  Basic constants:
    //
    //    Boltzmann constant [eV/K]
    //    Energy needed for creation of 1 e-h pair [eV]
    //    Pi                 [1]

    // Pi
    static const double     pi = 3.14159265358979323846;
    static const double piHalf = pi / 2;

    // Boltzmann constant in eV/K
    static const float       k = 8.617343 * 1.E-5 * eV / K;

    // Energy needed for e-h pair creation
    static const float     Eeh = 3.65  * eV;

    // Permittivity of Si
    static const double Perm_Si = 11.9 * 8.8542 *1.E-18 *C / V / um;

    // Thermal Voltage at room temperature
    static const double Utherm = 0.026*V;

    // Electron mobility in intrinsic Si at room temperature
    static const double e_mobility = 1415 * cm*cm / V / s;

    // Particle physics
    static const double e_mass = 0.510999 * MeV;
    static const double fine_str_const = 1. / 137.036;

  } // Namespace SiDigi
} // Namespace Belle2

#endif // PHYSICALCONSTANTS_H
