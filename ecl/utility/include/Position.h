/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
