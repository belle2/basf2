/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef COVARIANCEMATRIXINDICES_H
#define COVARIANCEMATRIXINDICES_H

#include "stddef.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Helper indices for meaningfull matrix access to the matrices entries
    const size_t iCurv = 0;
    const size_t iPhi0 = 1;
    const size_t iI = 2;

    const size_t iSZ = 3;
    const size_t iZ0 = 4;


  } // namespace CDCLocalTracking

} // namespace Belle2

#endif // COVARIANCEMATRIXINDICES_H
