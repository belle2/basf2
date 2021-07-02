/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calculation of shower positions using lists of digits and weights.     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>

// FRAMEWORK
#include <framework/geometry/B2Vector3.h>

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
    B2Vector3D computePositionLiLo(const std::vector<ECLCalDigit>& digits, std::vector<double>& weights,
                                   const std::vector<double>& parameters);

    //! Computes the sum of all energies.
    /*!
     \param digits List of ECLCalDigits
     \param weights List of weights
     */
    double computeEnergySum(const std::vector<ECLCalDigit>& digits, std::vector<double>& weights);

  } // namespace ECL
} // namespace Belle2
