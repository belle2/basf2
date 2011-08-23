/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMRECT_H
#define BKLMRECT_H

#include "CLHEP/Vector/ThreeVector.h"

using namespace CLHEP;

namespace Belle2 {

  namespace bklm {

    //! Define a rectangle (for a BKLM module or strip)
    struct Rect {
      //! Positions of the four corners of the rectangle
      Hep3Vector corner[4];
    };

  } // end of namespace bklm

} // end of namespace Belle2
#endif // BKLMRECT_H
