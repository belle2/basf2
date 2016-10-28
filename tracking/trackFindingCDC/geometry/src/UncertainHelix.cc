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

using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainHelix UncertainHelix::average(const UncertainHelix& fromHelix,
                                       const UncertainHelix& toHelix)
{
  HelixParameters fromPar = fromHelix.helixParameters();
  HelixCovariance fromCov = fromHelix.helixCovariance();

  HelixParameters toPar = toHelix.helixParameters();
  HelixCovariance toCov = toHelix.helixCovariance();

  using namespace NHelixParameterIndices;
  HelixParameters refPar = (fromPar + toPar) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  HelixParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;
  HelixCovariance avgCov;

  double chi2 =
    CovarianceMatrixUtil::average(relFromPar, fromCov, relToPar, toCov, relAvgPar, avgCov);

  HelixParameters avgPar = relAvgPar + refPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  // Calculating 5 parameters from 10 input parameters. 5 NDF remaining.
  size_t ndf = 5;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const JacobianMatrix<3, 5>& fromAmbiguity,
                                       const UncertainHelix& toHelix)
{
  PerigeeParameters fromPar = fromPerigeeCircle.perigeeParameters();
  PerigeeCovariance fromCov = fromPerigeeCircle.perigeeCovariance();

  HelixParameters toPar = toHelix.helixParameters();
  HelixCovariance toCov = toHelix.helixCovariance();
  JacobianMatrix<5, 5> toAmbiguity = JacobianMatrixUtil::identity<5>();

  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar.head<3>()) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters refHelixPar;
  refHelixPar << refPar, toPar.tail<2>();

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  HelixParameters relToPar = toPar - refHelixPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;
  HelixCovariance avgCov;

  // Chi2 value
  double chi2 = CovarianceMatrixUtil::average(relFromPar,
                                              fromCov,
                                              fromAmbiguity,
                                              relToPar,
                                              toCov,
                                              toAmbiguity,
                                              relAvgPar,
                                              avgCov);

  HelixParameters avgPar = relAvgPar + refHelixPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  // Calculating 5 parameters from 8 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const JacobianMatrix<3, 5>& fromAmbiguity,
                                       const UncertainPerigeeCircle& toPerigeeCircle,
                                       const JacobianMatrix<3, 5>& toAmbiguity,
                                       const SZParameters& szParameters)
{
  const PerigeeParameters& fromPar = fromPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& fromCov = fromPerigeeCircle.perigeeCovariance();

  const PerigeeParameters& toPar = toPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& toCov = toPerigeeCircle.perigeeCovariance();

  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters refHelixPar;
  refHelixPar << refPar, szParameters;

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  PerigeeParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;
  HelixCovariance avgCov;

  // Chi2 value
  double chi2 = CovarianceMatrixUtil::average(relFromPar,
                                              fromCov,
                                              fromAmbiguity,
                                              relToPar,
                                              toCov,
                                              toAmbiguity,
                                              relAvgPar,
                                              avgCov);

  HelixParameters avgPar = relAvgPar + refHelixPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  // Calculating 5 parameters from 6 input parameters. 1 NDF remaining.
  size_t ndf = 1;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}
