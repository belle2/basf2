/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/UncertainHelix.h>

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainHelix UncertainHelix::average(const UncertainHelix& fromHelix,
                                       const UncertainHelix& toHelix)
{
  ParameterVector<5> fromPar = fromHelix.parameters();
  CovarianceMatrix<5> fromCov = fromHelix.helixCovariance();

  ParameterVector<5> toPar = toHelix.parameters();
  CovarianceMatrix<5> toCov = toHelix.helixCovariance();

  CovarianceMatrix<5> commonCov;
  ParameterVector<5> commonPar;

  double chi2 = CovarianceMatrixUtil::average(fromPar, fromCov,
                                              toPar, toCov,
                                              commonPar, commonCov);

  // Calculating 5 parameters from 10 input parameters. 5 NDF remaining.
  size_t ndf = 5;

  return UncertainHelix(commonPar, commonCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const JacobianMatrix<3, 5>& fromAmbiguity,
                                       const UncertainHelix& toHelix)
{
  ParameterVector<3> fromPar = fromPerigeeCircle.parameters();
  CovarianceMatrix<3> fromCov = fromPerigeeCircle.perigeeCovariance();

  ParameterVector<5> toPar = toHelix.parameters();
  CovarianceMatrix<5> toCov = toHelix.helixCovariance();
  JacobianMatrix<5, 5> toAmbiguity = JacobianMatrixUtil::identity<5>();

  // Helix covariance
  CovarianceMatrix<5> commonCov;

  // Helix parameters
  ParameterVector<5> commonPar;


  // Calculating 5 parameters from 8 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  double chi2 = 0;

  // Use the mean circle parameters as the reference, since the ambiguity matrix is a expansion around that point.
  ParameterVector<3> refPar = (fromPar + ParameterVectorUtil::getSub<ParameterVector<3>, 0 >(toPar)) / 2;

  ParameterVector<3> relFromPar = fromPar - refPar;

  // Only first three coordinates are effected by a change of the reference (expansion) point
  ParameterVector<5> relToPar = toPar;
  for (int i = 0; i < 3; ++i) {
    relToPar(i) -= refPar(i);
  }

  // Chi2 value
  chi2 = CovarianceMatrixUtil::average(relFromPar, fromCov, fromAmbiguity,
                                       relToPar, toCov, toAmbiguity,
                                       commonPar, commonCov);

  for (int i = 0; i < 3; ++i) {
    commonPar(i) += refPar(i);
  }

  return UncertainHelix(commonPar, commonCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const JacobianMatrix<3, 5>& fromAmbiguity,
                                       const UncertainPerigeeCircle& toPerigeeCircle,
                                       const JacobianMatrix<3, 5>& toAmbiguity,
                                       const SZParameters& szParameters)
{
  const ParameterVector<3>&  fromPar = fromPerigeeCircle.parameters();
  const CovarianceMatrix<3>& fromCov = fromPerigeeCircle.perigeeCovariance();

  const ParameterVector<3>&  toPar = toPerigeeCircle.parameters();
  const CovarianceMatrix<3>& toCov = toPerigeeCircle.perigeeCovariance();

  // Helix parameters
  ParameterVector<5> commonPar;

  // Helix covariance
  CovarianceMatrix<5> commonCov;

  // Calculating 5 parameters from 6 input parameters. 1 NDF remaining.
  size_t ndf = 1;
  double chi2 = 0;

  // Use the mean circle parameters as the reference, since the ambiguity matrix is a expansion around that point.
  ParameterVector<3> refPar = (fromPar + toPar) / 2;

  ParameterVector<3> relFromPar = fromPar - refPar;
  ParameterVector<3> relToPar = toPar - refPar;

  // Chi2 value
  chi2 = CovarianceMatrixUtil::average(relFromPar, fromCov, fromAmbiguity,
                                       relToPar,   toCov,   toAmbiguity,
                                       commonPar,  commonCov);

  commonPar(EHelixParameter::c_Curv) += refPar(EPerigeeParameter::c_Curv);
  commonPar(EHelixParameter::c_Phi0) += refPar(EPerigeeParameter::c_Phi0);
  commonPar(EHelixParameter::c_I)    += refPar(EPerigeeParameter::c_I);
  commonPar(EHelixParameter::c_TanL) += szParameters(ESZParameter::c_TanL);
  commonPar(EHelixParameter::c_Z0)   += szParameters(ESZParameter::c_Z0);

  return UncertainHelix(commonPar, commonCov, chi2, ndf);
}
