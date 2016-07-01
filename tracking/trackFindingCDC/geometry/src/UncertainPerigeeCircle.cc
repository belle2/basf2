/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainPerigeeCircle UncertainPerigeeCircle::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                                       const UncertainPerigeeCircle& toPerigeeCircle)
{
  const ParameterVector<3>&  fromPar = fromPerigeeCircle.parameters();
  const CovarianceMatrix<3>& fromCov = fromPerigeeCircle.perigeeCovariance();

  const ParameterVector<3>&  toPar = toPerigeeCircle.parameters();
  const CovarianceMatrix<3>& toCov = toPerigeeCircle.perigeeCovariance();

  const ParameterVector<3>&  refPar = (fromPar + toPar) / 2.0;

  ParameterVector<3> relFromPar = fromPar - refPar;
  ParameterVector<3> relToPar = toPar - refPar;

  ParameterVector<3> commonPar;
  CovarianceMatrix<3> commonCov;
  double chi2 = CovarianceMatrixUtil::average(relFromPar, fromCov,
                                              relToPar, toCov,
                                              commonPar, commonCov);

  commonPar += refPar;

  // Calculating 3 parameters from 6 input parameters. 3 NDF remaining.
  size_t ndf = 3;
  return UncertainPerigeeCircle(commonPar, commonCov, chi2, ndf);
}
