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

#include <tracking/trackFindingCDC/numerics/CovarianceMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/ParameterVectorUtil.h>

#include <tracking/trackFindingCDC/numerics/CovarianceMatrix.h>
#include <tracking/trackFindingCDC/numerics/PrecisionMatrix.h>
#include <tracking/trackFindingCDC/numerics/JacobianMatrix.h>
#include <tracking/trackFindingCDC/numerics/ParameterVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class T, class AEParameters>
    void UncertainParametersUtil<T, AEParameters>::transport(const JacobianMatrix& jacobian,
                                                             CovarianceMatrix& cov)
    {
      CovarianceMatrixUtil::transport(jacobian, cov);
    }

    template <class T, class AEParameters>
    auto UncertainParametersUtil<T, AEParameters>::transported(const JacobianMatrix& jacobian,
                                                               const CovarianceMatrix& cov)
    -> CovarianceMatrix {
      return CovarianceMatrixUtil::transported(jacobian, cov);
    }

    template <class T, class AEParameters>
    auto UncertainParametersUtil<T, AEParameters>::reversalJacobian() -> JacobianMatrix {
      return JacobianMatrixUtil::scale(T::reversalSigns());
    }

    template <class T, class AEParameters>
    void UncertainParametersUtil<T, AEParameters>::reverse(CovarianceMatrix& cov)
    {
      return transport(reversalJacobian(), cov);
    }

    template <class T, class AEParameters>
    auto UncertainParametersUtil<T, AEParameters>::reversed(const CovarianceMatrix& cov)
    -> CovarianceMatrix {
      return transported(reversalJacobian(), cov);
    }

    template <class T, class AEParameters>
    auto UncertainParametersUtil<T, AEParameters>::identity() -> CovarianceMatrix {
      return CovarianceMatrixUtil::identity<c_N>();
    }

    template <class T, class AEParameters>
    template <class AParameterVector, int I>
    AParameterVector
    UncertainParametersUtil<T, AEParameters>::getSubParameterVector(const ParameterVector& par)
    {
      return ParameterVectorUtil::getSub<AParameterVector, I>(par);
    }

    template <class T, class AEParameters>
    template <class ACovarianceMatrix, int I>
    ACovarianceMatrix
    UncertainParametersUtil<T, AEParameters>::getSubCovarianceMatrix(const CovarianceMatrix& cov)
    {
      return CovarianceMatrixUtil::getSub<ACovarianceMatrix, I>(cov);
    }

    template <class T, class AEParameters>
    auto
    UncertainParametersUtil<T, AEParameters>::covarianceFromPrecision(const PrecisionMatrix& prec)
    -> CovarianceMatrix {
      return CovarianceMatrixUtil::fromPrecision(prec);
    }

    template <class T, class AEParameters>
    auto
    UncertainParametersUtil<T, AEParameters>::precisionFromCovariance(const CovarianceMatrix& cov)
    -> PrecisionMatrix {
      return CovarianceMatrixUtil::toPrecision(cov);
    }

    template <class T, class AEParameters>
    auto
    UncertainParametersUtil<T, AEParameters>::covarianceFromFullPrecision(const PrecisionMatrix& prec)
    -> CovarianceMatrix {
      return CovarianceMatrixUtil::fromFullPrecision(prec);
    }

    template <class T, class AEParameters>
    auto
    UncertainParametersUtil<T, AEParameters>::precisionFromFullCovariance(const CovarianceMatrix& cov)
    -> PrecisionMatrix {
      return CovarianceMatrixUtil::fullToPrecision(cov);
    }
  }
}
