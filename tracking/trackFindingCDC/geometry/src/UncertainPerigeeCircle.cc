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

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <ostream>

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

  PerigeeParameters avgPar;
  PerigeeCovariance avgCov;

  double chi2 = PerigeeUtil::average(fromPar, fromCov, toPar, toCov, avgPar, avgCov);

  // Calculating 3 parameters from 6 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainPerigeeCircle(avgPar, avgCov, chi2, ndf);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const UncertainPerigeeCircle& circle)
{
  return output << "UncertainPerigeeCircle("
         << "curvature=" << circle->curvature() << ","
         << "phi0=" << circle->phi0() << ","
         << "impact=" << circle->impact() << ")";
}
