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

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Helper indices for meaningfull matrix access to the matrices entries
    constexpr size_t iCurv = 0;
    constexpr size_t iPhi0 = 1;
    constexpr size_t iI = 2;

    constexpr size_t iSZ = 3;
    constexpr size_t iZ0 = 4;


  } // namespace CDCLocalTracking

} // namespace Belle2

#endif // COVARIANCEMATRIXINDICES_H
