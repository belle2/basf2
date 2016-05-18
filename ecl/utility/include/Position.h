/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calculation of shower positions using lists of digits and weights      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLPOSITION_H_
#define ECLPOSITION_H_

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>

// ROOT
#include "TVector3.h"

// OTHER
#include <vector>

namespace Belle2 {
  namespace ECL {
    //! Computes the (shower) position using log linear weights.
    /*!
     \param digits List of ECLCalDigits
     \param weights List of weights
     \param parameters List of parameters
     */
    TVector3 computePositionLiLo(std::vector<ECLCalDigit>& digits, std::vector<double>& weights, std::vector<double>& parameters);

    //! Computes the sum of all energies.
    /*!
     \param digits List of ECLCalDigits
     \param weights List of weights
     */
    double computeEnergySum(std::vector<ECLCalDigit>& digits, std::vector<double>& weights);

    //! Find the id of the closest crystal. This can go wrong if the position is not in the digit list.
    /*!
     \param digits List of ECLCalDigits
     \param direction weighted shower direction
     */
    int findClosestCrystal(std::vector<ECLCalDigit>& digits, TVector3& direction);

  } // namespace ECL
} // namespace Belle2

#endif
