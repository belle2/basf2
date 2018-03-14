/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/UncertainParameters.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Namespace to hide the contained enum constants
    namespace NLineParameterIndices {

      /// Enumeration to address the individual helix parameters in a vector or matrix
      enum ELineParameter {

        /// Constant to address the azimuth angle of the direction of flight at the perigee
        c_Phi0 = 0,

        /// Constant to address the impact parameter
        c_I = 1,

        /// Constant for the total number of indices.
        c_N = 2,
      };
    }

    /// Importing the enumeration for the namespace but not the constants.
    using ELineParameter = NLineParameterIndices::ELineParameter;

    // Guard to prevent repeated template symbol emission
    struct LineUtil;
    extern template struct UncertainParametersUtil<LineUtil, ELineParameter>;

    /// Utility struct for functions and types related to the line parameters.
    struct LineUtil : UncertainParametersUtil<LineUtil, ELineParameter> {

      /// Getter for the signs which have to be applied to reverse the traversal direction
      static ParameterVector reversalSigns()
      {
        return ParameterVector({ -1.0, -1.0});
      }

      /**
       *  Calculates the weighted average between two line parameter sets
       *  with their respective covariance matrix.
       *
       *  Returns the chi2 value of the average.
       */
      static double average(const LineUtil::ParameterVector& fromPar,
                            const LineUtil::CovarianceMatrix& fromCov,
                            const LineUtil::ParameterVector& toPar,
                            const LineUtil::CovarianceMatrix& toCov,
                            LineUtil::ParameterVector& avgPar,
                            LineUtil::CovarianceMatrix& avgCov);
    };

    /// Vector of the line parameters
    using LineParameters = LineUtil::ParameterVector;

    /// Covariance matrix of the line parameters
    using LineCovariance = LineUtil::CovarianceMatrix;

    /// Precision matrix of the line parameters
    using LinePrecision = LineUtil::PrecisionMatrix;

    /// Jacobian matrix for the line parameters
    using LineJacobian = LineUtil::JacobianMatrix;

  }

}
