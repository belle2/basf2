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

#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainPerigeeCircle
UncertainPerigeeCircle::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                const UncertainPerigeeCircle& toPerigeeCircle)
{
  const PerigeeParameters& fromPar = fromPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& fromCov = fromPerigeeCircle.perigeeCovariance();

  const PerigeeParameters& toPar = toPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& toCov = toPerigeeCircle.perigeeCovariance();

  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar) / 2.0;
  refPar(c_Phi0) = AngleUtil::average(fromPar(c_Phi0), toPar(c_Phi0));

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar(c_Phi0));

  PerigeeParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar(c_Phi0));

  PerigeeParameters relAvgPar;
  PerigeeCovariance avgCov;
  double chi2 =
    CovarianceMatrixUtil::average(relFromPar, fromCov, relToPar, toCov, relAvgPar, avgCov);

  PerigeeParameters avgPar = relAvgPar + refPar;
  AngleUtil::normalise(avgPar(c_Phi0));

  // Calculating 3 parameters from 6 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainPerigeeCircle(avgPar, avgCov, chi2, ndf);
}
