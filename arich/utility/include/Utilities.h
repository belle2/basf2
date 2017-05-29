/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHUTILITIES_H
#define ARICHUTILITIES_H

#include <vector>
#include <stdlib.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {
  namespace arich {

    double ringArea(double thetach, double thetatrk, double dist = 0.18)
    {
      double s = M_PI * dist * dist * sqrt(tan(thetach)) * pow((tan(thetach + thetatrk) + tan(thetach - thetatrk)) / 2, 3. / 2.);
      return s;
    };

  } // namespace arich
} // namespace Belle2

#endif
