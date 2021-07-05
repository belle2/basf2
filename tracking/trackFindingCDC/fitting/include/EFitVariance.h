/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <stdexcept>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Enumeration for the variance information to be used.
    enum class EFitVariance {
      /// Use an equal variance of 1 for all observations
      c_Unit = 0,

      /// Use the fixed nominal drift length variance for all observations
      c_Nominal = 1,

      /// Use the drift length as variance.
      c_DriftLength = 2,

      /// Use the square of the drift length as variance.
      c_Pseudo = 3,

      /// Use the drift length variance
      c_Proper = 4,
    };

    /**
     *  Helper function to translate user provided strings to the enum values of the variance information
     *  information used in the two dimensional fit.
     */
    inline EFitVariance getFitVariance(const std::string& fitVarianceString)
    {
      if (fitVarianceString == std::string("unit")) {
        return EFitVariance::c_Unit;
      } else if (fitVarianceString == std::string("nominal")) {
        return EFitVariance::c_Nominal;
      } else if (fitVarianceString == std::string("driftLength")) {
        return EFitVariance::c_DriftLength;
      } else if (fitVarianceString == std::string("pseudo")) {
        return EFitVariance::c_Pseudo;
      } else if (fitVarianceString == std::string("proper")) {
        return EFitVariance::c_Proper;
      } else {
        throw std::invalid_argument("Unexpected fit variance string : '" + fitVarianceString + "'");
      }
    }

  }
}
