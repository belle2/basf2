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

namespace Belle2 {

  //! Define the positions of the four corners of a rectangle
  struct BKLMRect {
    CLHEP::Hep3Vector corner[4];
  };

} // end of namespace Belle2
#endif // BKLMRECT_H
