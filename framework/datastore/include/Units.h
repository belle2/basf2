/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Heck                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNITS_H
#define UNITS_H

#include <TMath.h>

namespace Belle2 {

  //! Definition of units
  /*! \file
      It defines the standard units that should be used everywhere.
      If the value of a variable is not given in standard units the variable name
      has to clearly indicate the used units.
      \author <a href="mailto:martin.heck@kit.edu,thomas.kuhr@kit.edu?subject=Units">Martin Heck, Thomas Kuhr</a>
  */

  // standard units
  const double cm  = 1.;  //!< Standard of [length]
  const double ns  = 1.;  //!< Standard of [time]
  const double rad = 1.;  //!< Standard of [angle]
  const double GeV = 1.;  //!< Standard of [energy, momentum, mass]
  const double K   = 1.;  //!< Standard of [temperature]
  const double T   = 1.;  //!< Standard of [magnetic field]
  const double e   = 1.;  //!< Standard of [electric charge]

  // length units
  const double km  = cm * 1e5;
  const double m   = cm * 1e2;
  const double mm  = m  * 1e-3;
  const double um  = m  * 1e-6;
  const double nm  = m  * 1e-7;
  const double fm  = m  * 1e-12;

  // area units
  const double m2  = m * m;
  const double cm2 = cm * cm;
  const double mm2 = mm * mm;

  const double b   = m2 * 1e-28;
  const double mb  = b  * 1e-3;
  const double ub  = b  * 1e-6;
  const double nb  = b  * 1e-9;
  const double pb  = b  * 1e-12;
  const double fb  = b  * 1e-15;
  const double ab  = b  * 1e-18;

  // volume units
  const double m3  = m * m * m;
  const double cm3 = cm * cm * cm;
  const double mm3 = mm * mm * mm;

  // time units
  const double s   = ns * 1e9;
  const double ms  = s  * 1e-3;
  const double us  = s  * 1e-6;
  const double ps  = s  * 1e-12;
  const double fs  = s  * 1e-15;

  // angle units
  const double mrad = rad * 1e-3;
  const double deg  = TMath::DegToRad();

  // energy units
  const double eV  = GeV * 1e-9;
  const double keV = eV  * 1e3;
  const double MeV = eV  * 1e6;

} // end namespace Belle2

#endif  // UNITS_H
